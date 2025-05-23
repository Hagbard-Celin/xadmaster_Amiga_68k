#ifndef XADMASTER_CLIENTS_C
#define XADMASTER_CLIENTS_C

/*  $Id: clients.c,v 1.12 2005/06/23 14:54:37 stoecker Exp $
    the xad packing library clients

    XAD library system for archive handling
    Copyright (C) 1998 and later by Dirk St�cker <soft@dstoecker.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <xadmaster.h>
#ifdef DEBUG
void DebugFileSearched(const struct xadArchiveInfo *ai, const xadSTRING *, ...); /* print with 'D' */
void DebugClient(const struct xadArchiveInfo *ai, const xadSTRING *, ...);       /* print with 'D' */
#endif

#include "version.h"
#define XADMASTERFILE

#define XADIOGETBITSHIGH
#define XADIOGETBITSLOW
#define XADIOREADBITSLOW
#define XADIOGETBITSLOWR
#include "clients/xadIO.c" /* needed to enable all bit functions */

/* last client comes first -- in client list they are sorted correct */

#define XADNEXTCLIENT 0

#include "clients/Ace.c"
#include "clients/Cpio.c"
#include "clients/CrunchDisk.c"
#include "clients/DCS.c"
#include "clients/FS_Amiga.c"
#include "clients/FS_FAT.c"
#include "clients/FS_SOS.c"
#include "clients/SuperDuper3.c"
#include "clients/xDisk.c"

const struct xadClient * const RealFirstClient = XADNEXTCLIENT;

#endif /* XADMASTER_CLIENTS_C */
