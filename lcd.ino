#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
// Cấu hình thông tin Wi-Fi
#define WIFI_SSID "OPPO A53"
#define WIFI_PASSWORD "00000000"
// Cấu hình thông tin Firebase
#define API_KEY "AIzaSyDbn7kJuDQq7QK-8hXG9C66s2vQZJNq_3Y"
#define DATABASE_URL "health-band-cebba-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "YMvSSQgZLukJeXap6CUac1Rf9ssFX3CfyQoYWeNL"
// Khởi tạo màn hình LCD 16x2 sử dụng giao tiếp I2C
LiquidCrystal_I2C lcd(0x27, 16, 2);
// Khởi tạo Firebase
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
// Biến lưu thời gian cập nhật dữ liệu từ Firebase
unsigned long lastUpdateTime = 0;
const unsigned long UPDATE_INTERVAL = 1000; // Chu kỳ cập nhật dữ liệu (1 giây)
void setup() {
  // Khởi động Serial Monitor để kiểm tra quá trình kết nối
  Serial.begin(115200);
  // Khởi tạo và cấu hình màn hình LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  // Kết nối Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  // Cấu hình Firebase
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);
  // Xác nhận Firebase sẵn sàng trên màn hình LCD
  lcd.clear();
  lcd.print("Firebase Ready!");
  delay(1000);
  lcd.clear();
}
void loop() {
  // Kiểm tra nếu đã đủ thời gian cập nhật dữ liệu từ Firebase
  if (millis() - lastUpdateTime > UPDATE_INTERVAL) {
    lastUpdateTime = millis();
    // Đọc dữ liệu nhịp tim và SpO2 từ Firebase
    int heartRate = getFirebaseData("/sensor/heart_rate");
    int spo2 = getFirebaseData("/sensor/spo2");
    // Cập nhật dữ liệu hiển thị lên màn hình LCD
    updateLCD(heartRate, spo2);
  }
}
// Hàm lấy dữ liệu từ Firebase
int getFirebaseData(const String &path) {
  if (Firebase.getInt(fbdo, path)) {
    return fbdo.intData(); // Trả về giá trị dữ liệu nếu lấy thành công
  } else {
    Serial.println("Error reading " + path + ": " + fbdo.errorReason()); // Báo lỗi nếu đọc thất bại
    return -999; // Trả về giá trị lỗi nếu không nhận được dữ liệu
  }
}
// Hàm cập nhật dữ liệu lên màn hình LCD
void updateLCD(int hr, int spo2) {
  // Hiển thị nhịp tim
  lcd.setCursor(0, 0);
  lcd.print("Nhip Tim:      ");
  lcd.setCursor(9, 0);
  if (hr != -999) {
    lcd.print(hr);
    lcd.print(" bpm");
  } else {
    lcd.print("---   "); // Nếu dữ liệu bị lỗi, hiển thị "---"
  }
  // Hiển thị SpO2
  lcd.setCursor(0, 1);
  lcd.print("SpO2:         ");
  lcd.setCursor(6, 1);
  if (spo2 != -999) {
    lcd.print(spo2);
    lcd.print("%");
  } else {
    lcd.print("---  "); // Nếu dữ liệu bị lỗi, hiển thị "---"
  }
}
