README for 10 July 2005 build of the 'EMMA' AES
===============================================

How to install:

1. Check that you have EMS memory available - 1Mb or so is fine.
2. Take an existing installation of GEM/3 or FreeGEM.
3. Back up the existing GEM.EXE and GEM.RSC.
4. Copy GEM.EXE, GEM.RSC, GEM.AES and GEMFSLIB.RSC into \GEMAPPS\GEMSYS.
5. If you have PALETTE.ACC installed, rename it to PALETTE.ACX so it doesn't 
load.
6. Cross your fingers and launch GEM.

What's so special about EMMA?
-----------------------------

  This version of the AES includes EMS memory support. It loads Desk 
Accessories into EMS rather than conventional memory, and it also uses EMS to 
store its windows. As a consequence, it supports rather more of either than a 
standard GEM; you can have 7 DAs rather than 3, and about 200 windows.

  As a consequence, you may find stability has taken a bit of a hammering, and
some programs won't run at all. PALETTE.ACC, for example, won't work because
it hooks interrupts and then gets paged out. Do take care not to trust
valuable files to EMMA without taking backups first.

  All the other FreeGEM AES features should be present and work correctly.
