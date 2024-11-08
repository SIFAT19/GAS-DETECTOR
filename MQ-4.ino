/************************Hardware Related Macros************************************/
#define         MQ_PIN                       (0)     // Define which analog input channel you are going to use
#define         RL_VALUE                     (10)    // Define the load resistance on the board, in kilo ohms
#define         RO_CLEAN_AIR_FACTOR          (4.4)   // RO_CLEAR_AIR_FACTOR for MQ-4, derived from datasheet
 
/***********************Software Related Macros************************************/
#define         CALIBARAION_SAMPLE_TIMES     (50)    // Define how many samples you are going to take in the calibration phase
#define         CALIBRATION_SAMPLE_INTERVAL  (500)   // Define the time interval (ms) between samples during calibration
#define         READ_SAMPLE_INTERVAL         (50)    // Define time interval (ms) between samples during normal operation
#define         READ_SAMPLE_TIMES            (5)     // Define how many samples to average in normal operation
 
/**********************Application Related Macros**********************************/
#define         GAS_CH4                      (0)     // Target gas: methane
 
/*****************************Globals***********************************************/
float           CH4Curve[3]  =  {2.3, 0.2, -0.45};   // Curve parameters for CH4 (methane) based on datasheet
                                                     // Data format: {x, y, slope}; from points (lg200, lg1.6), (lg10000, lg0.26)
float           Ro           =  10;                  // Initial Ro value for calibration
 
void setup() {
  Serial.begin(9600);                                // UART setup, baudrate = 9600 bps
  Serial.print("Calibrating...\n");                
  Ro = MQCalibration(MQ_PIN);                        // Calibrate sensor in clean air
  Serial.print("Calibration is done...\n"); 
  Serial.print("Ro=");
  Serial.print(Ro);
  Serial.print(" kohm\n");
}
 
void loop() {
   Serial.print("CH4 Concentration: "); 
   Serial.print(MQGetGasPercentage(MQRead(MQ_PIN)/Ro, GAS_CH4));
   Serial.print(" ppm\n");
   delay(200);
}
 
/****************** MQResistanceCalculation ****************************************
Input:   raw_adc - raw value read from ADC, representing the voltage
Output:  calculated sensor resistance
************************************************************************************/ 
float MQResistanceCalculation(int raw_adc) {
  return ((float)RL_VALUE * (1023 - raw_adc) / raw_adc);
}
 
/***************************** MQCalibration ****************************************
Input:   mq_pin - analog channel
Output:  Ro of the sensor
Remarks: Assumes the sensor is in clean air. Uses MQResistanceCalculation to find Rs
         and divides by RO_CLEAN_AIR_FACTOR.
************************************************************************************/ 
float MQCalibration(int mq_pin) {
  int i;
  float val = 0;

  for (i = 0; i < CALIBARAION_SAMPLE_TIMES; i++) {
    val += MQResistanceCalculation(analogRead(mq_pin));
    delay(CALIBRATION_SAMPLE_INTERVAL);
  }
  val = val / CALIBARAION_SAMPLE_TIMES;             // Calculate average
  return val / RO_CLEAN_AIR_FACTOR;                 // Divide by RO_CLEAN_AIR_FACTOR for Ro
}
 
/*****************************  MQRead *********************************************
Input:   mq_pin - analog channel
Output:  Rs of the sensor
************************************************************************************/ 
float MQRead(int mq_pin) {
  int i;
  float rs = 0;

  for (i = 0; i < READ_SAMPLE_TIMES; i++) {
    rs += MQResistanceCalculation(analogRead(mq_pin));
    delay(READ_SAMPLE_INTERVAL);
  }
 
  return rs / READ_SAMPLE_TIMES;
}
 
/*****************************  MQGetGasPercentage **********************************
Input:   rs_ro_ratio - Rs divided by Ro
         gas_id      - target gas type
Output:  ppm of the target gas
************************************************************************************/ 
int MQGetGasPercentage(float rs_ro_ratio, int gas_id) {
  if (gas_id == GAS_CH4) {
    return MQGetPercentage(rs_ro_ratio, CH4Curve);
  }
  return 0;
}
 
/*****************************  MQGetPercentage **********************************
Input:   rs_ro_ratio - Rs divided by Ro
         pcurve      - pointer to the target gas curve
Output:  ppm of the target gas
Remarks: Uses slope and a point of the line for logarithmic conversion.
************************************************************************************/ 
int MQGetPercentage(float rs_ro_ratio, float *pcurve) {
  return (pow(10, ((log(rs_ro_ratio) - pcurve[1]) / pcurve[2]) + pcurve[0]));
}
