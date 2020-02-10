/*
 * This file is part of the sketch HeaterShield.ino. For licensing details please refer to HeaterShield.ino.
 * 
 * Functions to read system configuration from SD card and read and write configuration to/from EEPROM. 
 * Note that EEPROM is not filled in a continuous fashion, some gaps occur. 
 */


void getConfigSD(){
  unsigned int years,mons,days,hours,mins,secs;
  float tmpfloat;
  byte tmpbytes[8];
  
  csvReadUint16(&cardFile, &days, '-');                      // parse date and time format dd-mm-yyyy hh:mm:ss
  csvReadUint16(&cardFile, &mons, '-');                      // change order of timestamp elements and seperators to 
  csvReadUint16(&cardFile, &years, ' ');                     // use other format (e.g. mm/dd/yyyy hh:mm:ss)
  csvReadUint16(&cardFile, &hours, ':');                   
  csvReadUint16(&cardFile, &mins, ':');                    
  csvReadUint16(&cardFile, &secs, '\r');      
  rtc.adjust(DateTime(years,mons,days,hours,mins,secs));     // update system clock

  csvReadText(&cardFile, systemID, 9, '\r');                 // parse system ID
  csvReadFloat(&cardFile, &kp, '\r');                        // parse Kp, Ki, Kd
  csvReadFloat(&cardFile, &ki, '\r');                       
  csvReadFloat(&cardFile, &kd, '\r');                       
  csvReadUint16(&cardFile, &windUp, '\r');                   // parse windup limit and high temperature limit                 
  csvReadUint16(&cardFile, &highTemp, '\r');                       

  csvReadHex(&cardFile, tmpbytes, 16, '\r');                 // parse unit 0 sensor address
  unit0.init('0', tmpbytes);                                 // init unit 0 with name 0

  csvReadHex(&cardFile, tmpbytes, 16, ',');                  // parse unit 1 sensor address
  csvReadFloat(&cardFile, &tmpfloat, '\r');                  // parse unit 1 delta T
  unit1.init('A', tmpbytes, tmpfloat, 3);                    // init unit 1 with name A on pin 3

  csvReadHex(&cardFile, tmpbytes, 16, ',');                  // unit 2 named B on pin 5
  csvReadFloat(&cardFile, &tmpfloat, '\r');                 
  unit2.init('B', tmpbytes, tmpfloat, 5); 

  csvReadHex(&cardFile, tmpbytes, 16, ',');                  // unit 3 named C on pin 6
  csvReadFloat(&cardFile, &tmpfloat, '\r');                 
  unit3.init('C', tmpbytes, tmpfloat, 6); 

  csvReadHex(&cardFile, tmpbytes, 16, ',');                  // unit 4 named D on pin 9
  csvReadFloat(&cardFile, &tmpfloat, '\r');
  unit4.init('D', tmpbytes, tmpfloat, 9); 
}


boolean getConfigEEPROM(){
  int offset = 0;
  int tmpint;
  float tmpfloat;
  byte tmpbytes[8];
  
  if (EEPROM.read(offset+0) != 'C') {return false;}          // if no valid config info, return 0
  
  EEPROM.get(offset+1, systemID);
  EEPROM.get(offset+10, kp);
  EEPROM.get(offset+14, ki);
  EEPROM.get(offset+18, kd);
  EEPROM.get(offset+22, windUp);
  EEPROM.get(offset+24, highTemp);

  EEPROM.get(offset+35, tmpbytes);                           // get info unit 0 and initialize
  unit0.init('0', tmpbytes);                            

  EEPROM.get(offset+52, tmpbytes);                           // get info unit 1 and initialize on pin 3  
  EEPROM.get(offset+60, tmpfloat);
  unit1.init('A', tmpbytes, tmpfloat, 3);                        

  EEPROM.get(offset+73, tmpbytes);                           // get info unit 2 and initialize on pin 5
  EEPROM.get(offset+81, tmpfloat);
  unit2.init('B', tmpbytes, tmpfloat, 5);    
 
  EEPROM.get(offset+94, tmpbytes);                           // get info unit 3 and initialize on pin 6
  EEPROM.get(offset+102, tmpfloat);
  unit3.init('C', tmpbytes, tmpfloat, 6);  
  
  EEPROM.get(offset+115, tmpbytes);                          // get info unit 4 and initialize on pin 9
  EEPROM.get(offset+123, tmpfloat);
  unit4.init('D', tmpbytes, tmpfloat, 9);    
  
  return true;
}


void writeConfigEEPROM(){
  int offset = 0;
  int tmpint;
  float tmpfloat;
  byte tmpbytes[8];
  
  EEPROM.put(offset,'C');
  
  EEPROM.put(offset+1, systemID);
  EEPROM.put(offset+10, kp);
  EEPROM.put(offset+14, ki);
  EEPROM.put(offset+18, kd);
  EEPROM.put(offset+22, windUp);
  EEPROM.put(offset+24, highTemp);

  memcpy(tmpbytes,unit0.getSensorAddr(),8);                  // write info unit 0
  EEPROM.put(offset+35, tmpbytes);

  memcpy(tmpbytes,unit1.getSensorAddr(),8);                  // write info unit 1
  EEPROM.put(offset+52, tmpbytes);
  EEPROM.put(offset+60, unit1.getDeltaT());
  
  memcpy(tmpbytes,unit2.getSensorAddr(),8);                  // write info unit 2
  EEPROM.put(offset+73, tmpbytes);
  EEPROM.put(offset+81, unit2.getDeltaT());
  
  memcpy(tmpbytes,unit3.getSensorAddr(),8);                  // write info unit 3
  EEPROM.put(offset+94, tmpbytes);
  EEPROM.put(offset+102, unit3.getDeltaT());
  
  memcpy(tmpbytes,unit4.getSensorAddr(),8);                  // write info unit 4
  EEPROM.put(offset+115, tmpbytes);
  EEPROM.put(offset+123, unit4.getDeltaT());
}
  





