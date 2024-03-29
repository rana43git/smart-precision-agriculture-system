//TODO
//Remove this line and uncommnet avobe 4 line
//find this and fix it
//----------------------------------------Include the NodeMCU ESP8266 Library
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Wire.h> //For I2C Display Communication
#include <LiquidCrystal_I2C.h>  //For Display
#include <NewPing.h>  //For AJ-SR04M Ultrasonic Sensor
#include <MedianFilter.h> //For AJ-SR04M Ultrasonic Sensor
//----------------------------------------
//Display Related Info
//LCD Address: 0x3F
//ESP8266: SDA D2(GPIO 4)
//ESP8266: SCL D1(GPIO 5)
//Arduino: SDA A4
//Arduino: SCL A5
//----------------------------------------
//AJ-SR04M Ultrasonic Sensor
#define TRIGGER_PIN  12  // D6(ESP8266) Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     13  // D7(ESP8266) Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 450 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.
MedianFilter filter(31, 0);

float TD = 43; //Total Distace(TD), sensor height from soil level
float BS = 18; //Blind Spot, for AJ-SR04M Ultrasonic Sensor, if object is between 0cm to 18cm, it fails to identify
float MD = (TD - BS); //Measureable Distance
float PH = 0; //Plant Growth
//----------------------------------------
//Soil Moitsure Sensor
int SoilMoitsureValue = 0;  //ESP8266 A0,Define a variable to store the analog sensor reading
#define PumpStatusLED 14  //ESP8266 D5, Define the pin for the pump status LED
//----------------------------------------
#include "DHT.h"  //--> Include the DHT Sensor Library 
#define DHTTYPE DHT11 //--> Defines the type of DHT sensor used (DHT11, DHT21, and DHT22), in this project the sensor used is DHT11.

#include "PageIndex.h" //--> Include the contents of the User Interface Web page, stored in the same folder as the .ino file

#define LEDonBoard 2  //--> Defining an On Board LED, used for indicators when the process of connecting to a wifi router

//----------------------------------------SSID and Password of your WiFi router
const char* ssid = "SPAS";
const char* password = "SPAS2023";
//----------------------------------------

ESP8266WebServer server(80);  //--> Server on port 80

const int DHTPin = 0; //--> The pin used for the DHT11 sensor is Pin D3=Pin 0###
DHT dht(DHTPin, DHTTYPE); //--> Initialize DHT sensor, DHT dht(Pin_used, Type_of_DHT_Sensor);

//----------------------------------------This routine is executed when you open NodeMCU ESP8266 IP Address in browser
void handleRoot() {
  String s = MAIN_page; //--> Read HTML contents
  server.send(200, "text/html", s); //--> Send web page
}
//----------------------------------------
//----------------------------------------Procedure for reading the temperature value of a DHT11 sensor
void handleDHT11Temperature() {
  float t = dht.readTemperature();  //--> Read temperature as Celsius (the default). If you want to read the temperature sensor in Fahrenheit, use this -> float t = dht.readTemperature (true);
  String Temperature_Value = String(t);

  server.send(200, "text/plane", Temperature_Value); //--> Send Temperature value only to client ajax request

  if (isnan(t)) { //--> Check if any reads failed and exit early (to try again).
    Serial.println("Failed to read from DHT sensor!");
  }
  else {
    Serial.print("DHT11 || Temperature : ");
    Serial.print(t);
    Serial.print(" || ");
  }
}


void hanldePlantGrowth() {
  delay(50);  // Wait 50ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
  //unsigned int o, uS = sonar.ping(); // Send ping, get ping time in microseconds (uS).
  //filter.in(uS);
  //o = filter.out();
  //int SV = o / US_ROUNDTRIP_CM; //Sensor Value, Convert ping time to distance in cm and print result (0 = outside set distance range)
  int SV = sonar.ping_cm(); //Remove this line and uncommnet avobe 4 line
  Serial.print("Distance: ");
  Serial.print(SV); Serial.println("cm");
  Serial.print("Distance in percentage: ");
  PH = ((TD - SV) / MD) * 100; //Distance in percentage Calculation
  Serial.print((int)PH);
  Serial.println("%");
  String PlantGrowth_Value = String((int)PH);
  server.send(200, "text/plane", PlantGrowth_Value); //--> Send Humidity value only to client ajax request
}


