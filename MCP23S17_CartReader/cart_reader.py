import time
import sys
import os 
import getopt
from RPiMCP23S17.MCP23S17 import MCP23S17


def readData():
 return SNESBankAndData._readRegister(GPIOB)

def gotoAddr(addr,isLowROM):
 if addr <= 0xffff: 
  upByte = int(addr/256)
  lowByte = addr - (upByte * 256)
  gotoAddr.currentAddr = addr
  
  if isLowROM != 0:
   upByte = upByte | 0x80 # ORs a 1 to A15 if LoROM

  if gotoAddr.currentUpByte != upByte:
   SNESAddressPins._writeRegister(GPIOB,upByte)
   gotoAddr.currentUpByte = upByte
   
  if gotoAddr.currentLowByte != lowByte: 
   SNESAddressPins._writeRegister(GPIOA,lowByte)
   gotoAddr.currentLowByte = lowByte  
   
 else:
  SNESAddressPins._writeRegister(GPIOA,0x00)
  SNESAddressPins._writeRegister(GPIOB,0x00)
  gotoAddr.currentAddr = 0

gotoAddr.currentAddr = -1
gotoAddr.currentUpByte = -1
gotoAddr.currentLowByte = -1

def gotoBank(bank):
 if bank != gotoBank.currentBank:
  SNESBankAndData._writeRegister(GPIOA,bank)
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
  IOControls._writeRegister(GPIOA,0x06)#reset

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

def returnNULLheader():
 charStr = ""
 for x in range(0, 512):
  charStr += chr(0x00)
 return charStr

def CX4setROMsize(ROMsize):
 gotoOffset(0x007F52,0)
 ROMsizeRegister = readData()
 print "$007F52 offset reads    " + str( ROMsizeRegister )
 SNESBankAndData._writeRegister(IODIRB,0x00) # Set MCP bank B to outputs  (SNES Data 0-7)
 IOControls._writeRegister(GPIOA,0x03)#reset + /RD high
 # GPA0: /RD
 # GPA1: /RESET
 # GPA2: /WR
 # GPA3: /CS
 # GPA4: CART MOSFET
 # GPA7: /IRQ

 if ROMsize > 8:
  if ROMsizeRegister == 1:
   print "ROM is larger than 8 megs, writing 0x00 to CX4 register"
   SNESBankAndData._writeRegister(GPIOB,0x00)
  else:
   print "CX4 register is at correct value, will not change"

 else:
  if ROMsizeRegister == 1:  
   print "CX4 register is at correct value, will not change"
  else:
   print "ROM is 8 megs, writing 0x01 to CX4 register"
   SNESBankAndData._writeRegister(GPIOB,0x01)

 IOControls._writeRegister(GPIOA,0x06)#reset + /WR high
 SNESBankAndData._writeRegister(IODIRB,0xFF) # Set MCP bank B to back to inputs  (SNES Data 0-7)
 print "$007F52 offset now reads " + str( readData() )

 
 
def ripROM (startBank, isLowROM,numberOfPages):
  #cart.write_byte_data(_IOControls,GPIOA,0x04)
 ROMdump = ""
 pageChecksum = 0
 currentByte = 0
 bank = 0
 
 if isLowROM == 1:
  startOffset = startBank * 0x8000
 else:
  startOffset = startBank * 0x10000
 
 offset = startOffset  # Set current Offset to starting offset
 gotoOffset(startOffset,isLowROM)# Change current bank & address to offset

 print "----Start Cart Read------" 
 print ""
 #Start at current bank, and increment the number of banks needed
 for bank in range(startBank, (numberOfPages + startBank)  ): 
  print "Current Bank:  DEC: " + str( gotoBank.currentBank ) + "; HEX: " + str( hex(gotoBank.currentBank ))
  
  #If bank increments, exit the following inner loop, else keep scanning
  while bank == gotoBank.currentBank:
   currentByte = readData()
   ROMdump += chr(currentByte)
   pageChecksum += currentByte 
   offset += 1 #Increment offset
   gotoOffset(offset,isLowROM) #goto new offset
 
  if isLowROM == 0 or (isLowROM == 1 and gotoBank.currentBank % 2 == 0):
   print " - Page Checksum:       " + str( pageChecksum ) 
   ripROM.totalChecksum += pageChecksum
   pageChecksum = 0
   print ""
   print "Current Checksum:      " + str( ripROM.totalChecksum ) + " | Hex: " + str( hex( ripROM.totalChecksum ) )
   print "Header Checksum:       " + str(hex(ROMchecksum))
   print ""
  
 return ROMdump
 
