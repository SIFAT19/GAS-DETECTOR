/************************Hardware Related Macros************************************/
#define MQ_PIN                       (0)      // Define analog input channel
#define RL_VALUE                     (10)     // Load resistance on the board (in kilo ohms)
#define RO_CLEAN_AIR_FACTOR          (3.6)    // Clean air factor for MQ-135 sensor

/***********************Software Related Macros************************************/
#define CALIBARAION_SAMPLE_TIMES     (50)     // Number of samples for calibration
#define CALIBRATION_SAMPLE_INTERVAL  (500)    // Interval between calibration samples in milliseconds
#define READ_SAMPLE_INTERVAL         (50)     // Interval between normal operation samples
#define READ_SAMPLE_TIMES            (5)      // Number of samples for normal operation

/**********************Application Related Macros**********************************/
#define GAS_AMMONIA                  (0)
#define GAS_CO2                      (1)

/*****************************Globals***********************************************/
float NH3Curve[3]  = {1.5, 0.4, -0.37};  // Ammonia curve {log10(x1), log10(y1), slope}
float CO2Curve[3]  = {2.3, 0.72, -0.46}; // Adjusted CO2 curve based on experimental data
float Ro = 10;                            // Initialize Ro to 10 kilo ohms

void setup() {
  Serial.begin(9600);                    // Initialize serial communication
  Serial.print("Calibrating...\n");
  Ro = MQCalibration(MQ_PIN);            // Calibrate the sensor in clean air
  Serial.print("Calibration done...\n");
  Serial.print("Ro = ");
  Serial.print(Ro);
  Serial.print(" kohm\n");
}

void loop() {
  float rs_ro_ratio = MQRead(MQ_PIN) / Ro;
  
  Serial.print("NH3 Concentration: ");
  Serial.print(MQGetGasPercentage(rs_ro_ratio, GAS_AMMONIA));
  Serial.print(" ppm\n");

  Serial.print("CO2 Concentration: ");
  Serial.print(MQGetGasPercentage(rs_ro_ratio, GAS_CO2));
  Serial.print(" ppm\n");
  
  delay(1000); // Delay between readings
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

  for (i = 0; i < CALIBARAION_SAMPLE_TIMES; i++) {   // Take multiple samples
    val += MQResistanceCalculation(analogRead(mq_pin));
    delay(CALIBRATION_SAMPLE_INTERVAL);
  }
  val = val / CALIBARAION_SAMPLE_TIMES;             // Calculate average
  val = val / RO_CLEAN_AIR_FACTOR;                  // Divide by clean air factor
  
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

/*****************************  MQGetGasPercentage **********************************
Input:   rs_ro_ratio - Rs divided by Ro
         gas_id      - target gas type
Output:  ppm of the target gas
************************************************************************************/
int MQGetGasPercentage(float rs_ro_ratio, int gas_id) {
  if (gas_id == GAS_AMMONIA) {
    return MQGetPercentage(rs_ro_ratio, NH3Curve);
  } else if (gas_id == GAS_CO2) {
    return MQGetPercentage(rs_ro_ratio, CO2Curve);
  }
  return 0;
}

/*****************************  MQGetPercentage *************************************
Input:   rs_ro_ratio - Rs divided by Ro
         pcurve      - pointer to the curve of the target gas
Output:  ppm of the target gas
************************************************************************************/
int MQGetPercentage(float rs_ro_ratio, float *pcurve) {
  return (pow(10, ((log10(rs_ro_ratio) - pcurve[1]) / pcurve[2]) + pcurve[0]));
}
