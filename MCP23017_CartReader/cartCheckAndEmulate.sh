#!/bin/bash    
ROMPATH=~/RetroPie/roms/snes
USERHOME=~/


python cart_reader.py -d "$ROMPATH"
LIBRETROPATH="/home/pi/RetroPie/emulatorcores/pocketsnes-libretro/libretro.so"
EMULATIONCMD="sudo -u pi retroarch "


if [ -f /tmp/insertedCart ]; then 
   CARTNAME=$(</tmp/insertedCart)
   if [ "$CARTNAME" != "NULL" ]; then
      SMCEXT=".smc"
      FILENAME=$CARTNAME$SMCEXT
      FULLPATH=$ROMPATH/${FILENAME// /\ }
      $EMULATIONCMD "$FULLPATH"  -L $LIBRETROPATH "--savestate" $ROMPATH "--save" $ROMPATH
   else
    emulationstation
   fi
else
 emulationstation
fi
