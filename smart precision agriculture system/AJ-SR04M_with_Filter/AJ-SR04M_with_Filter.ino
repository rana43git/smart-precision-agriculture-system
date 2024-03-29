// ---------------------------------------------------------------------------
// Example NewPing library sketch that does a ping about 20 times per second.
// ---------------------------------------------------------------------------


#include <NewPing.h>
#include <MedianFilter.h>
#include <Wire.h>
//----------------------------------------
//AJ-SR04M Ultrasonic Sensor
#define TRIGGER_PIN  12  // D6(ESP8266) Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     13  // D7(ESP8266) Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 450 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.
MedianFilter filter(31, 0);

float TD = 63; //Total Distace(TD), sensor height from soil level
float BS = 18; //Blind Spot, for AJ-SR04M Ultrasonic Sensor, if object is between 0cm to 18cm, it fails to identify
float MD = (TD - BS); //Measureable Distance
float PH = 0; //Plant Growth
//----------------------------------------
void setup() {
  Serial.begin(115200); // Open serial monitor at 115200 baud to see ping results.
}

void loop() {
  delay(50);  // Wait 50ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
  unsigned int o, uS = sonar.ping(); // Send ping, get ping time in microseconds (uS).
  filter.in(uS);
  o = filter.out();
  int SV = o / US_ROUNDTRIP_CM; //Sensor Value, Convert ping time to distance in cm and print result (0 = outside set distance range)
  Serial.print("Distance: ");
  Serial.print(SV); Serial.println("cm");
  Serial.print("Distance in percentage: ");
  PH = ((TD - SV) / MD) * 100; //Distance in percentage Calculation
  Serial.print((int)PH);
  Serial.println("%");
}
