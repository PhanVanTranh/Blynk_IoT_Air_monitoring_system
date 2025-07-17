#include <ArduinoIoTCloud.h>
#include <WiFi.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Định nghĩa chân
#define DHTPIN 23       // DHT11 (SENSOR)
#define DHTTYPE DHT11
#define MQ2PIN 36       // MQ-2 (SENSOR, ADC1_0)
#define BUZZER 16       // Buzzer
#define RELAY1 19       // Relay 1 (quạt thông gió)
#define RELAY2 18       // Relay 2 (máy lọc không khí)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Khai báo đối tượng
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Biến cloud (được tạo tự động bởi Arduino Cloud)
float temperature;
float humidity;
int gas;
bool relay1;
bool relay2;
bool buzzer;

// Ngưỡng khí gas và nhiệt độ
const int GAS_THRESHOLD_LOW = 500;   // Ngưỡng thấp (bật quạt)
const int GAS_THRESHOLD_HIGH = 1000; // Ngưỡng cao (bật máy lọc)
const float TEMP_THRESHOLD = 38.0;   // Ngưỡng nhiệt độ (bật quạt)

void setup() {
  Serial.begin(115200);
  
  // Khởi động DHT11
  dht.begin();
  
  // Khởi động OLED
  Wire.begin(21, 22); // SDA: GPIO21, SCL: GPIO22
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED init failed");
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  // Cấu hình các chân
  pinMode(BUZZER, OUTPUT);
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  digitalWrite(BUZZER, LOW);
  digitalWrite(RELAY1, LOW);
  digitalWrite(RELAY2, LOW);
  
  // Kết nối Arduino Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
}

// Hàm xử lý khi relay1 thay đổi từ app
void onRelay1Change() {
  digitalWrite(RELAY1, relay1);
  Serial.print("Relay1 State updated from app: ");
  Serial.println(relay1);
}

// Hàm xử lý khi relay2 thay đổi từ app
void onRelay2Change() {
  digitalWrite(RELAY2, relay2);
  Serial.print("Relay2 State updated from app: ");
  Serial.println(relay2);
}

// Hàm xử lý khi buzzer thay đổi từ app
void onBuzzerChange() {
  digitalWrite(BUZZER, buzzer);
  Serial.print("Buzzer State updated from app: ");
  Serial.println(buzzer);
}

void loop() {
  ArduinoCloud.update(); // Cập nhật kết nối với Arduino Cloud
  
  // Đọc dữ liệu từ DHT11
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  
  // Kiểm tra lỗi đọc DHT11
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT11 sensor!");
    return;
  }
  
  // Đọc dữ liệu từ MQ-2
  gas = analogRead(MQ2PIN); // Giá trị 0-4095
  
  // Hiển thị trên OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Temp: "); display.print(temperature); display.println("C");
  display.print("Humid: "); display.print(humidity); display.println("%");
  display.print("Gas: "); display.print(gas); display.println(" ppm");
  display.display();
  
  // Điều khiển relay và buzzer dựa trên ngưỡng
  if (gas > GAS_THRESHOLD_HIGH || temperature > TEMP_THRESHOLD) {
    buzzer = true;
    relay1 = true;
    relay2 = true;
    digitalWrite(BUZZER, HIGH);  // Bật buzzer
    digitalWrite(RELAY1, HIGH);  // Bật quạt thông gió (LED1 sáng)
    digitalWrite(RELAY2, HIGH);  // Bật máy lọc không khí (LED2 sáng)
    Serial.println("High gas or temp! Both relays ON");
  } else if (gas > GAS_THRESHOLD_LOW) {
    buzzer = false;
    relay1 = true;
    relay2 = false;
    digitalWrite(BUZZER, LOW);   // Tắt buzzer
    digitalWrite(RELAY1, HIGH);  // Bật quạt thông gió (LED1 sáng)
    digitalWrite(RELAY2, LOW);   // Tắt máy lọc (LED2 tắt)
    Serial.println("Moderate gas! Relay 1 ON");
  } else {
    buzzer = false;
    relay1 = false;
    relay2 = false;
    digitalWrite(BUZZER, LOW);   // Tắt buzzer
    digitalWrite(RELAY1, LOW);   // Tắt quạt (LED1 tắt)
    digitalWrite(RELAY2, LOW);   // Tắt máy lọc (LED2 tắt)
  }
  
  // Gửi dữ liệu qua Serial
  Serial.print("Temp: "); Serial.print(temperature);
  Serial.print(" | Humid: "); Serial.print(humidity);
  Serial.print(" | Gas: "); Serial.println(gas);
  
  delay(2000); // Cập nhật mỗi 2 giây
}