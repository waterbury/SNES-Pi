import smbus
import time
import sys
import os
import getopt

def bin(x):
 return ''.join(x & (1 << i) and '1' or '0' for i in range(7,-1,-1)) 


def readData():
 setPin(RD_PIN)
 clrPin(WR_PIN)
 commitPins()
 setDataDirection(1)
 return apu.read_byte_data(_SNES_APU,GPIOB)

def setDataDirection(direction):
 if direction == 1:
  if setDataDirection.current != direction:
   apu.write_byte_data(_SNES_APU, IODIRB,0xFF) # Set MCP bank B to inputs (DATA 0-7)
   setDataDirection.current = direction
 elif direction == 0:
  if setDataDirection.current != direction:
   apu.write_byte_data(_SNES_APU, IODIRB,0x00) # Set MCP bank B to outputs (DATA 0-7)
   setDataDirection.current = direction
setDataDirection.current = -1 

def writeStatus(dataByte):
 gotoAddr(2140)
 write_8bit(dataByte)

def readAddr(addr):
 gotoAddr(addr)
 return readData()

def gotoAddr(addr):
 setPin(WR_PIN)
 clrPin(RD_PIN)
 
 addr = addr & 0x03
 if (addr & 0x01) == 0x01:
  setPin(ADDR_0)
 else:
  clrPin(ADDR_0)

 if (addr & 0x02) == 0x02:
  setPin(ADDR_1)
 else:
  clrPin(ADDR_1)
 
 commitPins()

def setPin(pin):
 commitPins.pins = commitPins.pins | pin

def clrPin(pin):
 setPin(pin)
 commitPins.pins = commitPins.pins ^ pin

def commitPins():
  if commitPins.pins != commitPins.pins_previous:
   apu.write_byte_data(_SNES_APU, GPIOA,commitPins.pins)
   #print "Committed: ",
   #print bin(commitPins.pins) 
   commitPins.pins_previous = commitPins.pins
commitPins.pins = 0
commitPins.pins_previous = 0


def resetAPU():
  clrPin(RESET_PIN)
  setPin(RESET_LED)
  commitPins()
  time.sleep(.10)
  setPin(RESET_PIN)
  clrPin(RESET_LED)
  commitPins()
  time.sleep(.10)

def initAPU():
 commitPins.pins = 0
 commitPins()
 resetAPU()

def write_16bit(packet):
 packet_low = packet & 0xFF 
 packet_high = (packet & 0xFF00) >> 8
 
 gotoAddr(2142)
 write_8bit(packet_low)
 gotoAddr(2143)
 write_8bit(packet_high)
 

def write_8bit(dataByte):
 setPin(WR_PIN)
 clrPin(RD_PIN)
 commitPins()
 setDataDirection(0)
 apu.write_byte_data(_SNES_APU, GPIOB, dataByte) 

 


directory = ""

readSRAM = 0
readCart = 1
convertedSRAMsize = 0
val =0

try:
 opts, args = getopt.getopt(sys.argv[1:],"Ssz:d:",["directory="])
except getopt.GetoptError:
 print "Usage: apu_reader.py -d <optional directory> -S (Reads only SRAM)"
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

_SNES_APU = 0x20 # MCP23017 Chip 

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

apu = smbus.SMBus(0)

setDataDirection(1) # Set MCP bank B to inputs (DATA 0-7)
 
RESET_PIN = 0X01 # GPA0: /RESET
RESET_LED = 0X08 # GPA3: /RESET LED
RD_PIN = 0X10 # GPA4: RD_PIN
WR_PIN = 0X20 # GPA5: WR_PIN
ADDR_1 = 0X40 # GPA6: ADDR1
ADDR_0 = 0X80 # GPA7: ADDR0

STATUS = 2140
COMMAND = 2141
ADDR_HIGH = 2143
ADDR_LOW = 2142


apu.write_byte_data(_SNES_APU, IODIRA,0x00) # Set MCP bank A to outputs 




#----------------------------------------------------------------------------------------------------
initAPU()
time.sleep(.1)

#-----------------------------------------------------


timeStart = time.time()
print format( readAddr(2140), '02x')
print format( readAddr(2141), '02x')
writeStatus(0xcc)
print format( readAddr(2140), '02x')


for x in range(0, 16):
 writeStatus(x)#write incrementor to status
 gotoAddr(2141)#goto data addr
 write_8bit(x)#write data
 print format( readAddr(2140), '02x')
 

for x in range(0, 128):
 writeStatus(x)#write incrementor to status
 gotoAddr(2141)#goto data addr
 write_8bit(x)#write data
 print format( readAddr(2140), '02x')


#print format( readAddr(2142), '02x')

  
    
timeEnd = time.time()
print ""
print "It took " + str(timeEnd - timeStart) + "seconds to run"

  
 



#--- Clean Up & End Script ------------------------------------------------------
initAPU()

apu.write_byte_data(_SNES_APU,IODIRA,0xFF) # Set MCP bank A to outputs (SNES Addr 0-7)
apu.write_byte_data(_SNES_APU,IODIRB,0xFF) # Set MCP bank B to outputs (SNES Addr 8-15)
apu.write_byte_data(_SNES_APU,GPPUB,0x00) # Disables Pull-Up Resistors on SNES APU Data 0-7
#apu.write_byte_data(_SNES_APU,GPPUA,0x00) # Disables Pull-Up Resistors on SNES APU Pins

