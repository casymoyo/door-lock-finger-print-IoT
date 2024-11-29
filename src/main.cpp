#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <UrlEncode.h>

// I2C LCD
LiquidCrystal_I2C lcd(0x27, 16, 2); 

// Relay Pin
#define RELAY_PIN D8
#define BUZZZER_PIN D4

// twilio
const char* FROM_WHATSAPP = "whatsapp:+15865018478";  
const char* TO_WHATSAPP = "whatsapp:+263789953575"; 

String phoneNumber = "+263789953575";
String apiKey = "1636956";

// SoftwareSerial for Fingerprint
SoftwareSerial mySerial(D3, D4); 
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// WiFi Credentials
const char* ssid = "Casy";
const char* password = "";

void setup() {
  Serial.begin(115200);
  mySerial.begin(57600);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);

  lcd.init();
  lcd.clear();  
  lcd.backlight();
  
  finger.begin(57600);
  connectToWiFi();
  
  if (finger.verifyPassword()) {
    lcd.clear();
    lcd.print("Sensor Ready!");
  } else {
    lcd.clear();
    lcd.print("No Sensor Found!");
    while (1) { delay(1); }
  }

  displayMenu();
}

void loop() {
  if (Serial.available()) {
    char command = Serial.read();
    if (command == '1') {
      enrollFinger();
      displayMenu(); 
    } else if (command == '2') {
      int id = getFingerprintID();
      if (id >= 0) {
        digitalWrite(RELAY_PIN, LOW); 
        delay(8000); 
        lcd.clear();
        lcd.print("Access Granted!!");
        Serial.println("Access Granted!! ID: " + String(id));

        // Send WhatsApp notification
        sendMessage("Access granted. Fingerprint ID: " + String(id));

        delay(8000);
      } else {
        lcd.clear();
        lcd.print("No Match");
        Serial.println("No Match");
        digitalWrite(RELAY_PIN, LOW);
      }
      digitalWrite(RELAY_PIN, HIGH); 
      displayMenu();
    }
  }
}

void displayMenu() {
  lcd.clear();
  lcd.print("1: Enroll Finger");
  lcd.setCursor(0, 1); 
  lcd.print("2: Check Access");
}

void enrollFinger() {
  lcd.clear();
  lcd.print("Place Finger...");
  delay(2000); 

  int p = finger.getImage();
  if (p != FINGERPRINT_OK) {
    lcd.clear();
    lcd.print("Failed to get img");
    return;
  }

  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) {
    lcd.clear();
    lcd.print("Failed to convert");
    return;
  }

  lcd.clear();
  lcd.print("Remove Finger...");
  delay(2000);

  p = finger.getImage();
  if (p != FINGERPRINT_OK) {
    lcd.clear();
    lcd.print("Failed to get img");
    return;
  }

  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) {
    lcd.clear();
    lcd.print("Failed to convert");
    return;
  }

  p = finger.createModel();
  if (p != FINGERPRINT_OK) {
    lcd.clear();
    lcd.print("Failed to create");
    return;
  }

  p = finger.storeModel(1); 
  if (p != FINGERPRINT_OK) {
    lcd.clear();
    lcd.print("Failed to store");
    return;
  }

  lcd.clear();
  lcd.print("Finger enrolled!");
  Serial.println("Finger enrolled!");
  delay(2000);
}

int getFingerprintID() {
  int p = finger.getImage();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.fingerSearch();
  if (p != FINGERPRINT_OK) return -1;

  return finger.fingerID; 
}

void connectToWiFi() {
  lcd.clear();
  lcd.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    lcd.print(".");
  }
  lcd.clear();
  lcd.print("Connected!");
  delay(2000);
}

void sendMessage(String message){

  // Data to send with HTTP POST
  String url = "http://api.callmebot.com/whatsapp.php?phone=263789953575&text=Access+Granted+!&apikey=8893619";
  WiFiClient client;    
  HTTPClient http;
  http.begin(client, url);

  // Specify content-type header
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
  // Send HTTP POST request
  int httpResponseCode = http.POST(url);
  if (httpResponseCode == 200){
    Serial.print("Message sent successfully");
  }
  else{
    Serial.println("Error sending the message");
    Serial.print("HTTP response code: ");
    Serial.println(httpResponseCode);
  }

  // Free resources
  http.end();
}