ripROM.totalChecksum = 0

def ripSRAM(SRAMsize, ROMsize, isLowROM):
 SRAMdump = ""
 pageChecksum = 0
 currentByte = 0
 bank = 0 

 startBank = 0
 startAddr = 0
 endAddr   = 0x7FFF 
 
 if isLowROM == 1:
  startBank = 0x70  
  startAddr = 0x0000
  endAddr   = 0x7FFF
 else: #Else, is HiROM
  startBank = 0x30
  startAddr = 0x6000
  endAddr   = 0x7FFF
  IOControls._writeRegister(GPIOA,0x0E)# RESET + /WR + /CS high
  # GPA7: /IRQ | GPA4: CART MOSFET | GPA3: /CS | GPA2: /WR | GPA1: /RESET  | GPA0: /RD
  
 SRAMsize = (SRAMsize / 8.0) * 1024
 gotoBank(startBank)
 gotoAddr(startAddr,0)

 while SRAMsize > currentByte:
  currentByte += 1
  SRAMdump += chr( readData() )

  if gotoAddr.currentAddr >= endAddr:
   gotoBank( gotoBank.currentBank + 1)
   gotoAddr(startAddr,0)
  else:  
   gotoAddr( gotoAddr.currentAddr +1, 0)

 IOControls._writeRegister(GPIOA,0x06)#reset + /WR high
 # GPA7: /IRQ | GPA4: CART MOSFET | GPA3: /CS | GPA2: /WR | GPA1: /RESET  | GPA0: /RD

 print str(currentByte) + " SRAM bytes read"

 return SRAMdump



directory = ""
readSRAM = 0
readCart = 1
convertedSRAMsize = 0
val =0

try:
 opts, args = getopt.getopt(sys.argv[1:],"Ssz:d:",["directory="])
except getopt.GetoptError:
 print "Usage: cart_reader.py -d <optional directory> -S (Reads only SRAM)"
 sys.exit(2)
for opt, arg in opts:
 if opt in ("-d","--directory"):
  directory = arg
 if opt in ("-z"):
  val = int(arg)
  if val > 256 or val < 0:
   convertedSRAMsize = 0
  else:
   convertedSRAMsize = val
 elif opt in ("-S"):
  readSRAM = 1
  readCart = 0
 elif opt in ("-s"):
  readSRAM = 1
  readCart = 1



#if __name__ == "__main__":
#  main(sys.argv[1:])

# ------------ Setup Register Definitions ------------------------------------------
databyte = ""

#_SNESAddressPins = 0x20 # MCP23017 Chip with SNES Address Pins
#_SNESBankAndData = 0x22 # MCP23017 Chip with SNES Bank and Data
#_IOControls      = 0x23 # MCP23017 Chip to control SNES IO Controls including MOSFET Power

IODIRA = 0X00
IODIRB = 0X01
GPIOA  = 0X12
GPIOB  = 0X13
GPINTENB = 0x05
DEFVALB = 0x07
INTCONB = 0x09
IOCON_B = 0x0B
GPPUB   = 0x0D
# ------------- Set Registers -----------------------------------------------------

IOControls      = MCP23S17(bus=0x00, ce=0x00, deviceID=0x03)
SNESAddressPins = MCP23S17(bus=0x00, ce=0x00, deviceID=0x00)
SNESBankAndData = MCP23S17(bus=0x00, ce=0x00, deviceID=0x02)

IOControls.open()
SNESAddressPins.open()
SNESBankAndData.open()

#IOControls._writeRegister(IOCON_B,0x08)


SNESAddressPins._writeRegister(IODIRA,0x00) # Set MCP bank A to outputs (SNES Addr 0-7)
SNESAddressPins._writeRegister(IODIRB,0x00) # Set MCP bank B to outputs (SNES Addr 8-15)

SNESBankAndData._writeRegister(IODIRA,0x00) # Set MCP bank A to outputs (SNES Bank 0-7)
SNESBankAndData._writeRegister(IODIRB,0xFF) # Set MCP bank B to inputs  (SNES Data 0-7)

