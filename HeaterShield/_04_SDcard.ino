/*
 * This file is part of the sketch HeaterShield.ino. For licensing details please refer to HeaterShield.ino.
 *
 * Functions to write header and data lines to SD card. The function to read configuration data from 
 * the SD card is located in the tab _01_Configuration, while the functions to read csv data from SD 
 * are located in this tab. SD read functions are based on the example ReadCsv.ino found in the SdFat 
 * library.
 */


void writeHeaderDatalog(){
  //char charBuffer[28];
  
  cardFile.print(F("Start time: "));                                           // print start time
  cardFile.println(formatTimeDate());
  cardFile.print(F("System name: "));                                          // print system name
  cardFile.println(systemID);
  cardFile.print(F("Kp: "));                                                   // print PID parameters
  cardFile.println(kp);
  cardFile.print(F("Ki: "));
  cardFile.println(ki);
  cardFile.print(F("Kd: "));
  cardFile.println(kd);
  cardFile.print(F("WindUp: "));
  cardFile.println(windUp);
  cardFile.print(F("HighTemp: "));
  cardFile.println(highTemp);
  cardFile.println();
  writeUnitInfo(unit0);                                                        // print setup units
  writeUnitInfo(unit1);
  writeUnitInfo(unit2);
  writeUnitInfo(unit3);
  writeUnitInfo(unit4);
  cardFile.println();

  sprintf(charBuffer,("TS,T_%c"), unit0.getId());                                                          // column names based on unit IDs
  cardFile.print(charBuffer);
  sprintf(charBuffer,(",T_%c,T_%c"), unit1.getId(), unit2.getId());
  cardFile.print(charBuffer);
  sprintf(charBuffer,(",T_%c,T_%c"), unit3.getId(), unit4.getId());
  cardFile.print(charBuffer);
  sprintf(charBuffer,(",PWM_%c,PWM_%c"), unit1.getId(), unit2.getId());                                    // identical formatting strings are only held in RAM once
  cardFile.print(charBuffer);
  sprintf(charBuffer,(",PWM_%c,PWM_%c"), unit3.getId(), unit4.getId());
  cardFile.println(charBuffer);
  cardFile.print(F("(dd-mm-yyyy hh:mm:ss),(degC),(degC),"));                                               // units data, split over two lines 
  cardFile.println(F("(degC),(degC),(degC),(%),(%),(%),(%)"));                                             // even with F macro the array is copied to RAM at one
}                                                                                                          // point and if too long it breaks things

void writeUnitInfo(PIDunit unitX){
  cardFile.print("Unit");
  cardFile.print(unitX.getId());
  cardFile.print(F("\t"));
  cardFile.print(constrain(unitX.getDeltaT(),0,99.9));                    // set -1 deltaT from reference unit0 to zero
  cardFile.print(F("\t"));
  for(int i=0; i<8; i++){
    byte tmp = *(unitX.getSensorAddr()+i);
    if(tmp<16){cardFile.print(F("0"));}
    cardFile.print(tmp,HEX);
  }
  cardFile.println();
}


void writeUnitAvgT(PIDunit* unitX){
  cardFile.print(F(","));
  if(unitX->getErrorFlagSD() == false){
    cardFile.print(unitX->getAvgT(60));      // average temperature last 60 seconds (clears temperature sum)
  }
  else{
    cardFile.print(F("NaN"));                
    unitX->getAvgT(60);                      // clear sum
  }
}  


void writeUnitAvgPWM(PIDunit* unitX){
  cardFile.print(F(","));
  if(unitX->getErrorFlagHalt() == false){
    cardFile.print(unitX->getAvgPWM(60));    // average PWM last 60 seconds (clears PWM sum)
  }
  else{
    cardFile.print(F("-99"));                // we use -99 to flag a heater was halted
    unitX->getAvgT(60);                      // clear sum
  }
}


void writeLineDatalog(){
  //char charBuffer[18];
  
  cardFile.print(formatTimeDate());
  writeUnitAvgT(&unit0);
  writeUnitAvgT(&unit1);
  writeUnitAvgT(&unit2);
  writeUnitAvgT(&unit3);
  writeUnitAvgT(&unit4);
  writeUnitAvgPWM(&unit1);
  writeUnitAvgPWM(&unit2);
  writeUnitAvgPWM(&unit3);
  writeUnitAvgPWM(&unit4);
  cardFile.println();
  //sprintf(charBuffer, (",%i,%i,%i,%i"), unit1.getAvgPWM(60), unit2.getAvgPWM(60), unit3.getAvgPWM(60), unit4.getAvgPWM(60));                                                
}

  
int csvReadText(SdFile* file, char* str, size_t size, char delim){
  char ch;
  int rtn;
  size_t n = 0;
  
  while (true){
    if (!file->available()){                    // check if EOF
      rtn = 0;
      break;
    }
    if (file->read(&ch, 1) != 1){               // if read error
      rtn = -1;
      break;
    }
    if (ch == '\r'){continue;}                  // skip CR character
    if (ch == delim || ch == '\n'){             // if delimiter or new line
      rtn = ch;
      break;
    }
    if ((n+1) >= size){                         // if string to long
      rtn = -2;
      n--;
      break;
    }
    str[n++] = ch;                              // add character to string
  }
  str[n] = '\0';                                // add terminator to string
  return rtn;                                   
}


int csvReadUint32(SdFile* file, uint32_t* num, char delim){
  char buf[20];
  char* ptr;
  int rtn = csvReadText(file, buf, sizeof(buf), delim);
  if (rtn < 0) return rtn;
  *num = strtoul(buf, &ptr, 10);
  if (buf == ptr) return -3;
  while(isspace(*ptr)) ptr++;
  return *ptr == 0 ? rtn : -4;
}


int csvReadUint16(SdFile* file, uint16_t* num, char delim){
  uint32_t tmp;
  int rtn = csvReadUint32(file, &tmp, delim);
  if (rtn < 0) return rtn;
  if (tmp > UINT_MAX) return -5;
  *num = tmp;
  return rtn;
}


int csvReadDouble(SdFile* file, double* num, char delim){
  char buf[20];
  char* ptr;
  int rtn = csvReadText(file, buf, sizeof(buf), delim);
  if (rtn < 0) return rtn;
  *num = strtod(buf, &ptr);
  if (buf == ptr) return -3;
  while(isspace(*ptr)) ptr++;
  return *ptr == 0 ? rtn : -4;
}


int csvReadFloat(SdFile* file, float* num, char delim){
  double tmp;
  int rtn = csvReadDouble(file, &tmp, delim);
  if (rtn < 0) return rtn;
  *num = tmp;
  return rtn;
}


int csvReadHex(SdFile* file, byte* num, size_t size, char delim){         // reads HEX characters and places corresponding bytes in array
  char buf[int(size)+1];                                                  // csvReadText appends a null terminator, so provide extra byte
  int rtn = csvReadText(file, buf, sizeof(buf), delim);
  for (byte i=0; i<8; i++){
    num[i] = charToNibble(buf[i*2])<<4 | charToNibble(buf[i*2+1]);
  }
  return rtn;
}


byte charToNibble(char c){                                                // no check implemented for valid hexadecimal input
  if (48<=c && c<=57){return(c-48);}                 
  else {return(c-55);}
}


void fileTimeStamp (uint16_t* fileDate, uint16_t* fileTime){                                    // callback function to timestamp SD files
  * fileDate = FAT_DATE(timeStamp.year(),timeStamp.month(),timeStamp.day());
  * fileTime = FAT_TIME(timeStamp.hour(),timeStamp.minute(),timeStamp.second());
}

