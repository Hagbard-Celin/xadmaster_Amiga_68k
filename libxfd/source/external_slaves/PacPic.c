/* XFD external slave for AMOS Pac.Pic. format by Kyzer/CSG <kyzer@4u.net>
 * Pac.Pic. format (C) Fran�ois Lionet
 *
 * This slave code is in the Public Domain.
 */

#include <libraries/xfdmaster.h>
#include <proto/exec.h>
#include <exec/memory.h>
#include <string.h>
#include "SDI_compiler.h"

static char version[]="$VER: AMOS Pac.Pic. 1.2 (23.02.2002) by <kyzer@4u.net>";

#define EndGetM32(a)  ((((a)[0])<<24)|(((a)[1])<<16)|(((a)[2])<<8)|((a)[3]))
#define EndGetM16(a)  ((((a)[0])<<8)|((a)[1]))
#define GETLONG(x,n) EndGetM32(&(x)[(n)])
#define GETWORD(x,n) EndGetM16(&(x)[(n)])

#define ERROR(x)  { xbi->xfdbi_Error = XFDERR_##x ; return 0; }

/* AMOS PAC.PIC. FORMAT:
 *
 * AMOS records details about the physical hardware screen that the picture
 * is displayed on (all the required details to re-perform the SCREEN OPEN
 * and SCREEN DISPLAY command, if the picture is unpacked to a screen number
 * that doesn't exist).
 *
 * It is also possible to only pack an area of the screen. Therefore,
 * seperate screen and picture headers.
 *
 * off	local	what
 * 00	00	"AmBk"
 * 04	04	bank number.w
 * 06	06	???.w
 * 08	08	bank length.l (may have bit 31 set)
 * 0a	0a	"Pac.Pic."
 *      14	SIZEOF amos header
 *
 * 14	00	SCREEN header id.l (=$12031990, 31-December-1990)
 * 18	04	screen width.w (eg $0140, 320)
 * 1A	06	screen height.w (eg $00c8, 200)
 * 1C	08	hardware top-left x coord.w (eg $0081, 129)
 * 1E	0a	hardware top-left y coord.w (eg $0032, 50)
 * 20	0c	hardware screen width.w
 * 22	0e	hardware screen height.w
 * 24	10	???.w
 * 26	12	???.w
 * 28	14	contents of bplcon0.w (details the amiga screen mode)
 * 2a	16	number of colours.w (2,4,8,16,32,64,4096)
 * 2c	18	number of bitplanes.w (1-6)
 * 2e	1a	uword palette[32]
 *	5a	SIZEOF screen header
 *
 * 6e	00	PICTURE header id.l (=$06071963, 7-June-1963)
 * 72	04	picture X offset (in bytes, within screen).w
 * 74	06	picture Y offset (in lines, within screen).w
 * 76	08	picture width (in bytes, within screen).w
 * 78	0a	picture height (in 'line lumps's, within screen).w
 * 7A	0c	number of lines in a 'line lump'.w
 * 7C	0e	number of bitplanes.w
 * 7E	10	RLEDATA offset (from picture header).l
 * 82	14	POINTS offset (from picture header).l
 *	18	SIZEOF picture header
 *
 * 86	picdata begins here...
 *
 *
 * Actual PICDATA/RLEDATA/POINTS stream format:
 *
 * First, the data in the screen is reordered:
 * - we compress bitplane 0, then bitplane 1, bitplane 2...
 * - within a bitplane, we compress a 'lump' of complete lines,
 *   working from y=0 towards y=height-1
 * - within a lump, we compress blocks of 8 pixels by lumpheight pixels,
 *   from x=0, 8, 16, 24, 32 ... to width
 * - within that block, we compress from top to bottom. As 8 pixels in
 *   one bitplane is 1 byte, that is the stream to compress.
 *
 * The size LUMPHEIGHT is varied to get the best compression.
 *
 * With that stream, we look at each byte.
 * - We always store the first picture byte.
 * - For each picture byte, we store 1 RLE bit, to say whether this
 *   picture byte is repeated or not. The RLE bits are stored in the
 *   RLEDATA stream, from most significant to least significant bit of a byte.
 * - If a picture byte is different from the previous, we store a '1'
 *   as the RLE bit, and we output that picture byte to the PICDATA stream.
 * - If a picture byte is the same as the previous, we store a '0' as the
 *   RLE bit, and do noe write anything to the PICDATA stream.
 *
 * Once the PICDATA and RLEDATA streams are completed, we compress the
 * RLEDATA by the same method as we compressed the raw bitmap! Now, the
 * output of this is 'RLEDATA', and the RLE bits generated by this are
 * in the POINTS stream.
 */

#define IFF_HEADER_LEN (0xA4)
#define IFF_HEADER_LEN_STATIC (0x3C)

