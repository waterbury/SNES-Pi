#!/bin/bash    
ROMPATH=/home/pi/RetroPie/roms/snes/
USERHOME=/home/pi/


python "$USERHOME"SNES-Pi/MCP23017_CartReader/cart_reader.py -d "$ROMPATH"
LIBRETROPATH="/home/pi/RetroPie/emulatorcores/pocketsnes-libretro/libretro.so"
EMULATIONCMD="retroarch "


if [ -f /tmp/insertedCart ]; then 
   CARTNAME=$(</tmp/insertedCart)
   if [ "$CARTNAME" != "NULL" ]; then
      SMCEXT=".smc"
      FILENAME=$CARTNAME$SMCEXT
      FULLPATH=$ROMPATH/${FILENAME// /\ }

      if [ $# -eq 1 ]
       then
        aoss snes9x "$FULLPATH"
       else
        $EMULATIONCMD "$FULLPATH"  -L $LIBRETROPATH "--savestate" $ROMPATH -c "/etc/retroarch.cfg" "--save" $ROMPATH 
       fi
   else
    sudo emulationstation
   fi
else
 sudo emulationstation
fi
