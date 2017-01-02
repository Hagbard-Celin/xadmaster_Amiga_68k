/*
 $Id: CLASS_CfgPageFT_Defs.h,v 1.1 2004/01/20 06:24:59 stoecker Exp $
 Holds defines for the file types config page.

 VX - User interface for the XAD decompression library system.
 Copyright (C) 1999 and later by Andrew Bell <mechanismx@lineone.net>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#ifndef CFGPAGEFT_DEF_H
#define CFGPAGEFT_DEF_H

struct FileTypeEntry {
	ULONG       fte_State;   /* FTYPESSTATE_#? */
	UBYTE      *fte_Pattern, *fte_Viewer, *fte_Comment;
};

#define FTYPES_FILENAME     "ENVARC:VX/VX.filetypes"
#define OLDFTYPES_ID		"VXFILETYPES" /* Use prior to VX 1.5 BETA 8 */
#define FTYPES_ID           "VXFT" /* Introduced in VX 1.5 BETA 8 */

/* Note: OLDFTYPES_ID represents older filetypes that don't
				 have encoded quotes. It's still supported for backwards
				 compatibility.  */

#define FTYPES_HEADER       FTYPES_ID "\n" \
		";\n" "; Do not edit this file by hand!\n" ";\n"

#define FTYPES_SAVETEMPLATE "STATE=\"%s\" PAT=\"%s\" VIEWER=\"%s\" COMMENT=\"%s\"\n"
#define FTYPES_TEMPLATE     "STATE/K/A,PAT=PATTERN/K/A,VIEWER/K/A,COMMENT/K/A"

struct FT_Template {
	UBYTE *ftt_State, *ftt_Pattern, *ftt_Viewer, *ftt_Comment;
};

#define FTYPES_IOBUFFER_SIZE (1024*8)

#endif /* CFGPAGEFT_DEF_H */