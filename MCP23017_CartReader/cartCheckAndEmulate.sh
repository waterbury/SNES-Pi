#!/bin/bash    
ROMPATH=~/RetroPie/roms/snes
USERHOME=~/


python "$USERHOME"SNES-Pi/MCP23017_CartReader/cart_reader.py -d "$ROMPATH"
LIBRETROPATH="/home/pi/RetroPie/emulatorcores/pocketsnes-libretro/libretro.so"
EMULATIONCMD="retroarch "


if [ -f /tmp/insertedCart ]; then 
   CARTNAME=$(</tmp/insertedCart)
   if [ "$CARTNAME" != "NULL" ]; then
      SMCEXT=".smc"
      FILENAME=$CARTNAME$SMCEXT
      FULLPATH=$ROMPATH/${FILENAME// /\ }
      $EMULATIONCMD "$FULLPATH"  -L $LIBRETROPATH "--savestate" $ROMPATH -c "/etc/retroarch.cfg" "--save" $ROMPATH 
   else
    emulationstation
   fi
else
 emulationstation
fi
