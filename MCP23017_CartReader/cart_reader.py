import smbus
import time
import sys

def readData():
 return cart.read_byte_data(_SNESBankAndData,GPIOB)

def gotoAddr(addr,isLowROM):
 if addr <= 0xffff: 
  upByte = int(addr/256)
  lowByte = addr - (upByte * 256)
  
  if isLowROM != 0:
   upByte = upByte + 0x80
   #print "UpByte LowROM: " + str((upByte))
  #else:
  # print "UpByte hiROM: " + str((upByte))

  if gotoAddr.currentUpByte != upByte:
   cart.write_byte_data(_SNESAddressPins,GPIOB,upByte)
   gotoAddr.currentUpByte = upByte
   #time.sleep(0.05)
   #print "upByte: " + str( upByte )
   
  if gotoAddr.currentLowByte != lowByte: 
   cart.write_byte_data(_SNESAddressPins,GPIOA,lowByte)
   gotoAddr.currentLowByte = lowByte
   #time.sleep(.05)
   #print "lowByte: " + str( lowByte )
 else:
  cart.write_byte_data(_SNESAddressPins,GPIOA,0x00)
  cart.write_byte_data(_SNESAddressPins,GPIOB,0x00)

gotoAddr.currentAddr = -1
gotoAddr.currentUpByte = -1
gotoAddr.currentLowByte = -1

def gotoBank(bank):
 if bank != gotoBank.currentBank:
  cart.write_byte_data(_SNESBankAndData,GPIOA,bank)
  gotoBank.currentBank = bank
  
gotoBank.currentBank = -1

def readAddr(addr,isLowROM):
 gotoAddr(addr,isLowROM) 
 return readData()

def readAddrBank(addr,bank):
 gotoBank(bank) 
 gotoAddr(addr,0)
 return readData()

def gotoOffset(offset,isLowROM):

 if isLowROM == 0:
  bank = int( offset / 65536) #64Kilobyte pages
  addr = offset - (bank * 65536) #64kilobyte pages

 else:
  bank = int( offset / 32768)#32kilobyte pages
  addr = offset - (bank * 32768)#32kilobyte pages

 gotoBank(bank)
 gotoAddr(addr,isLowROM)
  
 gotoOffset.currentOffset = offset
 
gotoOffset.currentOffset = 0

  
def readOffset(offset,isLowROM):
 gotoOffset(offset,isLowROM)
 return readData()

def compareROMchecksums(header,isLowROM):
 if isLowROM == 1:
  cart.write_byte_data(_IOControls,GPIOA,0x06)#reset

 currentOffset = header + 28
 inverseChecksum  = readOffset(currentOffset,isLowROM)
 inverseChecksum += readOffset(currentOffset+1,isLowROM) * 256
 print "Inverse Checksum: " + str( hex(inverseChecksum) )

 currentOffset = header + 30

 ROMchecksum  = readOffset(currentOffset,isLowROM)
 ROMchecksum += readOffset(currentOffset+1,isLowROM) * 256
 print "Checksum: " + str(hex(ROMchecksum) )


 if (inverseChecksum ^ ROMchecksum) == 0xFFFF:
  return 1
 else:
  return 0
 

def getUpNibble(value):
 return int(value/16)

def getLowNibble(value):
 return ( value - (getUpNibble(value) * 16) )


currentBank = 0
currentAddr = 0
currentOffset = 0

def getROMsize(offset, isLowROM):
 ROMsizeRegister = readOffset(offset,isLowROM)
 ROMsizeRegister -= 7

 if ROMsizeRegister >=0:
  return  pow(2, ROMsizeRegister)
 else:
  return -1

def getNumberOfPages(actualROMsize,isLowROM):
 actualROMsize *= 2
 if isLowROM == 1:
  actualROMsize *= 2

 return actualROMsize



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
cart.write_byte_data(_IOControls,GPIOA,0x06)#reset
time.sleep(.25)
#cart.write_byte_data(_IOControls,GPIOA,0x04)
#-----------------------------------------------------

cartname = ""

headerAddr =32704
isLowROM = 1
isValid = 0

if compareROMchecksums(32704,1) == 1:
 print "Checksums matched"
 ROMmakeup =  readAddr(headerAddr + 21,isLowROM)
 ROMspeed = getUpNibble(ROMmakeup)
 bankSize = getLowNibble(ROMmakeup)

 if bankSize == 0:
   print "Checksum match for LoROM. Assuming this is the case!"
   isLowROM = 1
   isValid = 1
 elif bankSize == 1:
   print "Checksum match for HiROM. Assuming this is the case!"
   isLowROM = 0
   isValid = 1
 else:
  print "Bank Configuration Read Error"
else:
 print "Checksums did not match. Either no cart, or cart read error"