SNESBankAndData._writeRegister(GPPUB,0xFF) # Enables Pull-Up Resistors on MCP SNES Data 0-7
#SNESBankAndData._writeRegister(DEFVALB,0xFF) # Expect MCP SNES Data 0-7 to default to 0xFF
#cart.write_byte_data(_SNESBankAndData,GPINTENB,0xFF) # Sets up all of SNES Data 0-7 to be interrupt enabled
#SNESBankAndData._writeRegister(GPINTENB,0x89) # Sets up some of SNES Data 0-7 to be interrupt enabled

#cart.write_byte_data(_SNESBankAndData,INTCONB,0x00) # compares interrupts to previous SNES Data 0-7
#SNESBankAndData._writeRegister(INTCONB,0xFF) # compares interrupts to DEFVALB
                       



IOControls._writeRegister(IODIRA,0x80) # Set MCP bank A to outputs; WITH EXCEPTION TO IRQ

# GPA0: /RD
# GPA1: /RESET
# GPA2: /WR
# GPA3: /CS
# GPA4: CART MOSFET
# GPA7: /IRQ 

IOControls._writeRegister(IODIRB,0x00) # Set MCP bank B to outputs 

#----------------------------------------------------------------------------------------------------
IOControls._writeRegister(GPIOA,0x06)#reset
time.sleep(.25)
#cart.write_byte_data(_IOControls,GPIOA,0x04)

#-----------------------------------------------------

cartname = ""

headerAddr =32704
isLowROM = 1
isValid = 0

if compareROMchecksums(32704,1) == 1:
 print "Checksums matched"
 ROMmakeup =  readOffset(headerAddr + 21,isLowROM)
 ROMspeed = getUpNibble(ROMmakeup)
 bankSize = getLowNibble(ROMmakeup)

 if bankSize == 0:
   print "ROM Makeup match for LoROM. Assuming this is the case!"
   isLowROM = 1
   isValid = 1
 elif bankSize == 1:
   print "ROM Makeup match for HiROM. Assuming this is the case!"
   headerAddr = 65472
   isLowROM = 0
   isValid = 1
 else:
  print "Bank Configuration Read Error"
else:
 print "Checksums did not match. Either no cart, or cart read error"

#--- Debug. Manually set bank size ----------
#isLowROM = 1
#-------------------------------------------


currentAddr = headerAddr
gotoOffset(headerAddr, isLowROM)

for x in range(headerAddr, (headerAddr + 20) ):
 cartname += chr( readOffset(x,isLowROM) )

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

currentAddr = headerAddr + 32
VBLvector = readAddr(currentAddr,isLowROM)
VBLvector += readAddr(currentAddr+1,isLowROM) * 256

currentAddr = headerAddr + 34
resetVector = readAddr(currentAddr,isLowROM)
resetVector += readAddr(currentAddr+1,isLowROM) * 256



numberOfPages = getNumberOfPages(ROMsize,isLowROM)
#numberOfPages = 16

print "Game Title:         " + cartname
print "ROM Makeup:         " + str(ROMmakeup)
print " - ROM Speed:       " + str(ROMspeed)
print " - Bank Size:       " + str(bankSize)
print "ROM Type:           " + str(ROMtype)

if ROMtype == 243:
 print ""
 print "Capcom CX4 ROM Type detected!"
 CX4setROMsize(ROMsize)
 print ""
 

print "ROM Size:           " + str(ROMsize) + "  MBits"

print "SRAM Size:          " + "Value: " + str(SRAMsize),
if convertedSRAMsize == 0:
 if SRAMsize <= 12 and SRAMsize > 0:
  convertedSRAMsize  =  1<<(SRAMsize +3)
print " | " + str(convertedSRAMsize) +  " KBits"

print "Country:            " + str(country)
print "License:            " + str(license)
print "Version:            1." + str(version)
print "Inverse Checksum:   " + str(hex(inverseChecksum))
print "ROM Checksum:       " + str(hex(ROMchecksum))
print " - Checksums xOr'ed:   " + str( hex(inverseChecksum | ROMchecksum) )
print ""
print "VBL Vector:         " + str(VBLvector)
print "Reset Vector:       " + str(resetVector)
print ""
print "Number of pages:    " + str( numberOfPages )
print ""

