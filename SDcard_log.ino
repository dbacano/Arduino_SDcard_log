//------------------------------------------------------------------------------------
//Libraries
//------------------------------------------------------------------------------------

#include <SD.h>
#include <SPI.h>
#include <Ethernet.h>
#include <OneWire.h>
#include <avr/wdt.h>

//------------------------------------------------------------------------------------
//Define pin configuration on arduino ethernet and initializing variables
//------------------------------------------------------------------------------------

int DS18S20_Pin = 2; //DS18S20 Signal pin on digital 2
OneWire ds(DS18S20_Pin); // on digital pin 2

const int plogbutton = 7; 

const int ptensao = A0;
const int pshunt1 = A1;
const int pshunt2 = A2;
const int pshunt3 = A3;
const int pshunt4 = A4;

float refresh_rate = 85.0;  //Dataloger Refresh Rate
long id = 0;               //Use this to store the id # of our reading.

int CS_pin = 4;

long temperature=0;
long tensao=0;
long shunt1=0;
long shunt2=0;
long shunt3=0;
long shunt4=0;
unsigned long tempo=0;
unsigned long newtime=0;
unsigned long elapsed=0;
unsigned long prenewtime=0;
unsigned long pretime=0;
File logFile;

int i=0;

//------------------------------------------------------------------------------------
//Setup
//------------------------------------------------------------------------------------

void setup(){

  pinMode(7,INPUT);
  pinMode(A0,INPUT);
  pinMode(A1,INPUT);
  pinMode(A2,INPUT);
  pinMode(A3,INPUT);
  pinMode(A4,INPUT);
  
  Serial.begin(9600);
  
 // watchdogSetup();
  Serial.println("Watchdog is watching!");
//------------------------------------------------------------------------------------
//Check SD Card
//------------------------------------------------------------------------------------

  Serial.print("Initializing SD card...");
    // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
    // Note that even if it's not used as the CS pin, the hardware SS pin 
    // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
    // or the SD library functions will not work. 
    pinMode(4, OUTPUT);
  
    if (!SD.begin(4)) {
      Serial.println("initialization failed!");
      return;
    }
    Serial.println("initialization done.");
  
    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
  
//--------------------------------------------------------------------------------  
//Write Log File Header
//--------------------------------------------------------------------------------
  
  logFile = SD.open("LOG.csv", FILE_WRITE);
  if (logFile)
  {
    logFile.println(" , , , , , , , "); 
    logFile.println("id, voltage, shunt1, shunt2, shunt3, shunt4, temperature, time");
    logFile.close();

  }
  else
  {
    Serial.println("Couldn't open log file 1");
  }

  Serial.println("id, voltage, shunt1, shunt2, shunt3, shunt4, temperature, time");
  prenewtime=millis();

} 
//-------------------------------------------------------------------------   
//Start Loop
//-------------------------------------------------------------------------

void loop(){
  
//-------------------------------------------------------------------------   
//Write SDcard
//-------------------------------------------------------------------------

  wdt_reset();
  elapsed = millis()-newtime;
  if (elapsed>=refresh_rate){
    newtime=millis();
    
    
      temperature=temperature+(long) getTemp();
      tensao=tensao+(long) get_tensao();
      shunt1=shunt1+(long) get_shunt1();
      shunt2=shunt2+(long) get_shunt2();
      shunt3=shunt3+(long) get_shunt3();
      shunt4=shunt4+(long) get_shunt4();
      i++;
      Serial.println(i);
  }
   pretime=millis()-prenewtime;

if(pretime>=60000){  

      temperature=temperature/i;
      tensao=tensao/i;
      shunt1=shunt1/i;
      shunt2=shunt2/i;
      shunt3=shunt3/i;
      shunt4=shunt4/i;
      tempo=millis();
      id++;
      i=0;

      logFile = SD.open("LOG.csv", FILE_WRITE);
      if (logFile){
        logFile.print(id);
        logFile.print(",");
        logFile.print(tensao);
        logFile.print(",");
        logFile.print(shunt1);
        logFile.print(",");
        logFile.print(shunt2);
        logFile.print(",");
        logFile.print(shunt3);
        logFile.print(",");
        logFile.print(shunt4);
        logFile.print(",");
        logFile.print(temperature);
        logFile.print(",");
        logFile.print(tempo);
        logFile.println();
        logFile.close();
       
        Serial.print(id);
        Serial.print(",   ");
        Serial.print(tensao);
        Serial.print(",   ");
        Serial.print(shunt1);
        Serial.print(",   ");
        Serial.print(shunt2);
        Serial.print(",   ");
        Serial.print(shunt3);
        Serial.print(",   ");
        Serial.print(shunt4);
        Serial.print(",   ");
        Serial.print(temperature);
        Serial.print(",   ");
        Serial.print(tempo);
        Serial.println();
        }
      else{
        Serial.println("Couldn't open log file");
      }
        prenewtime=millis();
  }
}

