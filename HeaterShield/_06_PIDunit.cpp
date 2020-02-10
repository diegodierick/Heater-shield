/*
 * This file is part of the sketch HeaterShield.ino. For licensing details please refer to HeaterShield.ino.
 *
 * Implementation of PID class constructor and member functions
 */

#include <Arduino.h>
#include "_05_PIDunit.h"
#include <OneWire.h>                            

extern float kp;                                                           // defined globally, use extern to make them accessible 
extern float ki;
extern float kd;
extern unsigned int windUp;
extern unsigned int highTemp;                                               

static OneWire microLAN(8);                                                // initiate OneWire bus on pin 8 (4k7 pull up)   // TBD place into constructor?           
const int maxErrorCount = 5;                                               // error count which will cause unit to halt


PIDunit::PIDunit(){}                                                       // constructor


void PIDunit::init(const char unit, const byte* address, const float dT, const char pin){
  id = unit;                                                               // copy single char unit name 
  for (int i=0; i<8; i++){sensorAddr[i] = address[i];}                     // copy DS1820 address
  deltaT = int(dT*100);                                                    // stored in 1/100ths of degC
  if(pinPWM != -1){                                                        // if PWM pin defined initialise PWM to zero
    pinPWM = pin;
    pinMode(pinPWM,OUTPUT);
    analogWrite(pinPWM,0);
  }
}


void PIDunit::updatePID(const float ambientT){  
  temp = readSensor(sensorAddr);
  initSensor();                                                               // start conversion for next pass PID loop
  if(temp != -999){
    errorPprevious = errorP;
    errorP = ambientT*100.0 - temp + deltaT;                                  // in 1/100ths degrees C
    errorI = errorI + errorP;
    errorD = errorP - errorPprevious;                                         // 1sec interval makes this sensitive to noise (smoothing)
    errorI = long(constrain(errorI, int(-windUp), int(windUp)));              // cast because windUp is unsigned int
    float tmp = kp*errorP + ki*errorI + kd*errorD;
    valuePWM = constrain(tmp,0,255);
    if(ambientT == -999){valuePWM = 0;}                                       // if ambient sensor returns invalid data we do not want to heat
    sumTemp = sumTemp + temp;
    sumPWM = sumPWM + valuePWM;
  }
  else{                                                                       // if sensor error switch off heater and increment error count
    valuePWM = 0;             
    errorCountSensor++;
    if (errorCountSensor > 250){errorCountSensor = 250;}                      // avoid error count overflow
    errorFlagSD = true;
  }
  
  if(temp > highTemp*100){                                                    // if temp exceeds limit
    errorCountLimit++;
    if (errorCountSensor > 250){errorCountSensor = 250;}                      // avoid error count overflow
  }
  
  if((errorCountSensor>maxErrorCount) | (errorCountLimit>maxErrorCount)){     // excessive errors 
    errorFlagHalt = true;
    valuePWM = 0;                                                             // don't put a negative flag value e.g. -99 in here as the PWM output will turn on
  }
  
  if(pinPWM != -1){analogWrite(pinPWM, valuePWM);}                            // write PWM if output pin defined
}


void PIDunit::initSensor(void){
  microLAN.reset();
  microLAN.select(sensorAddr);    
  microLAN.write(0x44); 
}

  
char PIDunit::getId(void){return id;}


byte* PIDunit::getSensorAddr(void){return sensorAddr;}


float PIDunit::getDeltaT(void){return constrain(deltaT,-100,9999)/100.0;}                   // deltas are positive, but -1 used for unit 0


float PIDunit::getT(void){
   if (temp==-999){return -999;}
   else {return temp/100.0;}
}


int PIDunit::getPWM(void){return valuePWM;}


float PIDunit::getAvgT(int samples){
  float tmp = float(sumTemp/samples/100.0);             
  sumTemp = 0;                                          
  return tmp;
}

  
int PIDunit::getAvgPWM(int samples){
  float tmp = int(sumPWM/samples/2.55);             
  sumPWM = 0;                               
  return tmp;
}


byte PIDunit::getErrorCountSensor(void){return errorCountSensor;}                           


byte PIDunit::getErrorCountLimit(void){return errorCountLimit;}                            


bool PIDunit::getErrorFlagSD(void){
  bool tmp = errorFlagSD; 
  errorFlagSD = false;                                  // auto-reset
  return tmp;
}                                    


bool PIDunit::getErrorFlagHalt(void){return errorFlagHalt;} 


void PIDunit::clearErrorCounts(void){                   // called every midnight to clear sporadic errors
  if (errorFlagHalt == false){                          // not cleared when more then maxErrorCount errors occurred
    errorCountSensor = 0;
    errorCountLimit = 0;
  }
}                                             


void PIDunit::clearErrorFlagSD(void){errorFlagSD = 0;}                                  

  
int PIDunit::readSensor(const byte* address){
  byte data[9];
  microLAN.reset();
  microLAN.select(address);    
  microLAN.write(0xBE); 
                                                                    
  for ( int i = 0; i < 9; i++){data[i] = microLAN.read();}                    // read 9 bytes from DS18B20 scratchpad
  int rawTemp = (data[1] << 8) | data[0];                                     // merge MSB and LSB temperature data (no need to mask anything in 12 bit mode)
  if (OneWire::crc8(data, 8) != data[8]){return -999;}                        // if CRC error return flag value
  else{return int(rawTemp/16.0*100.0);}                                       // if data valid return it (math differs for DS18S20)
}




