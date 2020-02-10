  
/* 
 * Code for an for an Arduino based four channel heater controller implemented as UNO compatible shield.
 * The system monitors ambient temperature and temperatures in up to four targets. The power to the four 
 * heaters associated to the targets is continuously adjusted to raise the target temperature above 
 * ambient by a configurable offset. Measured temperatures and power input to each heater can be logged 
 * to an SD card. See https://bitbucket.org/ddierick/heater-shield for details.
 *   
 * Copyright 2019 Diego Dierick. 
 * 
 * This program is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 *  
 * Credits:
 *
 * New LiquidCrystal Library by F. Malpartida is licensed under a Creative Commons Attribution-ShareAlike 3.0 Unported License. CC BY-SA 3.0
 * It is available at https://bitbucket.org/fmalpartida/new-liquidcrystal/wiki/Home.
 *
 * The Arduino SdFat Library is licenced under the MIT license. Copyright (C) 2011-2017 Bill Greiman. It is available at https://github.com/greiman.
 * Code snippets to read floats from SD card are taken from the examples that come with the SdFat library.
 * 
 * The Adafruit RTClib is licensed under the MIT License. Copyright Adafruit Industries 2019.  This library can be found at https://github.com/adafruit/RTClib.
 */  

#include <SdFat.h>                      // SD card library
#include <Wire.h>                       // I2C for RTC and LCD display     
#include <OneWire.h>                    // OneWire for DS1820
#include <EEPROM.h>                     // built-in EEPROM
#include <LiquidCrystal_I2C.h>          // LCD with I2C backpack
#include <RTClib.h>                     // real time clock library for PCF8523
#include <avr/interrupt.h>              // interrupts
#include <avr/wdt.h>                    // watchdog
#include "_05_PIDunit.h"                // prototypes PIDunit class

const byte pinIntRTC = 2;               // pins interrupt RTC, SD chip select
const byte cs = 10;                      

DateTime timeStamp;                    // simple date/time class part of the RTClib
char charBuffer[28];                        // char array for SD output and timestamp formatting

char systemID[9];                      // system level configuration parameters (8chars and terminator)
float kp;                              // unit specific config variables are part of PIDunit class
float ki;
float kd;
unsigned int windUp;
unsigned int highTemp;

byte errorCountWDT;                    // system level errors 
byte errorCountSD;                     // unit level errors (sensor and temperature limit errors) are part of the PIDunit class

boolean syncFlag;                      // flag set when RTC interrupt triggers


RTC_PCF8523 rtc;                                                          // instantiate object class FCP8523
LiquidCrystal_I2C i2cLCD(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);         // set LCD I2C address and pin order (address may vary)
SdFat sdCard;                                                             // instantiate SD file system
SdFile cardFile;                                                          // file instance alternately used to access config, profile and datalog

PIDunit unit0;                                                            // create four PID units (unit0 will be the reference temp unit without control)
PIDunit unit1;                                                            // they will later be initialized with an ID, sensor address and temperature offset            
PIDunit unit2; 
PIDunit unit3; 
PIDunit unit4;  


void setup(){
  pinMode(pinIntRTC, INPUT_PULLUP);                                       // interrupt pin uses internal pullup
  pinMode(cs, OUTPUT);                                                    // set up SD card slave select pin (probably doen in SD library anyway)    
  
  Wire.begin();                                                           // begin I2C bus
  i2cLCD.begin(16,2);                                                     // initialize 16 by 2 LCD  
  createCustomSymbols();
  
  rtc.begin();                                                            // initialize PCF8523 RTC
  rtc.writeSqwPinMode(PCF8523_SquareWave1HZ );                            // 1Hz out
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));                       // FOR DEBUGGING
  
  SdFile::dateTimeCallback(fileTimeStamp);                                // to provide file timestamps see SD tab

  if (!sdCard.begin(cs,SPI_HALF_SPEED)){                                  // halt system if no SD card found
    haltSystemWithMessage(F("No SD card found"));
  }                                  
  
  if (cardFile.open("config.txt", FILE_READ)){                            // if available, read configuration from SD and write to EEPROM
    getConfigSD();
    cardFile.close();
    sdCard.remove("config.txt");  
    writeConfigEEPROM();                          
  }
      
  if (!getConfigEEPROM()){                                                // get latest configuration and initialize all units
    haltSystemWithMessage(F("No configuration"));                         // halt system if no configuration found in EEPROM
  }

  timeStamp = rtc.now(); 
  cardFile.open("datalog.txt", FILE_WRITE);
  writeHeaderDatalog();
  cardFile.close();
                             
  displaySetupScreen();                                                   // takes approx. 16 sec
  
  unit0.initSensor();                                                     // start DS1820 temperature conversions to have valid data ready
  unit1.initSensor();
  unit2.initSensor();
  unit3.initSensor();
  unit4.initSensor();
  
  cli();                                                                  //disable global interrupts
  MCUSR = 0x00;                                                           //clear reset flags status register (WDRF overrides WDE)
  WDTCSR = (1<<WDCE) | (1<<WDE);                                          //WDCE and WDE have to be set to 1 to modify WDTSCR
  WDTCSR = (0<<WDE) | (1<<WDIE) | (1<<WDP2) | (1<<WDP1) | (1<<WDP0);      //enable WDT interrupts at 2sec interval
  wdt_reset();                                                            //reset WDT
  sei();                                                                  //enable global interrupts

  attachInterrupt(0,syncToRTC,RISING);                                    // when all setup is done attach interrupt and set sleep mode 
}                                                                       


void loop(){
  timeStamp = rtc.now();                                                  // get RTC data
  displayRunScreen();                                                     // update LCD info every second

  unit0.updatePID(unit0.getT());                                          // update all PID units 
  unit1.updatePID(unit0.getT());
  unit2.updatePID(unit0.getT());
  unit3.updatePID(unit0.getT());
  unit4.updatePID(unit0.getT());
    
  if(timeStamp.second()%60==0){                                           // runs every minute
    if(sdCard.cardErrorCode()){                                           // if card error occurred (on previous write attempt) try to reinitialize card and update error count
      cardFile.close();                                                          
      sdCard.begin();                                                            
      errorCountSD++;
    }
    cardFile.open("datalog.txt", FILE_WRITE);                             // open file and write line of data
    writeLineDatalog();
    cardFile.close(); 
  }                                                                        

  doErrorCheck();                                                         // check errors and shut down if needed
  wdt_reset();                                                            // reset watchdog to avoid interrupt
  while(syncFlag == false) {delay(1);}                                    // wait for next second
  syncFlag = false;                                                       // re-init flag
}                                                                              


  