dump = ""
dump = returnNULLheader()
y = 0
pageChecksum = 0
totalChecksum = 0
currentByte = 0
numberOfRemainPages = 0
firstNumberOfPages = 0



if directory != "" :
 if directory[len(directory)-1] != "/":
  directory += "/"

g = open("/tmp/insertedCart",'w')
if isValid == 1:
 g.write(cartname)

 if readCart == 1:
  if os.path.exists(directory + cartname + '.smc'):
   print "Cart has already been ripped, not ripping again!"
   readCart = 0
 elif readCart == 0:  
  print("Will not rip cart due to OPTs")
   
 if readCart == 1:   
  numberOfRemainPages = 0
  firstNumberOfPages = numberOfPages 
  timeStart = time.time()
  
  f = open(directory + cartname + '.smc','w')

 
  if isLowROM == 1:    
   print "Reading " + str(numberOfPages) + " Low ROM pages."

   dump = ripROM(0x00, isLowROM, firstNumberOfPages)
  
  else:
   if numberOfPages > 64:
    numberOfRemainPages = ( numberOfPages - 64 ) # number of pages over 64
    print "Reading first 64 of " + str(numberOfPages) + " Hi ROM pages."
    firstNumberOfPages = 64
   else:
    print "Reading " + str(numberOfPages) + " Hi ROM pages."

   dump = ripROM(0xC0, isLowROM, firstNumberOfPages) 

   if numberOfRemainPages > 0:
    print "Reading last " + str(numberOfRemainPages) + " of High ROM pages."
    dump += ripROM(0x40, isLowROM, numberOfRemainPages)

  print ""
  print "Entire Checksum:             " + str( hex( ripROM.totalChecksum ) )
  print ""
  print "Header Checksum:             " + str( hex( ROMchecksum ) )

  ripROM.totalChecksum = ( ripROM.totalChecksum & 0xFFFF )

  print "16-bit Generated Checksum:   " + str( hex( ripROM.totalChecksum ) )

  if ripROM.totalChecksum == ROMchecksum:
   print "--------------------------   CHECKSUMS MATCH!"
  else:
   print "----------WARNING: CHECKSUMS DO NOT MATCH: " +str( hex( ripROM.totalChecksum ) ) + " != " +  str( hex( ROMchecksum ) )
    
    
  timeEnd = time.time()
  print ""
  print "It took " + str(timeEnd - timeStart) + "seconds to read cart"

  f.write(dump)
  f.close
 
 if readSRAM == 1:
  f = open(directory + cartname + '.srm','w')

  timeStart = time.time()
  dump = ripSRAM(convertedSRAMsize,ROMsize,isLowROM)
  timeEnd = time.time()

  print ""
  print "It took " + str(timeEnd - timeStart) + "seconds to read SRAM Data"
  f.write(dump)
  f.close
  
else:
 g.write("NULL")
 g.close



#--- Clean Up & End Script ------------------------------------------------------
gotoAddr(00,0)
gotoBank(00)

SNESBankAndData._writeRegister(GPPUB,0x00) # Disables Pull-Up Resistors on MCP SNES Data 0-7
SNESBankAndData._writeRegister(DEFVALB,0xFF) # Expect MCP SNES Data 0-7 to default to 0xFF
SNESBankAndData._writeRegister(GPINTENB,0x00) # Sets up all of SNES Data 0-7 to be interrupt disabled

SNESAddressPins._writeRegister(IODIRA,0xFF) # Set MCP bank A to outputs (SNES Addr 0-7)
SNESAddressPins._writeRegister(IODIRB,0xFF) # Set MCP bank B to outputs (SNES Addr 8-15)

SNESBankAndData._writeRegister(IODIRA,0xFF) # Set MCP bank A to outputs (SNES Bank 0-7)
SNESBankAndData._writeRegister(IODIRB,0xFF) # Set MCP bank B to inputs (SNES Data 0-7)




IOControls._writeRegister(IODIRA,0xEF) # Set MCP bank A to inputs; WITH EXCEPTION TO MOSFET

IOControls._writeRegister(GPIOA,0x10) #Turn off MOSFET
#cart.write_byte_data(_IOControls,IODIRA,0xFF) # Set MCP bank A to outputs; WITH EXCEPTION TO IRQ

SNESAddressPins.close()
SNESBankAndData.close()
IOControls.close()
 
