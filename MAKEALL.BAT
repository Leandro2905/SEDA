@echo off

rem This should be the path to your PPD GEM bindings

set GEMLIB=d:/tools/c/lib

pacc -c -Bs -O -Zg GEMINIT.C
pacc -c -Bs -O -Zg GEMASYNC.C
pacc -c -Bs -O -Zg GEMCTRL.C
pacc -c -Bs -O -Zg EMSLIB.C
pacc -c -Bs -O -Zg GEMBASE.C
pacc -c -Bs -O -Zg GEMDISP.C
pacc -c -Bs -O -Zg GEMFLAG.C
pacc -c -Bs -O -Zg GEMINPUT.C
pacc -c -Bs -O -Zg GEMPD.C
pacc -c -Bs -O -Zg GEMQUEUE.C
pacc -c -Bs -O -Zg GEMWRECT.C
pacc -c -Bs -O -Zg GEMSUP1.C
pacc -c -Bs -O -Zg GEMSUP2.C
pacc -c -Bs -O -Zg GEMAPLIB.C
pacc -c -Bs -O -Zg GEMEVLIB.C
pacc -c -Bs -O -Zg GEMMNLIB.C
pacc -c -Bs -O -Zg GEMOBED.C
pacc -c -Bs -O -Zg GEMOBLIB.C
pacc -c -Bs -O -Zg GEMOBEMS.C
pacc -c -Bs -O -Zg GEMFMALT.C
pacc -c -Bs -O -Zg GEMFMLIB.C
pacc -c -Bs -O -Zg GEMSCLIB.C
pacc -c -Bs -O gemfslib.c	
pacc -c -Bs -O -Zg GEMWM1.C
pacc -c -Bs -O -Zg GEMWM2.C
pacc -c -Bs -O -Zg GEMSHLIB.C
pacc -c -Bs -O -Zg GEMRSLIB.C
pacc -c -Bs -O -Zg GEMGRLIB.C
pacc -c -Bs -O -Zg GEMGSXIF.C
pacc -c -Bs -O -Zg GEMPPLIB.C
pacc -c -Bs -O -Zg GEMCICON.C
pacc -c -Bs -O -Zg XSTATE.C
pacc -c -Bs -O -Zg DEBUG.C
pacc -c -Bs -O -Zg GEMGRAF.C
pacc -c -Bs -O -Zg OPTIMIZE.C
pacc -c -Bs -O -Zg GEMDOS.C
pacc -c -Bs -O -Zg GEMOBJOP.C
libr r emma.lib GEMINIT.OBJ GEMGMAIN.OBJ GEMASYNC.OBJ GEMCTRL.OBJ EMSLIB.OBJ
libr r emma.lib GEMBASE.OBJ GEMDISP.OBJ GEMFLAG.OBJ GEMINPUT.OBJ GEMPD.OBJ GEMQUEUE.OBJ
libr r emma.lib GEMWRECT.OBJ GEMSUP1.OBJ GEMSUP2.OBJ GEMAPLIB.OBJ GEMEVLIB.OBJ GEMMNLIB.OBJ
libr r emma.lib GEMOBED.OBJ GEMOBLIB.OBJ GEMOBEMS.OBJ GEMFMALT.OBJ GEMFMLIB.OBJ GEMSCLIB.OBJ
libr r emma.lib GEMFSLIB.OBJ GEMWM1.OBJ GEMWM2.OBJ GEMSHLIB.OBJ GEMRSLIB.OBJ GEMGRLIB.OBJ
libr r emma.lib GEMGSXIF.OBJ GEMPPLIB.OBJ GEMASM.OBJ GEMCICON.OBJ XSTATE.OBJ 
libr r emma.lib DEBUG.OBJ GEMGRAF.OBJ GSX2.OBJ OPTIMIZE.OBJ LARGE.OBJ GEMDOS.OBJ
libr r emma.lib GEMDOS.OBJ GEMOBJOP.OBJ GEMDOSIF.OBJ GEMEMSIF.OBJ OPTIMOPT.OBJ 
libr r emma.lib GEMGLOBE.OBJ LDIV.OBJ LMUL.OBJ PROEND.OBJ
hlink -z -lm -p_TEXT,text,CODE=0/,LDATA=0/,LBSS=0/,NVRAM=0/,data=0/,bss,stack=0/ -Memma.map -otmp.obj GEMSTART.OBJ emma.lib
objtoexe -e -lDos_hdr tmp.obj emma.exe
erase tmp.obj
cd accs
pacc -Bs -O -Ogem.exe main.c gem.c %GEMLIB%/ppdgem-s.lib
pacc -Bs -O -Ggem.sym main.c gem.c %GEMLIB%/ppdgem-s.lib
exe2acc gem.exe gem.aes
cd ..
