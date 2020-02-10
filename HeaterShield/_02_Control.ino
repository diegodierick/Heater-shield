/*
 * This file is part of the sketch HeaterShield.ino. For licensing details please refer to HeaterShield.ino.
 *
 * Function to track system errors and clear errors at midnight as well as a function to halt the system with LCD warning.
 */

void doErrorCheck(){                                                                       // checks for excessive errors and if needed halts the system
  if (errorCountSD >= 5){haltSystemWithMessage(F("SD card errors"));}                      // SD related functions take a while to time-out in case of SD trouble and can meanwhile trigger several WDT errors
  if (errorCountWDT >= 60){haltSystemWithMessage(F("WDT errors"));}                        // so we provide a bit of room for WDT errors (amounts to ~120 sec total)                                                                            
                                                                                          
  if ((timeStamp.hour() == 0) && (timeStamp.minute() == 0) && (timeStamp.second() == 0)){  // reset error counts at midnight
    unit0.clearErrorCounts();
    unit1.clearErrorCounts();
    unit2.clearErrorCounts();
    unit3.clearErrorCounts();
    unit4.clearErrorCounts();
    errorCountWDT = 0;
    errorCountSD = 0;
  }
}

 
void haltSystemWithMessage(const __FlashStringHelper* errorMessage){
  i2cLCD.clear();                                                                         
  i2cLCD.print(errorMessage);                                                             
  i2cLCD.setCursor(0,1);                  
  i2cLCD.print(F("System halted")); 
  pinMode(3,OUTPUT);
  digitalWrite(3,LOW); 
  pinMode(5,OUTPUT);
  digitalWrite(5,LOW);  
  pinMode(6,OUTPUT);
  digitalWrite(6,LOW);  
  pinMode(9,OUTPUT);
  digitalWrite(9,LOW);       
  while(true){;}                          
}
 

