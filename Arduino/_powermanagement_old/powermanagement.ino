

// constants won't change. They're used here to 
// set pin numbers:
const int MOSFET = 2;
const int ledPin =  13;      // the number of the LED pin
const int powerSwitch = 4;
const int PiTX = 3;

// variables will change:
int PowerState = 0;
int MOSFET_State = 0;


void setup() 
{
  Serial.begin(115200);
  
  pinMode(powerSwitch, INPUT);//Set Power Switch Pin as input
  digitalWrite(powerSwitch, HIGH); //Turns on internal Pullup Resistor

  pinMode(MOSFET, OUTPUT);
  digitalWrite(MOSFET, MOSFET_State);
  
  pinMode(PiTX,INPUT);//Sets Pi TX Pin as input. BIAS Pin with Ext Pull-Down Resistor
}

void loop()
{
  
  if (PowerState == 0) // If power is Off
   {
   if ( powerSwitchOn() ) //AND power Switch is On
     turnMOSFETon(); //Turn On System Power
   }
  else if (PowerState == 1 && (powerSwitchOn == false) )//If Power Is On, but Power Switch is off...
  {
    PowerOffViaSerial();//Send Pi commands to Power Off via serial connection
    delay(15000);//Wait for system to power down
    if (isPiOn() == false )//check to see if Pi has safely shut down
    {
    turnMOSFEToff();
    }
    else//If Pi is STILL on..wait
    {
     delay(30000 );
    }
  }
    
}


int powerSwitchOn(){
 return digitalRead(powerSwitch);}


int PowerOffViaSerial()
{  
     // $: "exit"
     Serial.write("exit");

     //*<sleep 1>
    delay(1000);

    //<ctrl+c>
    Serial.print(0x03, BIN);
  
    //*<sleep 2>
    delay(2000);
  
    //<username>
    Serial.write("powermanagement");
  
    //*<sleep 1>
    delay(1000);
  
    //<password>
    Serial.write("poweruser");
  
    //$: "poweroff"
    Serial.write("poweroff");

}    

void turnMOSFETon()
{  
      // turn Power on:    
    MOSFET_State = 1; 
    digitalWrite(MOSFET, MOSFET_State);  
    delay(35000);
}

void turnMOSFEToff()
{
      // turn Power off:    
    MOSFET_State = 0; 
    digitalWrite(MOSFET, MOSFET_State);  
    delay(2000);  
}



boolean isPiOn()
{
 int i =0;
 for(i=0;i<10;i++)
 {
  if(digitalRead(PiTX) == HIGH)
   return(true);
  else
   delay(50); 
 }

 return(false);
}
  
  
  


