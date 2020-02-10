/*
 * This file is part of the sketch HeaterShield.ino. For licensing details please refer to HeaterShield.ino.
 *
 * Functions implementing LCD sequences displayed during startup and while running. Unit level error messages are implemented here as well. 
 */


void createCustomSymbols(){
  const byte customChars[][8]={                   // custom characters for LCD
    { 0x04,0x0A,0x0E,0x0E,0x1F,0x1F,0x0E },       // thermometer
    { 0x00,0x01,0x03,0x07,0x0F,0x1F,0x1F },       // duty cycle heater
    { 0x18,0x18,0x03,0x04,0x04,0x04,0x03 },       // deg symbol
    //{ 0x06,0x09,0x04,0x0A,0x11,0x11,0x0E }      // delta symbol      
  };
  
  i2cLCD.createChar( 0, (byte *)customChars[0] );                         // create custom characters
  i2cLCD.createChar( 1, (byte *)customChars[1] );
  i2cLCD.createChar( 2, (byte *)customChars[2] );
  //i2cLCD.createChar( 3, (byte *)customChars[3] );                       // seems to crash stack on setup()
}

  
void displaySetupScreen(){
  i2cLCD.clear();                                                           
  i2cLCD.print(systemID);                                                       
  i2cLCD.setCursor(0,1);                                                        
  i2cLCD.print(formatTimeDate());      
  delay(2000);

  i2cLCD.clear();
  i2cLCD.print(F("Kp "));                
  i2cLCD.print(kp,2);
  i2cLCD.setCursor(9,0);
  i2cLCD.print(F("Ki "));                
  i2cLCD.print(ki,2);           
  i2cLCD.setCursor(0,1);                
  i2cLCD.print(F("Kd "));                
  i2cLCD.print(kd,2);
  delay(2000);

  i2cLCD.clear();
  i2cLCD.print(F("WindUp "));                
  i2cLCD.print(windUp);                                  
  i2cLCD.setCursor(0,1);
  i2cLCD.print(F("HighT "));                
  i2cLCD.print(highTemp);           
  delay(2000);
 
  displayUnitSetup(unit0);       
  delay(2000); 
  displayUnitSetup(unit1); 
  delay(2000);
  displayUnitSetup(unit2);
  delay(2000);
  displayUnitSetup(unit3); 
  delay(2000);
  displayUnitSetup(unit4);
  delay(2000);
}


void displayUnitSetup(PIDunit unitX){
  i2cLCD.clear();
  i2cLCD.print("Unit");
  i2cLCD.print(unitX.getId());                          // print unit name
  i2cLCD.setCursor(9,0);
  i2cLCD.write('d');                                    // proxy for delta symbol
  //i2cLCD.print(char(3));                              // delta symbool
  i2cLCD.print(F(" "));
  float tmpValue = unitX.getDeltaT();                   // desired temperature offset
  if (tmpValue < 10){i2cLCD.print(F(" "));}             // align numbers
  tmpValue = constrain(tmpValue,0,99.9);                // this will limit to xx.x digits
  i2cLCD.print(tmpValue,1);                             // print error 
  i2cLCD.print(char(2));                                // degC symbol  
  i2cLCD.setCursor(0,1);                                // next line
  for(int i=0; i<8; i++){
    byte tmp = *(unitX.getSensorAddr()+i);
    if(tmp<16){i2cLCD.print(F("0"));}
    i2cLCD.print(tmp,HEX);                              
  }
}


void displayUnitInfo(PIDunit unitX){
  i2cLCD.clear();                                                                       // clear LCD and print unit name and temperature
  i2cLCD.print("Unit");
  i2cLCD.print(unitX.getId()); 
  i2cLCD.setCursor(9,0);
  i2cLCD.print(char(0)); 
  i2cLCD.print(F(" "));              
  if (unitX.getT()==-999){i2cLCD.print(F("NaN"));} 
  else {i2cLCD.print(unitX.getT(),1);}                                                  
  i2cLCD.print(char(2));                                                                
  i2cLCD.setCursor(0,1); 
  
  if(unitX.getDeltaT()<0){i2cLCD.print(formatTimeDate());}                              // if unit 0 print time/date
  else{
    if(unitX.getErrorFlagHalt() == false){                                              // if unit not halted due to errors
      i2cLCD.print(char(1));
      i2cLCD.print(F(" "));
      int tmp = map(unitX.getPWM(),0,255,0,99);                                         // remap PWM value to 0-99% (avoiding 3 digits)
      if (tmp<10){i2cLCD.print(F(" "));}                                                // align
      i2cLCD.print(tmp);
      i2cLCD.print(F(" %"));
      
      i2cLCD.setCursor(9,1);
      i2cLCD.write(0xE3);                                                               // print error symbol
      if (unit0.getT()==-999 | unitX.getT()==-999){i2cLCD.print(F(" NaN"));}            // if sensor error print NaN
      else{
        float tmp2 = unit0.getT()+unitX.getDeltaT()-unitX.getT();                       // calculate temperature error
        tmp2 = constrain(tmp2,-99.9,99.9);                                              // this will limit to x.x digits
        if(tmp2>0){i2cLCD.print(F(" "));}                                               // add space if no leading - sign
        i2cLCD.print(tmp2,1);                                                           // print error 
      }                                                          
      i2cLCD.print(char(2)); 
    }
    else{                                                                                           // if unit is halted 
      i2cLCD.setCursor(9,0); 
      i2cLCD.print(F("Halted  "));                                                                  // modify top line
      i2cLCD.setCursor(0,1); 
      if (unitX.getErrorCountSensor() > 0){i2cLCD.print(F("Sensor errors"));}                       // indicate sensor erors (prioritized) or limit errors
      else {if (unitX.getErrorCountLimit() > 0){i2cLCD.print(F("T limit errors"));};}               // or limit errors
    }
  }
}


void displayRunScreen(){
  switch(timeStamp.second()%15) {                                                                   // alternate every 3 sec between unit info screens
    case 0:
      displayUnitInfo(unit0);
      break;
    case 3:
      displayUnitInfo(unit1);
      break;
    case 6:
      displayUnitInfo(unit2);
      break; 
    case 9:
      displayUnitInfo(unit3);
      break;
    case 12:
      displayUnitInfo(unit4);
      break; 
  }
}

