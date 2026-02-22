#include <Preferences.h>
Preferences preferences;
#include <Arduino.h>
#include <WiFi.h>
#include <ESP_Mail_Client.h>
#include "esp_camera.h"
#include "time.h"

String ssid;
String password;
String smtp_host;
int smtp_port;
String email_sender;
String email_password;
String email_recipient;
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM       5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
#ifndef LED_BUILTIN
#define LED_BUILTIN 33
#endif
ESP_Mail_Session session;
SMTPSession smtp;
static uint32_t lastChecksum = 0;
static int motionCounter = 0;
static unsigned long lastMotionEmail = 0;
const unsigned long MOTION_COOLDOWN = 30000; 
const uint32_t MOTION_THRESHOLD = 300000;    


void resyncTime() {
  configTzTime("TAHT-10", "pool.ntp.org", "time.nist.gov");
  struct tm tm_now;
  if (getLocalTime(&tm_now)) {
    Serial.printf("Tahiti Time: %04d-%02d-%02d %02d:%02d:%02d\n",
                  tm_now.tm_year + 1900, tm_now.tm_mon + 1, tm_now.tm_mday,
                  tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec);
  }
}

String makeMovementSubject() {
  time_t now = time(nullptr);
  struct tm tm_now;
  localtime_r(&now, &tm_now);

  char subjBuf[80];
  snprintf(subjBuf, sizeof(subjBuf),
           "Movement detected at %02d-%02d-%04d %02d:%02d:%02d",
           tm_now.tm_mday, tm_now.tm_mon + 1, tm_now.tm_year + 1900,
           tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec);

  return String(subjBuf);
}

String makeFilename(char *outFilename, size_t fnameLen) {
  time_t now = time(nullptr);
  struct tm tm_now;
  localtime_r(&now, &tm_now);

  char fnBuf[80];
  strftime(fnBuf, sizeof(fnBuf),
           "movement_%H_%M_%S_%Y%m%d.jpg", &tm_now);

  strncpy(outFilename, fnBuf, fnameLen - 1);
  outFilename[fnameLen - 1] = '\0';

  return String(fnBuf);
}

void captureAndSendPhoto() {
//to do
}
void initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode    = CAMERA_GRAB_WHEN_EMPTY;
  config.frame_size = FRAMESIZE_HD;
  config.fb_location = psramFound() ? CAMERA_FB_IN_PSRAM : CAMERA_FB_IN_DRAM;
  config.fb_count = 1;
  config.jpeg_quality = psramFound() ? 10 : 12;
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x\n", err);
    return;
  }
  Serial.println("Camera initialized");
}
void setup() {
  Serial.begin(115200);
  delay(10);

  preferences.begin("esp32cam", true);

  ssid            = preferences.getString("ssid", "");
  password        = preferences.getString("password", "");
  smtp_host       = preferences.getString("smtp_server", "");
  smtp_port       = preferences.getInt("smtp_port", 0);
  email_sender    = preferences.getString("sender_email", "");
  email_password  = preferences.getString("sender_password", "");
  email_recipient = preferences.getString("recipient_email", "");
  preferences.end();
  if (ssid == "" || password == "" || smtp_host == "" || smtp_port == 0 ||
      email_sender == "" || email_password == "" || email_recipient == "") {
    Serial.println("ERROR: Missing credentials in NVS!");
    while (true) delay(1000);
  }
  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.print("Connecting to WiFi");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED)
    Serial.println("WiFi connected: " + String(WiFi.localIP()));
  else
    Serial.println("WiFi connect failed");
  initCamera();
  resyncTime();
  Serial.println("System ready — monitoring movement...");
}
void loop() {
  if (WiFi.status() != WL_CONNECTED) WiFi.reconnect();
  captureAndSendPhoto();  
  delay(2000);
}
