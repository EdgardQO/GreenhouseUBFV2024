#define BLYNK_TEMPLATE_ID "TMPL25Jq8ttAf" 
#define BLYNK_TEMPLATE_NAME "Invernadero UNFV" 
#define BLYNK_AUTH_TOKEN "6o-cexd5Ht-JiWmnW2ZxCL33B_wHJ2Wd" 
#define BLYNK_PRINT Serial 
 
#include <WiFi.h> 
#include <WiFiClient.h> 
#include <BlynkSimpleEsp32.h> 
#include <ESP32Servo.h> 
#include <MQ135.h> 
#include <DHT.h> 
 
char red[] = "Redmi Note 13"; 
char pass[] = "quijano2024"; 
 
#define RELAY_PIN 21    //Pin to which relay 1 is connected 
#define RELAY2_PIN 23    //Pin to which relay 2 is connected 
#define RELAY3_PIN 18    //Pin to which relay 3 is connected 
 
#define SERVO_PIN 22    //Physical pin for the first servo motor 
#define SERVO_VPIN V1   //Virtual pin for the first servo motor in Blynk 
#define RELAY_VPIN V0   //Virtual pin for the relay 1 in Blynk 
#define RELAY2_VPIN V64   //Virtual pin for the relay 2 in Blynk 
#define RELAY3_VPIN V65   //Virtual pin for the relay 3 in Blynk 
 
#define DHT_PIN 4      //Physical pin to which the DHT22 sensor is 
connected 
#define DHTTYPE DHT22 
DHT dht(DHT_PIN, DHTTYPE); 
#define BUZZER_PIN 16   //Physical pin for the buzzer 
#define ANALOG_PIN 32   //Physical pin to which the MQ-135 sensor is 
connected  
#define VIRTUAL_PIN V33 //Blynk virtual pin to display CO in ppm 
 
#define SERVO2_PIN 27    //Physical pin for the second servo motor 
#define SERVO2_VPIN V40  //Virtual pin for the second servo motor in 
Blynk 
 
#define SOIL_MOISTURE_PIN 34 //Connect soil moisture sensor to GPIO 34 
(ADC1_CH6) in ESP32 
 
Servo myServo;  //First servo object for the first servo motor 
Servo myServo2; //Second servo object for the second servomotor 
 
MQ135 gasSensor = MQ135(ANALOG_PIN); 
int pinLED = 2; 
 
BlynkTimer timer; 
 
void sendSensor() 
{ 
  int soilmoisture = analogRead(SOIL_MOISTURE_PIN);  //Read from the soil 
moisture sensor 
  int soilmoisturepercentage = map(soilmoisture, 3500, 4095, 100, 0); 
  //You can send any value at any time. 
  //Don't send more than 10 values per second. 
  Blynk.virtualWrite(V17, soilmoisturepercentage); 
  Serial.print("Soil Moisture : "); 
  Serial.print(soilmoisturepercentage); 
} 
void controlSystem() { 
float humidity = dht.readHumidity(); 
float temperature = dht.readTemperature(); 
float ppm = gasSensor.getPPM(); 
 
//Water pump control 
if (humidity < 60) { 
int wateringTime = (60 - humidity) * 1000 / (100 / 60); //Calculate 
watering time in milliseconds (100L/H) 
digitalWrite(RELAY_PIN, LOW); //Activate water pump 
delay(wateringTime); 
digitalWrite(RELAY_PIN, HIGH); //Deactivate water pump 
} 
 
//Fan control 
if (ppm > 1200) { 
digitalWrite(RELAY2_PIN, LOW); //Activate fan 
} else { 
digitalWrite(RELAY2_PIN, HIGH); //Turn off the fan 
} 
 
//Temperature control according to the time (day/night) 
int hour = hour(); //Get the current time 
if ((hour >= 6 && hour < 18) && (temperature < 18 || temperature > 25)) { 
digitalWrite(RELAY3_PIN, LOW); //Activate the daytime temperature control 
system 
} else if ((hour >= 18 || hour < 6) && (temperature < 7 || temperature > 
15)) { 
digitalWrite(RELAY3_PIN, LOW); //Activate the nighttime temperature 
control system 
} else { 
digitalWrite(RELAY3_PIN, HIGH); //Deactivate the temperature control 
system 
} 
} 
void readGasSensor() { 
  float airQuality = gasSensor.getPPM(); 
  int mappedValue = map(airQuality, 100, 900, 0, 5000); 
  Blynk.virtualWrite(VIRTUAL_PIN, mappedValue); 
} 
 
