import smbus
import time

def readData():
 return cart.read_byte_data(_SNESBankAndData,GPIOB)

def gotoAddr(addr):
 return cart.write_byte_data(_SNESAddressPins,GPIOA,addr)

def readAddr(addr):
 gotoAddr(addr) 
 return readData()

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


for x in range(192, 212):
 cartname += chr( readAddr(x) )

ROMmakeup =  readAddr(213)
ROMspeed = getUpNibble(ROMmakeup)
bankSize = getLowNibble(ROMmakeup)


ROMtype   =  readAddr(214)
ROMsize   =  readAddr(215)
SRAMsize  =  readAddr(216)
country   =  readAddr(217)
license   =  readAddr(218)
version   =  readAddr(219)


inverseChecksum = 0
for x in range(220, 221):
 inverseChecksum  += readAddr(x)

ROMchecksum = 0
for x in range(222, 223):
 ROMchecksum  += readAddr(x)

print "Game Title:         " + cartname
print "ROM Makeup:         " + str(ROMmakeup)
print " - ROM Speed:       " + str(ROMspeed)
print " - Bank Size:       " + str(bankSize)
print "ROM Type:           " + str(ROMtype)
print "ROM Size:           " + str(ROMsize)
print "Country:            " + str(country)
print "License:            " + str(license)
print "Version:            " + str(license)
print "Inverse Checksum:   " + str(inverseChecksum)
print "ROM Checksum:       " + str(ROMchecksum)



#--- Clean Up & End Script ------------------------------------------------------
gotoAddr(00)
cart.write_byte_data(_IOControls,GPIOA,0x10) #Turn off MOSFET
 
