

// ====================================================================================================//////



// ==== BLYNK CONFIGURATION ====

// #define BLYNK_TEMPLATE_ID "TMPL6x00K0B1y"
// #define BLYNK_TEMPLATE_NAME "BTLNhom5"
// #define BLYNK_AUTH_TOKEN "1qPwzgWT-aZBiFoVg6u722O5YT7Mci6L"

// ========== account Tranh ======
// #define BLYNK_TEMPLATE_ID "TMPL6p4gZ8_E_"
// #define BLYNK_TEMPLATE_NAME "Nhom5"
// #define BLYNK_AUTH_TOKEN "lUIIfWAI62Ve9mgjHY_tSj-Hd5wJIs-2"

// ========== account Duy ======
// #define BLYNK_TEMPLATE_ID "TMPL6XAgialRw"
// #define BLYNK_TEMPLATE_NAME "Nhom5"
// #define BLYNK_AUTH_TOKEN "UjDDqlcPAD9IwmlRmzx5UhOTYow5obOV"

#define BLYNK_TEMPLATE_ID "TMPL6XAgialRw"
#define BLYNK_TEMPLATE_NAME "Nhom5"
#define BLYNK_AUTH_TOKEN "UjDDqlcPAD9IwmlRmzx5UhOTYow5obOV"

#include <WiFi.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <BlynkSimpleEsp32.h>

// ==== PIN DEFINITIONS ====
#define DHTPIN 16
#define DHTTYPE DHT11
#define MQ2PIN 34
#define BUZZER 15
#define RELAY1 2
#define RELAY2 4
#define LED1 32
#define LED2 33
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// ==== SENSOR & DISPLAY OBJECTS ====
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ==== WIFI CONFIG ====
const char* ssid = "VitaminTL";
const char* password = "VitaminTL";

// ==== THRESHOLDS ====
const int GAS_THRESHOLD_LOW = 800;
const int GAS_THRESHOLD_HIGH = 1000;
const float TEMP_THRESHOLD = 35.0;

// ==== STATES ====
int relay1State = 0;
int relay2State = 0;
int buzzerState = 0;


// ==== TIMING ====
unsigned long lastSensorRead = 0;
unsigned long lastDisplayUpdate = 0;
const unsigned long sensorInterval = 2000;
const unsigned long displayInterval = 100;

// ==== FUNCTION DECLARATIONS ====
void updateDisplay(float temp, float humid, int gasValue);
void controlDevices(float temp, int gasValue);
void syncBlynk();

// ==== SETUP ====
void setup() {
  Serial.begin(115200);
  dht.begin();

  Wire.begin(21, 22);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED init failed");
    while (1);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  pinMode(BUZZER, OUTPUT);
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  digitalWrite(BUZZER, LOW);
  digitalWrite(RELAY1, LOW);
  digitalWrite(RELAY2, LOW);
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println(WiFi.localIP());

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
}

// ==== BLYNK CALLBACKS ====
BLYNK_CONNECTED() {
  Blynk.syncVirtual(V3, V4, V5);
}

BLYNK_WRITE(V3) { relay1State = param.asInt(); digitalWrite(RELAY1, relay1State); }
BLYNK_WRITE(V4) { relay2State = param.asInt(); digitalWrite(RELAY2, relay2State); }
BLYNK_WRITE(V5) { buzzerState = param.asInt(); digitalWrite(BUZZER, buzzerState); }

// ==== LOOP ====
void loop() {
  Blynk.run();
  unsigned long currentMillis = millis();

  // Read sensors periodically
  static float temp = 0.0, humid = 0.0;
  static int gasValue = 0;

  if (currentMillis - lastSensorRead >= sensorInterval) {
    lastSensorRead = currentMillis;
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    int g = analogRead(MQ2PIN);

    if (!isnan(t) && !isnan(h)) {
      temp = t;
      humid = h;
      gasValue = g;

      controlDevices(temp, gasValue);
      syncBlynk();
      Blynk.virtualWrite(V0, temp);
      Blynk.virtualWrite(V1, humid);
      Blynk.virtualWrite(V2, gasValue);

      Serial.printf("Temp: %.1f | Humid: %.1f | Gas: %d\n", temp, humid, gasValue);
    }
  }

  // Update display more frequently for smooth scroll
  if (currentMillis - lastDisplayUpdate >= displayInterval) {
    lastDisplayUpdate = currentMillis;
    updateDisplay(temp, humid, gasValue);
  }
}

// ==== DISPLAY FUNCTION ====
void updateDisplay(float temp, float humid, int gasValue) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("       Nhom 5");
  display.setCursor(0, 16); display.printf("Temp: %.1f C\n", temp);
  display.setCursor(0, 28); display.printf("Humid: %.1f %%\n", humid);
  display.setCursor(0, 40); display.printf("Gas: %d ppm\n", gasValue);
  if (gasValue > GAS_THRESHOLD_HIGH || temp > TEMP_THRESHOLD) {
    display.setCursor(0, 52); display.println("!! ALERT: DANGER !!");
  }
  display.display();
}

// ==== DEVICE CONTROL FUNCTION ====
void controlDevices(float temp, int gasValue) {
  if (gasValue > GAS_THRESHOLD_HIGH && temp > TEMP_THRESHOLD) {
    buzzerState = relay1State = relay2State = 1;
    Serial.println("Cảnh báo nhiệt + khí gas cao!");
  } else if (gasValue > GAS_THRESHOLD_HIGH) {
    buzzerState = relay1State = relay2State = 1;
    Serial.println("Cảnh báo khí gas cao!");
  } else if (gasValue > GAS_THRESHOLD_LOW) {
    buzzerState = 0; relay1State = 1; relay2State = 0;
    Serial.println("Khí gas trung bình.");
  } else if(temp > TEMP_THRESHOLD){
    buzzerState = 1; relay1State = 0; relay2State = 1;
    Serial.println("Cảnh báo nhiệt độ cao.");
  }else {
    buzzerState = relay1State = relay2State = 0;
  }

  digitalWrite(BUZZER, buzzerState);
  digitalWrite(RELAY1, relay1State);
  digitalWrite(RELAY2, relay2State);
  digitalWrite(LED1, relay1State);
  digitalWrite(LED2, relay2State);
}

// ==== SYNC STATE TO BLYNK ====
void syncBlynk() {
  static int lastRelay1 = -1, lastRelay2 = -1, lastBuzz = -1;
  if (relay1State != lastRelay1)
  { 
    Blynk.virtualWrite(V3, relay1State); 
    lastRelay1 = relay1State; 
  }
  if (relay2State != lastRelay2)
  { 
    Blynk.virtualWrite(V4, relay2State);
    lastRelay2 = relay2State; 
    }
  if (buzzerState != lastBuzz)
  { 
    Blynk.virtualWrite(V5, buzzerState);
    lastBuzz = buzzerState; 
  }
}


