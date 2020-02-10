#include <OneWire.h>
#include <LiquidCrystal_I2C.h>          // LCD with I2C backpack

// Looks for DS1820 on dsand displays address permanently on LCD
// Cycle power for new sensor

byte addr[8];
byte i;
 
OneWire  microLAN(8);                                                 // on pin 8 with pull-up
LiquidCrystal_I2C i2cLCD(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);     // set LCD I2C address and pin order (some use address 0x27)

void setup(void)
  {
  Serial.begin(9600);
  i2cLCD.begin(16,2);                                                     // initialize 16 by 2 LCD  
  i2cLCD.clear();
  delay(500);
  }

void loop(void)
  {
  microLAN.search(addr);
  for( i = 0; i < 8; i++)
    {
    if(addr[i]<16){i2cLCD.print(F("0"));}
    i2cLCD.print(addr[i], HEX);
    }
  while (true) {delay(1000);}
  }