void handleSoilMoitsure() {
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
  String SoilMoitsure_Value = String(moistureInPercentage);
  server.send(200, "text/plane", SoilMoitsure_Value); //--> Send Humidity value only to client ajax request
}


//----------------------------------------
//----------------------------------------Procedure for reading humidity values from DHT11 sensors
void handleDHT11Humidity() {
  float h = dht.readHumidity();
  String Humidity_Value = String(h);

  server.send(200, "text/plane", Humidity_Value); //--> Send Humidity value only to client ajax request
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor !");
  }
  else {
    Serial.print("Humidity : ");
    Serial.println(h);
  }
}
//----------------------------------------
//----------------------------------------Setup
LiquidCrystal_I2C lcd(0x3F, 20, 4); // CHANGE THE 0x3F ADDRESS TO YOUR SCREEN ADDRESS IF NEEDED
//Gloabal Variable for animated [.]
int animt = 0;
//
void setup(void) {
  Serial.begin(115200);
  //////////////////////////
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("WiFi: ");
  lcd.print(ssid);
  lcd.setCursor(0, 1);
  lcd.print("Password: ");
  lcd.print(password);
  //////////////////////////
  delay(500);
  dht.begin();  //--> Start reading DHT11 sensors
  delay(500);

  WiFi.begin(ssid, password); //--> Connect to your WiFi router
  Serial.println("");

  pinMode(PumpStatusLED, OUTPUT);  //Initialize the PumpStatusLED pin as an output
  pinMode(LEDonBoard, OUTPUT); //--> On Board LED port Direction output
  digitalWrite(LEDonBoard, HIGH); //--> Turn off Led On Board

  //----------------------------------------Wait for connection
  Serial.print("Connecting");
  lcd.setCursor(0, 2);
  lcd.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    //creating animation of [.]
    if (animt % 2 == 0) {
      lcd.setCursor(11, 2);
      lcd.print(".");
      lcd.setCursor(12, 2);
      lcd.print(" ");
      lcd.setCursor(13, 2);
      lcd.print(".");
      lcd.setCursor(14, 2);
      lcd.print(" ");
      lcd.setCursor(15, 2);
      lcd.print(".");
      lcd.setCursor(16, 2);
      lcd.print(" ");
    }
    if (animt % 2 != 0) {
      lcd.setCursor(11, 2);
      lcd.print(" ");
      lcd.setCursor(12, 2);
      lcd.print(".");
      lcd.setCursor(13, 2);
      lcd.print(" ");
      lcd.setCursor(14, 2);
      lcd.print(".");
      lcd.setCursor(15, 2);
      lcd.print(" ");
      lcd.setCursor(16, 2);
      lcd.print(".");
    }
    animt++;
    //end
    //----------------------------------------Make the On Board Flashing LED on the process of connecting to the wifi router.
    digitalWrite(LEDonBoard, LOW);
    delay(250);
    digitalWrite(LEDonBoard, HIGH);
    delay(250);
    //----------------------------------------
  }
  //----------------------------------------
  digitalWrite(LEDonBoard, HIGH); //--> Turn off the On Board LED when it is connected to the wifi router.
  //----------------------------------------If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  lcd.setCursor(0, 2);
  lcd.print("     Connected!     ");
  delay(1000);
  lcd.setCursor(0, 2);
  lcd.print("IP: ");
  lcd.print(WiFi.localIP());
  //----------------------------------------
  server.on("/", handleRoot); //--> Routine to handle at root location. This is to display web page.
  server.on("/readTemperature", handleDHT11Temperature);  //--> Routine to handle the call procedure handleDHT11Temperature
  server.on("/readHumidity", handleDHT11Humidity);  //--> Routine to handle the call procedure handleDHT11Humidity
  server.on("/readPlantGrowth", hanldePlantGrowth);  //--> Routine to handle the call procedure hanldePlantGrowth
  server.on("/readSoilMoitsure", handleSoilMoitsure);  //--> Routine to handle the call procedure handleSoilMoitsure

  server.begin(); //--> Start server
  Serial.println("HTTP Server Started!");
  lcd.setCursor(0, 3);
  lcd.print("HTTP Server Started!");
}
//----------------------------------------
//----------------------------------------Loop
void loop() {
  handleSoilMoitsure();
  server.handleClient();  //Handle client requests
}
//----------------------------------------
