As the binary download can be quite hard to find I'm putting the link here: [xadmasterV13m68kTestBuild.lha](https://github.com/Hagbard-Celin/xadmaster_Amiga_68k/releases/download/v13.0-Amiga-m68k-test_beta/xadmasterV13m68kTestBuild.lha)

Firstly, I'm not the right person to maintain this. The mathematics of compression is way over my head.
This version exists only because I of my personal need. And the fact that I would feel bad about myself if I sat on a working version, and did not share it.
And my only criteria for success is that it compiles/runs, and does not fill my serial-log with muForce/PatchWork hits.

Except making most clients external, to avoid the main library getting un-practically big(in my opinion) for m68k,
the only changes I've done are the absolute minimum required to get it compiling/working on m68k Amiga again.

Compiled with sas/c, no GCC needed.

The relevant files for compiling are:
"libxad/portable/amiga/SMakeFile"
"libxad/amiga/source/clients/SMakeFile"

See also "libxad/needed_assigns"

All copyrights belongs to the authors, see respective files for info.

The binary release contains only the xadmaster.library for 68000-68060 and the clients that goes Libs:xad.
Anything else that might be needed can be collected from the official xadmaster v12.1a release on Aminet.

-Hagbard Celine
