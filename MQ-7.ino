/************************Hardware Related Macros************************************/
#define MQ_PIN                       (0)       // Define analog input channel for MQ-7
#define RL_VALUE                     (10)      // Load resistance on the board (in kilo ohms)
#define RO_CLEAN_AIR_FACTOR          (27)      // Clean air factor for MQ-7 sensor

/***********************Software Related Macros************************************/
#define CALIBRATION_SAMPLE_TIMES     (50)      // Number of samples for calibration
#define CALIBRATION_SAMPLE_INTERVAL  (500)     // Interval between calibration samples in milliseconds
#define READ_SAMPLE_INTERVAL         (50)      // Interval between normal operation samples
#define READ_SAMPLE_TIMES            (5)       // Number of samples for normal operation

/**********************Application Related Macros**********************************/
#define CO_GAS                       (0)

/*****************************Globals***********************************************/
float COCurve[3]  = {2.3, 0.72, -0.34};        // CO curve {log10(x1), log10(y1), slope}
float Ro = 10;                                 // Initialize Ro to 10 kilo ohms

void setup() {
  Serial.begin(9600);                          // Initialize serial communication
  Serial.print("Calibrating...\n");
  Ro = MQCalibration(MQ_PIN);                  // Calibrate the sensor in clean air
  Serial.print("Calibration done...\n");
  Serial.print("Ro = ");
  Serial.print(Ro);
  Serial.print(" kohm\n");
}

void loop() {
  float rs = MQRead(MQ_PIN);                   // Get sensor resistance Rs
  float rs_ro_ratio = rs / Ro;                 // Calculate Rs/Ro ratio
  
  int ppm = MQGetCO_PPM(rs_ro_ratio);          // Get CO concentration in ppm
  Serial.print("CO Concentration: ");
  Serial.print(ppm);
  Serial.print(" ppm\n");
  
  delay(1000);
}

/****************** MQResistanceCalculation ****************************************
Input:   raw_adc - raw value read from ADC
Output:  the calculated sensor resistance
************************************************************************************/
float MQResistanceCalculation(int raw_adc) {
  return ((float)RL_VALUE * (1023 - raw_adc) / raw_adc);
}

/***************************** MQCalibration ****************************************
Input:   mq_pin - analog channel
Output:  Ro of the sensor
************************************************************************************/
float MQCalibration(int mq_pin) {
  int i;
  float val = 0;

  for (i = 0; i < CALIBRATION_SAMPLE_TIMES; i++) {   // Take multiple samples
    val += MQResistanceCalculation(analogRead(mq_pin));
    delay(CALIBRATION_SAMPLE_INTERVAL);
  }
  val = val / CALIBRATION_SAMPLE_TIMES;              // Calculate average
  val = val / RO_CLEAN_AIR_FACTOR;                   // Divide by clean air factor
  
  return val;
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

  rs = rs / READ_SAMPLE_TIMES;

  return rs;
}

/*****************************  MQGetCO_PPM **********************************
Input:   rs_ro_ratio - Rs divided by Ro
Output:  ppm of the CO gas
************************************************************************************/
int MQGetCO_PPM(float rs_ro_ratio) {
  // CO concentration calculation based on the sensor response curve
  return (int)pow(10, ((log10(rs_ro_ratio) - COCurve[1]) / COCurve[2]) + COCurve[0]);
}
