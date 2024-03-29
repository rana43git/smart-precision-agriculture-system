int SoilMoitsureValue = 0;  //Define a variable to store the analog sensor reading
#define PumpStatusLED 14  //ESP8266 D5, Define the pin for the pump status LED

void setup() {
  // Initialize the serial communication for debugging
  Serial.begin(115200);
  pinMode(PumpStatusLED, OUTPUT);  //Initialize the PumpStatusLED pin as an output
}

void loop() {
  SoilMoitsureValue = analogRead(A0); // Read the analog sensor value

  int moistureInPercentage = ((1024 - SoilMoitsureValue) / 1024.0) * 100; // Calculate the moisture percentage as a float

  Serial.print("Soil Moisture: "); // Print the sensor value to the serial monitor
  Serial.print(moistureInPercentage);
  Serial.println("%");

  if (moistureInPercentage < 40) {
    digitalWrite(PumpStatusLED, HIGH); //Turn LED on
    Serial.println("Water Pump ON.");
  }
  else if (moistureInPercentage > 60) {
    digitalWrite(PumpStatusLED, LOW); //Turn LED off
    Serial.println("Water Pump OFF.");
  }
  else {
    digitalWrite(PumpStatusLED, LOW);  //Turn LED off
    Serial.println("Water Pump OFF.");
  }

  delay(500);  //Add a delay to control the reading frequency
}
