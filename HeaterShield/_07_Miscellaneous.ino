/*
 * This file is part of the sketch HeaterShield.ino. For licensing details please refer to HeaterShield.ino.
 *
 * Some small helper functions to format timestamps for SD/LCD and interrupt service routines
 */


char* formatTimeDate(void){                                                                          // format date and time
  sprintf(charBuffer,("%02u-%02u-%u %02u:%02u:%02u"), timeStamp.day(), timeStamp.month(),            // dd-mm-yyyy hh:mm:ss
          timeStamp.year(), timeStamp.hour(), timeStamp.minute(), timeStamp.second());
  //sprintf(charBuffer, ("%02u/%02u/%u %02u:%02u:%02u"), timeStamp.month(), timeStamp.day(),         // uncomment and remove lines above for mm/dd/yyyy hh:mm:ss
  //        timeStamp.year(), timeStamp.hour(), timeStamp.minute(), timeStamp.second());
  return charBuffer;                                                                                 
}


//String formatTimeDate(){                                                                             // format date and time
//  char charBuffer[22];
//  sprintf(charBuffer,("%02u-%02u-%u %02u:%02u:%02u"), timeStamp.day(), timeStamp.month(),            // dd-mm-yyyy hh:mm:ss
//          timeStamp.year(), timeStamp.hour(), timeStamp.minute(), timeStamp.second());
//  //sprintf(charBuffer, ("%02u/%02u/%u %02u:%02u:%02u"), timeStamp.month(), timeStamp.day(),         // uncomment and remove lines above for mm/dd/yyyy hh:mm:ss
//  //        timeStamp.year(), timeStamp.hour(), timeStamp.minute(), timeStamp.second());
//  return charBuffer;                                                                                 
//}


void syncToRTC(){                            // ISR on rising edge on RTC square wave output (runs once every second when attached)
  syncFlag = true;
}


ISR (WDT_vect){ 
  ++ errorCountWDT;                         // increment WDT error counter
}  