//-------------------------------------------------------------------------
//Read temperature
//-------------------------------------------------------------------------

float getTemp(){  //returns the temperature from one DS18S20 in DEG Celsius

  byte data[12];
  byte addr[8];

  if ( !ds.search(addr)) {
    //no more sensors on chain, reset search
    ds.reset_search();
    return -1000;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
    Serial.print("CRC is not valid!\n");
    return -1000;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
    Serial.print("Device is not recognized");
    return -1000;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // start conversion, with parasite power on at the end

  byte present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE); // Read Scratchpad

  
  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = ds.read();
  }
  
  ds.reset_search();
  
  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;
  
  return TemperatureSum;  
}

//--------------------------------------------------------------------------------------
//tensao
//--------------------------------------------------------------------------------------

float get_tensao(){
  int tensao_med=analogRead(ptensao);
  
  float tensao_sum=tensao_med*31/1023;

  return 1000*tensao_sum;
}

//--------------------------------------------------------------------------------------
//corrente 1
//--------------------------------------------------------------------------------------

float get_shunt1(){
  float shunt_1=analogRead(pshunt1);
  
  float shunts1=-(((5000*shunt_1/1024)-2500)/0.066);
  
  return shunts1;
}

//--------------------------------------------------------------------------------------
//corrente 2
//--------------------------------------------------------------------------------------

float get_shunt2(){
  float shunt_2=analogRead(pshunt2);
  
  float shunts2=-(((5000*shunt_2/1024)-2500)/0.066);
  
  return shunts2;
}

//--------------------------------------------------------------------------------------
//corrente 3
//--------------------------------------------------------------------------------------

float get_shunt3(){
  float shunt_3=analogRead(pshunt3);
  
  float shunts3=-(((5000*shunt_3/1024)-2500)/0.066);
  
  return shunts3;
}

//--------------------------------------------------------------------------------------
//corrente 4
//--------------------------------------------------------------------------------------

float get_shunt4(){
  float shunt_4=analogRead(pshunt4);
  
  float shunts4=(((5000*shunt_4/1024)-2500)/0.066);
  
  return shunts4;
}

//--------------------------------------------------------------------------------------
//Watchdog Timer
//--------------------------------------------------------------------------------------

void watchdogSetup(void)
{
cli();  // disable all interrupts
wdt_reset(); // reset the WDT timer
/*
WDTCSR configuration:
WDIE = 1: Interrupt Enable
WDE = 1 :Reset Enable
WDP3 = 0 :For 2000ms Time-out
WDP2 = 1 :For 2000ms Time-out
WDP1 = 1 :For 2000ms Time-out
WDP0 = 1 :For 2000ms Time-out
*/
// Enter Watchdog Configuration mode:
WDTCSR |= (1<<WDCE) | (1<<WDE);
// Set Watchdog settings:
WDTCSR = (1<<WDIE) | (1<<WDE) | (1<<WDP3) | (0<<WDP2) | (0<<WDP1) | (1<<WDP0);
sei();
}
