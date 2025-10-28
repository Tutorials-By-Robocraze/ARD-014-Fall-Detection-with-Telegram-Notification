#include <WiFiS3.h>
#include <Wire.h>
#include <MPU6050.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>

// 🛜 WiFi & Telegram Config (use your values here)
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
String botToken = "YOUR_TELEGRAM_BOT_TOKEN"; // Example: "123456789:ABCDEF..."
String chat_id = "YOUR_TELEGRAM_CHAT_ID";    // Example: "1450410734"

WiFiSSLClient client;
UniversalTelegramBot bot(botToken, client);

// 📟 LCD I2C
LiquidCrystal_I2C lcd(0x27, 16, 2);

// 🎯 MPU6050
MPU6050 mpu;
int16_t ax, ay, az;
float accelTotal;

// 🔔 Buzzer & LEDs
const int buzzer = 6;
const int redLED = 7;
const int blueLED = 8;

// For LCD dot animation
unsigned long previousMillis = 0;
int dotCount = 0;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("System Booting");
  
  pinMode(buzzer, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(blueLED, OUTPUT);
  
  // ✔ Initialize MPU6050
  mpu.initialize();
  if (!mpu.testConnection()) {
    lcd.clear();
    lcd.print("MPU Error!");
    while (1);
  }
  
  // ✔ WiFi connecting
  lcd.clear();
  lcd.print("WiFi Connecting");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    lcd.setCursor(0, 1);
    lcd.print("...");
  }
  lcd.clear();
  lcd.print("WiFi Connected");
  delay(1000);

  // ✔ Telegram Start Notification
  bot.sendMessage(chat_id, "✅ System Started! Fall Detection Active.", "");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("G= ");
  lcd.setCursor(0, 1);
  lcd.print("Monitoring");
}

void loop() {
  // 📊 Read MPU values
  mpu.getAcceleration(&ax, &ay, &az);
  accelTotal = sqrt(ax * ax + ay * ay + az * az) / 16384.0;

  // Display G-force
  lcd.setCursor(0, 0);
  lcd.print("G= ");
  lcd.print(accelTotal, 2);
  lcd.print("      ");

  // LCD "Monitoring..." Animation
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 500) {
    previousMillis = currentMillis;
    dotCount = (dotCount + 1) % 6;

    lcd.setCursor(0, 1);
    lcd.print("Monitoring");
    for (int i = 0; i < dotCount; i++) {
      lcd.print(".");
    }
    lcd.print("      ");
  }

  // 🚨 Fall detection
  if (accelTotal > 2.5 || accelTotal < 0.5) {
    alertFall();
    delay(5000);
  }
}

void alertFall() {
  Serial.println("⚠️ FALL DETECTED!");
  lcd.clear();
  lcd.print("!! FALL ALERT !!");
  lcd.setCursor(0, 1);
  lcd.print("Sending Msg...");

  for (int i = 0; i < 5; i++) {
    digitalWrite(redLED, HIGH);
    digitalWrite(blueLED, LOW);
    tone(buzzer, 1000);
    delay(200);
    digitalWrite(redLED, LOW);
    digitalWrite(blueLED, HIGH);
    noTone(buzzer);
    delay(200);
  }
  
  // Send Telegram Alert
  bot.sendMessage(chat_id, "🚨 FALL DETECTED! Please check immediately.", "");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("G= ");
  lcd.setCursor(0, 1);
  lcd.print("Monitoring");
}