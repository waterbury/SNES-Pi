SNES-Pi
=======

Super Nintendo Emulated System
http://familab.org/blog/2012/12/snes-super-nintendo-emulated-system/

cart_reader.py -
 Acts as python-based cartridge reader, using MCP23017 chips connected to Raspberry Pi's I2C bus. The filename 
 of the ROM and SRAM save data will be determined by Game Title located in game's header. If ROM filename already
 exists, the cart reader will not attempt to rip again.
 
 The Game Title will be stored in "/tmp/insertedCart" If no cart is inserted, or the inverse, and ROM checksums
 do not match, "/tmp/insertedCart" will read "NULL"

 Options:
  -d (by default, working directory is directory where cart and save data will be stored. Use this to provide
      an optional, alternative directory.)
  -S (Use to rip only save game data, and quit)
  -s (Use to rip both save game, and ROM data, and quit.)
  -z (Use to manually specify size of SRAM in cart. Measured by Kilobits.)
