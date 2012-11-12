import smbus
import time

def readData():
 return cart.read_byte_data(_SNESBankAndData,GPIOB)

def gotoAddr(addr):
 if addr <= 0xffff: 
  upByte = int(addr/256)
  lowByte = addr - (upByte * 256)
  cart.write_byte_data(_SNESAddressPins,GPIOB,upByte)
  cart.write_byte_data(_SNESAddressPins,GPIOA,lowByte)
 else:
  cart.write_byte_data(_SNESAddressPins,GPIOA,0x00)
  cart.write_byte_data(_SNESAddressPins,GPIOB,0x00)


def gotoBank(bank):
 cart.write_byte_data(_SNESBankAndData,GPIOA,bank)

def readAddr(addr):
 gotoAddr(addr) 
 return readData()

def readAddrBank(addr,bank):
 gotoBank(bank) 
 gotoAddr(addr)
 return readData()

def gotoOffset(offset,isLowROM):
 if isLowROM > 0:
  bank = int( offset / 0xFFFF)
  addr = offset - (bank * 0xFFFF) 
  gotoBank(bank)
  gotoAddr(addr)
 #else:
  
def readOffset(offset,isLowROM):
 gotoOffset(offset,isLowROM)
 return readData()

def compareROMchecksums(addr):
 currentAddr = addr + 28

 inverseChecksum = 0
 for x in range(currentAddr, currentAddr + 1):
  inverseChecksum  += readAddr(x)

 currentAddr = addr + 30
 ROMchecksum = 0
 for x in range(currentAddr, currentAddr + 1):
  ROMchecksum  += readAddr(x)

 if inverseChecksum + ROMchecksum == 255:
  return 1
 else:
  return 0
 

def getUpNibble(value):
 return int(value/16)

def getLowNibble(value):
 return ( value - (getUpNibble(value) * 16) )


# ------------ Setup Register Definitions ------------------------------------------
databyte = ""

_SNESAddressPins = 0x20 # MCP23017 Chip with SNES Address Pins
_SNESBankAndData = 0x22 # MCP23017 Chip with SNES Bank and Data
_IOControls      = 0x23 # MCP23017 Chip to control SNES IO Controls including MOSFET Power

IODIRA = 0X00
IODIRB = 0X01
GPIOA  = 0X12
GPIOB  = 0X13

# ------------- Set Registers -----------------------------------------------------

cart = smbus.SMBus(0)

cart.write_byte_data(_SNESAddressPins,IODIRA,0x00) # Set MCP bank A to outputs (SNES Addr 0-7)
cart.write_byte_data(_SNESAddressPins,IODIRB,0x00) # Set MCP bank B to outputs (SNES Addr 8-15)

cart.write_byte_data(_SNESBankAndData,IODIRA,0x00) # Set MCP bank A to outputs (SNES Bank 0-7)
cart.write_byte_data(_SNESBankAndData,IODIRB,0xFF) # Set MCP bank B to inputs  (SNES Data 0-7)

cart.write_byte_data(_IOControls,IODIRA,0x80) # Set MCP bank A to outputs; WITH EXCEPTION TO IRQ
# GPA0: /RD
# GPA1: /RESET
# GPA2: /WR
# GPA3: /CS
# GPA4: CART MOSFET
# GPA7: /IRQ 

cart.write_byte_data(_IOControls,IODIRB,0x00) # Set MCP bank B to outputs 

#----------------------------------------------------------------------------------------------------
cart.write_byte_data(_IOControls,GPIOA,0x04)
#-----------------------------------------------------

cartname = ""

cart.write_byte_data(_SNESAddressPins,GPIOB,0x7F)
headerAddr =0

if compareROMchecksums(32704) == 1:
 print "Checksum matched for lowROM. Assuming this is the case!"
 print ""
 headerAddr = 32704

elif compareROMchecksums(65472) == 1:
 headerAddr = 65472
 print "Checksum match for hiROM. Assuming this is the case!"
 print ""

else:
 print "Checksums did not match. Either no cart, or cart read error"

currentAddr = headerAddr

for x in range(headerAddr, (headerAddr + 20) ):
 cartname += chr( readAddr(x) )

ROMmakeup =  readAddr(headerAddr + 21)
ROMspeed = getUpNibble(ROMmakeup)
bankSize = getLowNibble(ROMmakeup)


ROMtype   =  readAddr(headerAddr + 22)
ROMsize   =  readAddr(headerAddr + 23)
SRAMsize  =  readAddr(headerAddr + 24)
country   =  readAddr(headerAddr + 25)
license   =  readAddr(headerAddr + 26)
version   =  readAddr(headerAddr + 27)

currentAddr = headerAddr + 28

inverseChecksum = 0
for x in range(currentAddr, currentAddr + 1):
 inverseChecksum  += readAddr(x)


currentAddr = headerAddr + 30
ROMchecksum = 0
for x in range(currentAddr, currentAddr + 1):
 ROMchecksum  += readAddr(x)

print "Game Title:         " + cartname
print "ROM Makeup:         " + str(ROMmakeup)
print " - ROM Speed:       " + str(ROMspeed)
print " - Bank Size:       " + str(bankSize)
print "ROM Type:           " + str(ROMtype)
print "ROM Size:           " + str(ROMsize)
print "SRAM Size:          " + str(SRAMsize)
print "Country:            " + str(country)
print "License:            " + str(license)
print "Version:            " + str(version)
print "Inverse Checksum:   " + str(inverseChecksum)
print "ROM Checksum:       " + str(ROMchecksum)



#--- Clean Up & End Script ------------------------------------------------------
gotoAddr(00)
gotoBank(00)


cart.write_byte_data(_SNESAddressPins,IODIRA,0xFF) # Set MCP bank A to outputs (SNES Addr 0-7)
cart.write_byte_data(_SNESAddressPins,IODIRB,0xFF) # Set MCP bank B to outputs (SNES Addr 8-15)

cart.write_byte_data(_SNESBankAndData,IODIRA,0xFF) # Set MCP bank A to outputs (SNES Bank 0-7)
cart.write_byte_data(_SNESBankAndData,IODIRB,0xFF) # Set MCP bank B to inputs  (SNES Data 0-7)




cart.write_byte_data(_IOControls,IODIRA,0xEF) # Set MCP bank A to outputs; WITH EXCEPTION TO IRQ

cart.write_byte_data(_IOControls,GPIOA,0x10) #Turn off MOSFET
 
