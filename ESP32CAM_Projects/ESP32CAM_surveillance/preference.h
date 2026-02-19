#include <Preferences.h>
Preferences preferences;

void setup() {
  Serial.begin(115200);
  delay(500); // Tu laisses un peu de temps au port série de s'initialiser
  
  Serial.println(); Serial.println("=== ESP32-CAM Credential Writer ===");

  preferences.begin("esp32cam", false);
  Serial.println("Writing credentials to NVS...");

  preferences.putString("ssid", "your wifi name");
  Serial.println("✔ SSID stored");

  preferences.putString("password", "your wifi pwd");
  Serial.println("✔ WiFi password stored");

  preferences.putString("smtp_server", "smtp.gmail.com"); //your preference
  Serial.println("✔ SMTP server stored");

  preferences.putInt("smtp_port", 587);
  Serial.println("✔ SMTP port stored");

  preferences.putString("sender_email", "sender@gmail.com");
  Serial.println("✔ Sender email stored");

  preferences.putString("sender_password", "pwd");
  Serial.println("✔ Sender password stored");

  preferences.putString("recipient_email", "youremail@gmail.com");
  Serial.println("✔ Recipient email stored");

  preferences.end();
  Serial.println("All credentials stored securely in NVS!"); 
  
}

void loop() {}