static const UBYTE iff_header[IFF_HEADER_LEN_STATIC] = {
  'F', 'O', 'R', 'M',    /* 00 FORM                        */
   0,   0,   0,   0,     /* 04   form length               */
  'I', 'L', 'B', 'M',    /* 08   ILBM                      */ 
  'B', 'M', 'H', 'D',    /* 0c   BMHD                      */ 
   0,   0,   0,   20,    /* 10     bmhd chunk length (20)  */ 
   0,   0,               /* 14     width                   */
   0,   0,               /* 16     height                  */
   0,   0,               /* 18     x offset (0)            */
   0,   0,               /* 1a     y offset (0)            */
   0,                    /* 1c     number of bitplanes     */
   0,                    /* 1d     masking (0)             */
   0,                    /* 1e     compression (0)         */
   0,                    /* 1e     reserved1 (0)           */
   0,   0,               /* 20     transparent colour (0)  */
   1,                    /* 22     x aspect (1)            */
   1,                    /* 23     x aspect (1)            */
   0,   0,               /* 24     page width              */
   0,   0,               /* 26     page height             */
  'C', 'A', 'M', 'G',    /* 28   CAMG                      */
   0,   0,   0,   4,     /* 2c     camg chunk length (4)   */
   0,   0,   0,   0,     /* 30     viewmode                */
  'C', 'M', 'A', 'P',    /* 34   CMAP                      */
   0,   0,   0,   96     /* 38     cmap chunk length (96)  */
                         /* 3c     {UBYTE r,g,b}[32]       */
                         /* 9c   BODY                      */
                         /* a0     body chunk length       */
                         /* a4     unpacked raw bitplanes  */
};

/* BITPLANE format:
 * width_bytes of line 0 plane 0
 * width_bytes of line 0 plane 1 ...
 * width_bytes of line 1 plane 0
 * width_bytes of line 1 plane 1 ...
 * ...
 */




ASM(BOOL) PacPic_recog(REG(a0, UBYTE *d), REG(d0, ULONG len),
  REG(a1, struct xfdRecogResult *rr)) {

  /* skip over any AMOS headers */
       if (GETLONG(d,0) == 0x416D426B) d += 0x14, len -= 0x14; /* AmBk */
  else if (GETLONG(d,4) == 0x5069632E) d += 0x08, len -= 0x08; /* Pic. */
  else if (GETLONG(d,4) == 0x5061632E) d += 0x0c, len -= 0x0c; /* Pac. */

  /* d[] should now be pointing at a screen header or picture header */
  if (GETLONG(d,0) == 0x12031990) {
    if (len < 0x5e) return (BOOL) 0;
    d += 0x5a, len -= 0x5a;
  }

  if (GETLONG(d,0) == 0x06071963) {
    int width_bytes = GETWORD(d,0x08);
    int height = GETWORD(d,0x0a) * GETWORD(d,0x0c);
    int planes = GETWORD(d,0x0e);
    if (width_bytes & 0x0001) width_bytes++; /* for IFF ILBM row alignment */

    if (len < 0x18) return (BOOL) 0;
    rr->xfdrr_MinTargetLen = rr->xfdrr_FinalTargetLen =
      IFF_HEADER_LEN + (width_bytes * height * planes);
    return (BOOL) 1;
  }
  return (BOOL) 0;
}


