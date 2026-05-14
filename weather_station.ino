#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <LiquidCrystal.h>
#include <WiFi.h>
#include <ThingSpeak.h>
#include <Wire.h>


// --- WiFi Credentials ---
const char* ssid = "OCDSB-1BY3WB4 3654";
const char* password = ">9O2596f";


// --- ThingSpeak Setup ---
unsigned long myChannelNumber = 3282707;
const char* myWriteAPIKey = "WUKBEZAUHFKE07XA";
WiFiClient client;


// --- Hardware Configuration ---
int tempAlarmPin = 27;
int humAlarmPin = 26;
int buzzerAlarmPin = 35;


// LCD Pins: RS, EN, D4, D5, D6, D7
LiquidCrystal lcd(19, 23, 18, 5, 4, 17);


// --- Timers ---
unsigned long lastTime = 0;
unsigned long offsetTime = 0;


// I2C
Adafruit_BME680 bme;


void setup() {
  Serial.begin(115200);
  Wire.begin();


  pinMode(tempAlarmPin, OUTPUT);
  pinMode(humAlarmPin, OUTPUT);
  pinMode(buzzerAlarmPin, OUTPUT);


  lcd.begin(16, 2);


  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor. Check wiring.");
    while (1)
      ;  // infinite loop
  }


  // oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);


  // connect to WiFi
  Serial.println("\nConnecting to WiFi...");
  WiFi.begin(ssid, password);


  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < 20) {
    delay(1000);
    Serial.printf("Status: %d | Attempt: %d\n", WiFi.status(), attempt);
    lcd.setCursor(0, 1);
    lcd.print("WiFi Try: ");
    lcd.print(attempt);
    attempt++;
  }


  if (WiFi.status() == WL_CONNECTED) {
    lcd.clear();
    Serial.println("WiFi Connected!");
    lcd.print("WiFi Connected!");
  } else {
    lcd.clear();
    lcd.print("WiFi Failed!");
  }


  ThingSpeak.begin(client);


  // startup Message
  lcd.clear();
  lcd.print("Sensor Loading..");
  delay(2000);


  // 3. Sensor Warm-up
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sensor Warm-up:");
  for (int i = 3; i > 0; i--) {
    lcd.setCursor(0, 1);
    lcd.print("Ready in: ");
    lcd.print(i);
    lcd.print("s...");
    delay(1000);
  }


  offsetTime = millis();
  lastTime = millis();
  lcd.clear();
}


void loop() {
  if (!bme.performReading()) {
    Serial.println("Failed to perform reading.");
    return;
  }


  // get values
  float tempC = bme.temperature;
  float tempF = (tempC * 9.0 / 5.0) + 32.0;
  float hum = bme.humidity;


  unsigned long activeTime = millis() - offsetTime;


  // clear the status area (top right) every loop
  lcd.setCursor(14, 0);
  lcd.print("  ");


  // top row: temperature
  lcd.setCursor(0, 0);
  lcd.print(tempC, 2);
  lcd.print("C ");
  lcd.print(tempF, 2);
  lcd.print("F ");


  // bottom row: clock and humidity
  int h = activeTime / 3600000;
  int m = (activeTime / 60000) % 60;
  int s = (activeTime % 60000) / 1000;


  lcd.setCursor(0, 1);
  if (h < 10) lcd.print('0');
  lcd.print(h);
  lcd.print(':');
  if (m < 10) lcd.print('0');
  lcd.print(m);
  lcd.print(':');
  if (s < 10) lcd.print('0');
  lcd.print(s);


  lcd.setCursor(9, 1);
  lcd.print("H: ");
  lcd.print(int(hum));
  lcd.print("% ");


  // emergency alarm system
  int humEmergencyValue = 40;
  float tempEmergencyHighValue = 30.0;
  float tempEmergencyLowValue = 15.0;


  bool humEmergency = (hum >= humEmergencyValue);
  bool tempEmergencyHigh = (tempC > tempEmergencyHighValue);
  bool tempEmergencyLow = (tempC < tempEmergencyLowValue);


  // case A: both temperature and humidity are emergencies
  if ((tempEmergencyHigh || tempEmergencyLow) && humEmergency) {
    for (int i = 0; i < 3; i++) {
      digitalWrite(tempAlarmPin, HIGH);
      digitalWrite(humAlarmPin, LOW);
      tone(buzzerAlarmPin, 4500, 100);
      delay(100);
      digitalWrite(tempAlarmPin, LOW);
      digitalWrite(humAlarmPin, HIGH);
      tone(buzzerAlarmPin, 3000, 100);
      delay(100);
    }
    digitalWrite(humAlarmPin, LOW);
  }
  // case B: just high temperature
  else if (tempEmergencyHigh) {
    digitalWrite(tempAlarmPin, HIGH);
    tone(buzzerAlarmPin, 3500, 200);
    delay(300);
    digitalWrite(tempAlarmPin, LOW);
    tone(buzzerAlarmPin, 3500, 200);
  }
  // case C: just low temperature
  else if (tempEmergencyLow) {
    digitalWrite(tempAlarmPin, HIGH);
    tone(buzzerAlarmPin, 1500, 100);
    delay(200);
    digitalWrite(tempAlarmPin, LOW);
    tone(buzzerAlarmPin, 1500, 100);
    delay(200);
    digitalWrite(tempAlarmPin, HIGH);
    tone(buzzerAlarmPin, 1500, 400);
    digitalWrite(tempAlarmPin, LOW);
  }
  // case D: just humidity
  else if (humEmergency) {
    digitalWrite(humAlarmPin, HIGH);
    tone(buzzerAlarmPin, 4000, 500);
    delay(500);
    digitalWrite(humAlarmPin, LOW);
  }
  // case E: no emergency
  else {
    noTone(buzzerAlarmPin);
    digitalWrite(tempAlarmPin, LOW);
    digitalWrite(humAlarmPin, LOW);
  }


  // upload to ThingSpeak
  if (millis() - lastTime >= 14000) {
    ThingSpeak.setField(1, tempC);
    ThingSpeak.setField(2, (float)hum);


    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);


    lcd.setCursor(14, 0);
    if (x == 200) {
      Serial.println("Channel update successful")
    } else {
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }
    lastTime = millis();
  }


  delay(2000);
}