void readDHTSensor() { 
float humidity = dht.readHumidity(); //Read the humidity 
float temperature = dht.readTemperature(); //Read the temperature in 
degrees Celsius 
if (isnan(humidity) || isnan(temperature)) { //Check if the read is 
invalid 
Serial.println("Failed to read from DHT sensor!"); 
return; 
} 
Blynk.virtualWrite(V36, humidity); //Send humidity to Blynk at virtual 
pin 
Blynk.virtualWrite(V37, temperature); //Send temperature to Blynk at 
virtual pin V37 
Serial.print("Temperature : "); 
Serial.print(temperature); 
Serial.print(" Humidity : "); 
Serial.println(humidity); 
} 
 
void setup() { 
  Serial.begin(115200); 
  pinMode(pinLED, OUTPUT); 
  pinMode(BUZZER_PIN, OUTPUT); 
  dht.begin(); 
 
  Blynk.begin(BLYNK_AUTH_TOKEN, red, pass); 
 
  pinMode(RELAY_PIN, OUTPUT); 
  digitalWrite(RELAY_PIN, HIGH); 
  pinMode(RELAY2_PIN, OUTPUT); 
  digitalWrite(RELAY2_PIN, HIGH); 
  pinMode(RELAY3_PIN, OUTPUT); 
  digitalWrite(RELAY3_PIN, HIGH); 
  myServo.attach(SERVO_PIN); //Initialize the first servo motor on pin 22 
  myServo.write(180); //First servo motor initial position at 180 degrees 
  Blynk.virtualWrite(SERVO_VPIN, LOW); //Initial position in the app 
(LOW) 
  myServo.attach(SERVO2_PIN); //Initialize the first servo motor on pin 
22 
  myServo.write(180); //First servo motor initial position at 180 degrees 
  Blynk.virtualWrite(SERVO2_VPIN, LOW); //Initial position in the app 
(LOW) 
 
  for (int i = 0; i < 5000; i++) { 
    digitalWrite(BUZZER_PIN, HIGH); 
    delayMicroseconds(1000); 
    digitalWrite(BUZZER_PIN, LOW); 
    delayMicroseconds(1000); 
  } 
  timer.setInterval(5000L, readGasSensor); 
 
dht.begin(); //Initialize the DHT22 sensor 
timer.setInterval(2000L, readDHTSensor); //Set an interval to read the 
DHT sensor every 2 seconds 
timer.setInterval(3000L, sendSensor); 
} 
 
void loop() { 
  digitalWrite(pinLED, HIGH); 
  Blynk.run(); 
  timer.run(); 
} 
 
BLYNK_WRITE(RELAY_VPIN) { 
  int pinValue = param.asInt(); 
  digitalWrite(RELAY_PIN, pinValue); 
} 
BLYNK_WRITE(RELAY2_VPIN) { 
  int pinValue = param.asInt(); 
  digitalWrite(RELAY2_PIN, pinValue); 
} 
BLYNK_WRITE(RELAY3_VPIN) { 
  int pinValue = param.asInt(); 
  digitalWrite(RELAY3_PIN, pinValue); 
} 
BLYNK_WRITE(SERVO_VPIN) { 
  int servo_state = param.asInt(); 
  if (servo_state == HIGH) { 
    myServo.write(90); 
  } else { 
    myServo.write(180); 
  } 
} 
BLYNK_WRITE(SERVO2_VPIN) { 
  int servo_state = param.asInt(); 
  if (servo_state == HIGH) { 
    myServo.write(90); 
  } else { 
    myServo.write(180); 
  } 
} 
 
 