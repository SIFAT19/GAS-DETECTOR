#define RAIN_SENSOR_PIN A0 // Define the sensor pin  

void setup() {  
  Serial.begin(9600); // Start serial communication  
}  

void loop() {  
  int sensorValue = analogRead(RAIN_SENSOR_PIN); // Read the sensor value  
  if (sensorValue < 600) { // Adjust threshold as necessary  
    Serial.println("Rain detected");  
  } else {  
    Serial.println("No rain");  
  }  
  delay(1000); // Wait for a second before checking again  
}
