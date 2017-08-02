#define POWSWITCH 4
#define PICMD 1 // Command to let Pi know intent to shutdown. Goes High when System should be shutdown
#define PIRX 2 //  Command from Pi. Will be low during operation of Pi. Will go high when Pi has been safely shut down


const int MOSFET = 3;
const int powerSwitch = 4;
uint32_t startTime = millis();
int flag = 0;

int PowerState = 0; //assume power is off
int MOSFET_State = 0;//start with MOSFET Disabled
int Invrt_pow = 1;//Drive P-Channel MOSFETs low to turn on. set to 1 to invert MOSFET control output. Else, 0 to make "On" Logic 1.


uint32_t timeToWait_Max = 120; //Maximum number of seconds to wait before turning off system
uint32_t timeToWait_AfterOff = 10; //Number of seconds to wait after system sent OK to power off

void turnMOSFEToff(int invertOut)
{
      // turn Power off:    
    MOSFET_State = 0; 
    PowerState = 0;
    
  
    if(invertOut)
     digitalWrite(MOSFET, 1);  
    else
     digitalWrite(MOSFET, MOSFET_State);  
     
     
    delay(2000);  
}

void turnMOSFETon(int invertOut){  
      // turn Power on:    
    MOSFET_State = 1; 
    PowerState = 1;
    
    if(invertOut)
     digitalWrite(MOSFET, 0);  
    else
     digitalWrite(MOSFET, MOSFET_State);  
       
    delay(10000);
    
   
}

int PiIsOn()
{
 int j =0;
 for(j=0;j<10;j++)
 {
  if(digitalRead(PIRX) == LOW)
   return(1);
  else
   delay(50); 
 }

 return(0);
}

int powerSwitchOn(){
  int i = 0;
  int j = 0; 
  
  for (i=0;i<10;i++){
 if (digitalRead(powerSwitch) == 0 ){
	return 1;
 }

   delay(60);
  }
  
  if(j >= 5)
   return 1;
 else
  return 0;
 }

void setup() 
{
	
	delay(5000);

    pinMode(powerSwitch, INPUT);//Set Power Switch Pin as input
    digitalWrite(powerSwitch, HIGH); //Enable Pull Up Resistor
  
 
  
  pinMode(MOSFET, OUTPUT);//Set MOSFET Control as an output
  turnMOSFEToff(Invrt_pow); //Turn off MOSFET
  pinMode(PICMD, OUTPUT);//Set Pi CMD as an output
  digitalWrite(PICMD, LOW);

  pinMode(PIRX, INPUT);//Set Pi RX as an input
  digitalWrite(PIRX, HIGH); //Enable Pull Up Resistor
  


  
}

void loop()
{
  
  if ( (PowerState == 0) && powerSwitchOn() ) // If power is Off AND power Switch is On
     turnMOSFETon(Invrt_pow); //Turn On System Power
     
  else if ( PowerState && (powerSwitchOn() == 0) ) //If Power Is On, but Power Switch is NOT in the on state...
  {
    digitalWrite(PICMD, HIGH);//Initialize Pi to shutdown
    startTime = millis();
    
    if (PiIsOn() == 1){ //Assume Pi is notifying power controller of it's state. Use this to know when to shut down
     while ( (millis() < (startTime + ( (timeToWait_Max - timeToWait_AfterOff) * 1000)))  && PiIsOn() );//Wait Maximum time until Pi Turns off
      delay(timeToWait_AfterOff * 1000);//After Pi Turns off, wait specified ammount of time
      }

    else
      delay(timeToWait_Max * 1000);//Wait Maximum Ammount of time

    digitalWrite(PICMD, LOW);
    turnMOSFEToff(Invrt_pow);//Turn Off System Power
    
    delay(500);
   }
   
delay(10);     
}

    
  
    


