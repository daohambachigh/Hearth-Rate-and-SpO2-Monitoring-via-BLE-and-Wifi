#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <SoftwareSerial.h>
// Cấu hình thông tin Wi-Fi
#define WIFI_SSID "your wifi ssid"
#define WIFI_PASSWORD "your wifi pasword"
// Cấu hình thông tin Firebase
#define API_KEY "firebase api key"
#define DATABASE_URL "database url"
#define FIREBASE_AUTH "database "
// Định nghĩa chân RX và TX cho giao tiếp Bluetooth
#define BT_RX D2  
#define BT_TX D3  
// Khởi tạo giao tiếp Bluetooth
SoftwareSerial bluetooth(BT_RX, BT_TX);
// Khởi tạo Firebase
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
// Biến lưu thời gian gửi dữ liệu
unsigned long sendDataPrevMillis = 0;
unsigned long lastDataTime = 0;
bool bluetoothConnected = false;
const unsigned long BLUETOOTH_TIMEOUT = 30000; // Thời gian timeout cho Bluetooth
// Biến lưu dữ liệu nhịp tim và SpO2
int heartRate = 0;  
int spo2 = 0;
bool newData = false;
void setup() {
  // Khởi động Serial Monitor
  Serial.begin(115200);
  // Khởi động giao tiếp Bluetooth
  bluetooth.begin(9600);
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

  Serial.println("Waiting for Bluetooth connection...");
}
void loop() {
  // Đọc dữ liệu từ Bluetooth
  readBluetoothData();
  // Kiểm tra trạng thái kết nối Bluetooth
  checkBluetoothConnection();
  // Gửi dữ liệu lên Firebase khi có dữ liệu mới và thời gian gửi hợp lệ
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 1000) && newData) {
    sendDataPrevMillis = millis();
    // Gửi dữ liệu nhịp tim lên Firebase
    if (Firebase.setInt(fbdo, "/sensor/heart_rate", heartRate)) {
      Serial.print("Heart rate sent: ");
      Serial.println(heartRate);
    } else {
      Serial.println("Heart rate error: " + fbdo.errorReason());
    }
    // Gửi dữ liệu SpO2 lên Firebase
    if (Firebase.setInt(fbdo, "/sensor/spo2", spo2)) {
      Serial.print("SpO2 sent: ");
      Serial.println(spo2);
    } else {
      Serial.println("SpO2 error: " + fbdo.errorReason());
    }
    newData = false; // Đặt lại trạng thái dữ liệu
  }
}
// Kiểm tra trạng thái kết nối Bluetooth
void checkBluetoothConnection() {
  if (bluetoothConnected && (millis() - lastDataTime > BLUETOOTH_TIMEOUT)) {
    bluetoothConnected = false;
    Serial.println("Bluetooth disconnected!");
  }
}
// Đọc dữ liệu từ Bluetooth
void readBluetoothData() {
  static String buffer = "";
  while (bluetooth.available()) {
    char c = bluetooth.read();
    if (c == '\n') {
      parseSensorData(buffer);
      buffer = "";
    } else {
      buffer += c;
    }
  }
}
// Xử lý chuỗi dữ liệu nhận được từ Bluetooth
void parseSensorData(String data) {
  int hrIndex = data.indexOf("temp: ");  
  int spo2Index = data.indexOf("spo2: ");
  if (hrIndex != -1 && spo2Index != -1) {
    lastDataTime = millis(); // Cập nhật thời gian nhận dữ liệu
    // Trích xuất giá trị nhịp tim từ chuỗi dữ liệu
    String hrStr = data.substring(hrIndex + 6, data.indexOf(',', hrIndex));
    heartRate = hrStr.toInt();
    // Trích xuất giá trị SpO2 từ chuỗi dữ liệu
    String spo2Str = data.substring(spo2Index + 6);
    spo2 = spo2Str.toInt();
    if (!bluetoothConnected) {
      bluetoothConnected = true;
      Serial.println("Bluetooth connected!");
    }
    newData = true; // Đánh dấu dữ liệu mới đã nhận
    Serial.println("New data received!");
    Serial.println("Heart rate: " + String(heartRate));  
    Serial.println("SpO2: " + String(spo2));
  }
}

