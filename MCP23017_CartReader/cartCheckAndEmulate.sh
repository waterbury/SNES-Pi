#!/bin/bash    
ROMPATH=~/RetroPie/roms/snes
USERHOME=~/


python cart_reader.py -d "$ROMPATH"

CARTNAME=$(<insertedCart)
SMCEXT=".smc"
FILENAME=$CARTNAME$SMCEXT

FULLPATH=$ROMPATH/${FILENAME// /\ }


LIBRETROPATH="/home/pi/RetroPie/emulatorcores/pocketsnes-libretro/libretro.so"

EMULATIONCMD="sudo -u pi retroarch "

$EMULATIONCMD "$FULLPATH"  -L $LIBRETROPATH "--savestate" $ROMPATH "--save" $ROMPATH