ASM(BOOL) PacPic_decrunch(REG(a0, struct xfdBufferInfo * xbi),
  REG(a6, struct xfdMasterBase *xfdMasterBase)) {

  UBYTE *src  = (UBYTE *) xbi->xfdbi_SourceBuffer;
  UBYTE *dest = (UBYTE *) xbi->xfdbi_UserTargetBuf;
  UBYTE *ends = &src[xbi->xfdbi_SourceBufLen];
  UBYTE *d = dest, *colours, *picdata, *rledata, *points, *plane;
  ULONG length;

  int bplcon0, pic_width, pic_height, lump_height, lumps, 
      bitplanes, width_bytes, real_width_bytes, iff_line_bytes,
      i, j, k, l, rrbit, rbit, picbyte, rlebyte;

       if (GETLONG(src,0) == 0x416D426B) src += 0x14; /* AmBk */
  else if (GETLONG(src,4) == 0x5069632E) src += 0x08; /* Pic. */
  else if (GETLONG(src,4) == 0x5061632E) src += 0x0c; /* Pac. */

  if (GETLONG(src,0) == 0x12031990) {
    /* starts with a screen header */
    bplcon0 = GETWORD(src,0x14); /* get screenmode */
    colours = &src[0x1A];
    src += 0x5a; /* skip screen header */
  }
  else {
    /* starts with a picture header */
    bplcon0 = (GETWORD(src,0x0E) << 12) | 0x0200; /* invent a screen mode */
    colours = NULL; /* no palette */
  }
  real_width_bytes = GETWORD(src,0x08);
  lumps            = GETWORD(src,0x0A);
  lump_height      = GETWORD(src,0x0C);
  bitplanes        = GETWORD(src,0x0E);
  picdata          = &src[0x18];
  rledata          = &src[GETLONG(src,0x10)];
  points           = &src[GETLONG(src,0x14)];

  if (picdata > ends || rledata > ends || points > ends) ERROR(CORRUPTEDDATA);

  /* width in bytes is compensated for ILBM's word-alignment for rows */
  width_bytes = real_width_bytes;
  if (width_bytes & 0x0001) width_bytes++;

  /* number of bytes jump to get to the next line in IFF */
  iff_line_bytes = width_bytes * bitplanes;
  pic_width  = real_width_bytes << 3;
  pic_height = lumps * lump_height;

  /* write IFF headers */

  memcpy(d, &iff_header[0], IFF_HEADER_LEN_STATIC);
  d += 4;
  length = xbi->xfdbi_TargetBufSaveLen - 8;
  *d++ = (length >> 24) & 0xFF;
  *d++ = (length >> 16) & 0xFF;
  *d++ = (length >>  8) & 0xFF;
  *d++ = (length      ) & 0xFF;
  d += 12;
  *d++ = (pic_width  >> 8) & 0xFF;
  *d++ = (pic_width      ) & 0xFF;
  *d++ = (pic_height >> 8) & 0xFF;
  *d++ = (pic_height     ) & 0xFF;
  d += 4;
  *d++ = bitplanes;
  d += 7;
  *d++ = (pic_width  >> 8) & 0xFF;
  *d++ = (pic_width      ) & 0xFF;
  *d++ = (pic_height >> 8) & 0xFF;
  *d++ = (pic_height     ) & 0xFF;
  d += 10;
  *d++ = (bplcon0 >> 8) & 0xFF;
  *d++ = (bplcon0     ) & 0xFF;
  d += 8;
  for (i = 0; i < 32; i++) {
    UWORD colour = (colours) ? GETWORD(colours,i*2) : ((i & 0x0F) * 0x111);
    *d++ = ((colour >> 8) & 0xF) * 0x11;
    *d++ = ((colour >> 4) & 0xF) * 0x11;
    *d++ = ((colour     ) & 0xF) * 0x11;
  }
  *d++ = 'B';
  *d++ = 'O';
  *d++ = 'D';
  *d++ = 'Y';
  length = xbi->xfdbi_TargetBufSaveLen - IFF_HEADER_LEN;
  *d++ = (length >> 24) & 0xFF;
  *d++ = (length >> 16) & 0xFF;
  *d++ = (length >>  8) & 0xFF;
  *d++ = (length      ) & 0xFF;

  rbit = 7;
  rrbit = 6;
  picbyte = *picdata++;
  rlebyte = *rledata++;
  if (*points & 0x80) rlebyte = *rledata++;

  /* now do the actual decrunching... */
  plane = d;
  for (i = 0; i < bitplanes; i++) {
    UBYTE *lump_start = plane;
    for (j = 0; j < lumps; j++) {
      UBYTE *lump_offset = lump_start;
      for (k = 0; k < real_width_bytes; k++) {
        d = lump_offset;
        for (l = 0; l < lump_height; l++) {
          /* if the current RLE bit is set to 1, read in a new picture byte */
          if (rlebyte & (1 << rbit--)) picbyte = *picdata++;

          /* write picture byte and move down by one line in the picture */
          *d = picbyte; d += iff_line_bytes;

          /* if we've run out of RLE bits, check the POINTS bits to see if
           * a new RLE byte is needed
           */
          if (rbit < 0) {
            rbit = 7;
            if (*points & (1 << rrbit--)) rlebyte = *rledata++;
            if (rrbit < 0)  rrbit = 7, points++;
          }
        }
        lump_offset++;
      }
      lump_start += iff_line_bytes * lump_height;
    }
    plane += width_bytes; /* iff interleaved bitplanes */
  }

  picdata--; rledata--; points--;
  if (picdata > ends || rledata > ends || points > ends) ERROR(CORRUPTEDDATA);

  /* good exit */
  return 1;
}

struct xfdSlave FirstSlave = {
  NULL, XFDS_VERSION, 39, "(Pac.Pic.) Data Cruncher",
  XFDPFF_DATA|XFDPFF_RECOGLEN|XFDPFF_USERTARGET,
  0, (BOOL (*)()) PacPic_recog, (BOOL (*)()) PacPic_decrunch,
  NULL, NULL, 0, 0, 0x18
};