currentAddr = headerAddr
gotoOffset(headerAddr, isLowROM)

for x in range(headerAddr, (headerAddr + 20) ):
 cartname += chr( readAddr(x,isLowROM) )

ROMmakeup =  readAddr(headerAddr + 21,isLowROM)
ROMspeed = getUpNibble(ROMmakeup)
bankSize = getLowNibble(ROMmakeup)


ROMtype   =  readAddr(headerAddr + 22,isLowROM)
ROMsize   =  getROMsize(headerAddr + 23, isLowROM)
SRAMsize  =  readAddr(headerAddr + 24,isLowROM)
country   =  readAddr(headerAddr + 25,isLowROM)
license   =  readAddr(headerAddr + 26,isLowROM)
version   =  readAddr(headerAddr + 27,isLowROM)

currentAddr = headerAddr + 28
inverseChecksum  = readAddr(currentAddr,isLowROM)
inverseChecksum += readAddr(currentAddr+1,isLowROM) * 256

currentAddr = headerAddr + 30

ROMchecksum  = readAddr(currentAddr,isLowROM)
ROMchecksum += readAddr(currentAddr+1,isLowROM) * 256

numberOfPages = getNumberOfPages(ROMsize,isLowROM)


print "Game Title:         " + cartname
print "ROM Makeup:         " + str(ROMmakeup)
print " - ROM Speed:       " + str(ROMspeed)
print " - Bank Size:       " + str(bankSize)
print "ROM Type:           " + str(ROMtype)
print "ROM Size:           " + str(ROMsize) + " Mbits"
print "SRAM Size:          " + str(SRAMsize)
print "Country:            " + str(country)
print "License:            " + str(license)
print "Version:            " + str(version)
print "Inverse Checksum:   " + str(hex(inverseChecksum))
print "ROM Checksum:       " + str(hex(ROMchecksum))
print " - Checksums xOr'ed: " + str( hex(inverseChecksum | ROMchecksum) )
print ""
print "Number of pages:    " + str( numberOfPages )

dump = ""
#for x in range(0, 0x100000):
# sys.stdout.write( hex( readOffset(x,isLowROM) ) +" " )
  #dump += chr(readOffset(x,isLowROM))
#timeEnd = time.time()
#print "It took " + str(timeEnd - timeStart) + "seconds to read cart"

y = 0
pageChecksum = 0
totalChecksum = 0
currentByte = 0


#isValid = 0
if isValid == 1:
 #cart.write_byte_data(_IOControls,GPIOA,0x04)
 timeStart = time.time()
 for x in range(0, numberOfPages):
  print "----Start Cart Read------"
  print "Current Bank:       " + str( gotoBank.currentBank )
  while x == gotoBank.currentBank:
   currentByte = readOffset(y,isLowROM)
   dump += chr(currentByte)
   pageChecksum += currentByte
   #sys.stdout.write( str( currentByte ) + " ")
   y += 1
 
  if isLowROM == 0 or (isLowROM == 1 and gotoBank.currentBank % 2 == 0):
   print " - Page Checksum:       " + str( pageChecksum ) 
   totalChecksum += pageChecksum
   pageChecksum = 0
   print ""
   print "Current Checksum:      " + str( totalChecksum ) + " | Hex: " + str( hex( totalChecksum ) )
   print "Header Checksum:       " + str(hex(ROMchecksum))


 timeEnd = time.time()
 print ""
 print "It took " + str(timeEnd - timeStart) + "seconds to read cart"

 f = open(cartname + '.smc','w')
 f.write(dump)
 f.close

 print ""
 print "Checksum:             " + str( totalChecksum ) + " | Hex: " + str( hex( totalChecksum ) )
 print "Header Checksum:      " + str(hex(ROMchecksum))


#--- Clean Up & End Script ------------------------------------------------------
gotoAddr(00,0)
gotoBank(00)


cart.write_byte_data(_SNESAddressPins,IODIRA,0xFF) # Set MCP bank A to outputs (SNES Addr 0-7)
cart.write_byte_data(_SNESAddressPins,IODIRB,0xFF) # Set MCP bank B to outputs (SNES Addr 8-15)

cart.write_byte_data(_SNESBankAndData,IODIRA,0xFF) # Set MCP bank A to outputs (SNES Bank 0-7)
cart.write_byte_data(_SNESBankAndData,IODIRB,0xFF) # Set MCP bank B to inputs  (SNES Data 0-7)




cart.write_byte_data(_IOControls,IODIRA,0xEF) # Set MCP bank A to inputs; WITH EXCEPTION TO MOSFET

cart.write_byte_data(_IOControls,GPIOA,0x10) #Turn off MOSFET
#cart.write_byte_data(_IOControls,IODIRA,0xFF) # Set MCP bank A to outputs; WITH EXCEPTION TO IRQ
 
