Firstly, I'm not the right person to maintain this. The mathematics of compression is way over my head.
This version exists only because I of my personal need. And the fact that I vould feel bad about myself if I sat on a vorking version, and did not share it.
And my only criteria for success is that it compiles/runs, and does not fill my serial-log with muForce/PatchWork hits.

Except making most clients external, to avoid the main library getting un-practically big(in my opinion) for m68k,
the only changes I've done are the absolute minimum required to get it compiling/working on m68k Amiga again.

The relevant files for compiling are:
libxad/portable/amiga/SMakeFile
libxad/amiga/source/clients/SMakeFile

See also "libxad/needed_assigns"

All copyrights belongs to the authors, see respective files for info.

The binary release contains only the xadmaster.library for 68000-68060 and the clients that goes Libs:xad.
Anything else that might be needed can be collected from the official xadmaster v12.1a release on Aminet.

-Hagbard Celine
