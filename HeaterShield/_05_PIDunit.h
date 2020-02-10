/*
 * This file is part of the sketch HeaterShield.ino. For licensing details please refer to HeaterShield.ino.
 *
 * Class declaration for a PID temperature controller unit. See _06_PIDunit.cpp for implementation.
 * Functions to get the unit temperature, implement PID control and set heater output.
 * A unit is initialized with parameters sensor address (8 bytes), T offset and PWM pin
 * If only a unitID and sensor address are defined the temperature offset is set to -1 and 
 * the PWM pin is never written to. This is how the reference/ambient unit 0 is used. 
 * For other units the temperature offset could be specified to be negative (e.g. -9 degC) to
 * effectively shut down the heater. 
 * 
 */


class PIDunit{
  
  public:  
    
  PIDunit();                                                                  // constructor
  void init(const char, const byte*, const float=-1.0, const char=-1);        // configure instance with unit name, sensor address, deltaT and PWMpin (no PWM pin for reference unit) 
  void updatePID(const float);                                                // update control loop and PWM output
  void initSensor(void);                                                      // initiate DS1820 conversion (can take up to 750ms to complete)
    
  char getId(void);                                       // member functions to get unit name, address and delta T
  byte* getSensorAddr(void);
  float getDeltaT(void);
    
  float getT(void);                                       // get temperature and PWM data for display purposes
  int getPWM(void);                                       
  float getAvgT(int);                                     // get average temperature and PWM (takes number of samples as argument and clears summing variables)
  int getAvgPWM(int);
    
  byte getErrorCountSensor(void);                         // return T sensor error count
  byte getErrorCountLimit(void);                          // return T limit error count
  bool getErrorFlagSD(void);                              // flag used to reflect sensor errors in SD writes, flag cleared when executed                        
  bool getErrorFlagHalt(void);                            // flag to shut down PID unit                         
  void clearErrorCounts(void);                            
  void clearErrorFlagSD(void);
   
  private:

  char id;                          // unit ID (fixed to values 0, A, B, C and D, see Configuration tab)
  byte sensorAddr[8];               // unit sensor address
  int deltaT;                       // internal representation of delta T in 1/100ths of a degree (signed to allow -1 default)
  char pinPWM;                      // PWM pin
  int temp;                         // unit temperature in 1/100ths of degC (allows +-320 degC range)
  int errorP;                       // P error in 1/100ths of degC (allows +-320 degC error)
  int errorPprevious;               // P error  at (t-1) in 1/100ths of degC (allows +-320 degC error)
  long errorI;                      // I error (allows +- 21E6 degC*dT  error integral), this term is typically constrained by the windup limit (global var)
  int errorD;                       // D error (allows +-320 degC/dT error change), due to the small 1 sec interval this term is sensitive to measurement noise                                            
                                    // consider averaging it over 10 sec or so (needs RAM) or apply a smoother
  byte valuePWM;                    // resulting control action (0-225)
  int sumPWM;                       // sum PWM values within 60sec recording interval (int can sum up to 255 PWM values)    
  long sumTemp;                     // sum temp values within 60sec recording interval (long as temp values are expressed in 1/100 degC) 
  
  byte errorCountSensor;            // number of sensor CRC errors
  byte errorCountLimit;             // number of temperature limit errors 
  bool errorFlagSD;                 // flags temperature sensor error since last SD write 
  bool errorFlagHalt;               // permanent error flag for shutdown
   
  int readSensor(const byte*);      // read DS1820 temperature
};


