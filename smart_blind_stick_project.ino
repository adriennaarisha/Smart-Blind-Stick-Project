#include <ESP32Servo.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// ========== Pin Definitions ==========
#define TRIG_PIN     12
#define ECHO_PIN     13
#define BUZZER_PIN   26
#define SERVO_PIN    32
#define BUTTON_PIN   22   // SOS button connected to GPIO22 → GND

// ========== Objects ==========
Servo myServo;

// ========== WiFi Setup ==========
const char* ssid = "wifi_name";       
const char* password = "wifi_password"; 

// ========== Telegram Setup ==========
#define BOTtoken "8270811995:AAEOH7xiE6DBlqoJgUc7qEWstx6LJ-eWmQ8"  

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// ===== Multiuser Chat IDs =====
String chatIDs[] = {
  "ID_1",   
  "ID_2"   
};
int totalUsers = sizeof(chatIDs) / sizeof(chatIDs[0]);

// ===== Helper Function to Send Message to All =====
void sendToAll(String message) {
  for (int i = 0; i < totalUsers; i++) {
    bot.sendMessage(chatIDs[i], message, "");
  }
}

bool lastButtonState = HIGH;

void setup() {
  Serial.begin(115200);

  // Pin setup
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Button uses internal pull-up

  // Servo setup
  myServo.attach(SERVO_PIN);
  myServo.write(0);  // Initial servo position

  // WiFi setup
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");

  client.setInsecure(); // skip SSL certificate check

  // Send Telegram boot-up message
  sendToAll("✅ Smart Blind Stick is Online!");
}

void loop() {
  // ====== SOS Button (Telegram) ======
  bool buttonState = digitalRead(BUTTON_PIN);

  if (buttonState == LOW && lastButtonState == HIGH) {
    Serial.println("🚨 SOS Button Pressed!");
    sendToAll("🚨 SOS Alert! Please help immediately!");
    delay(1000); // debounce
  }
  lastButtonState = buttonState;

  // ====== Ultrasonic Distance Measurement ======
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = duration * 0.034 / 2;

  Serial.print("Distance: ");
  Serial.println(distance);

  if (distance < 100) {
    digitalWrite(BUZZER_PIN, HIGH);   // Activate buzzer
    myServo.write(90);                // Move servo to 90°
  } else {
    digitalWrite(BUZZER_PIN, LOW);    // Deactivate buzzer
    myServo.write(0);                 // Reset servo to 0°
  }

  delay(200); // Small delay for stability
}