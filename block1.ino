#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <SoftwareSerial.h>
// Khởi tạo phần mềm Serial cho giao tiếp với JDY-24M (Master)
SoftwareSerial BTSerial(2, 3);
PulseOximeter pox; // Khởi tạo đối tượng đo nhịp tim và SpO2
// Biến lưu thời gian gửi dữ liệu
unsigned long lastSendTime = 0;
// Thời gian giữa hai lần gửi dữ liệu, đặt là 1 giây
unsigned long sendInterval = 1000;
// Hàm callback được gọi khi phát hiện nhịp tim mới
void onBeatDetected() {
}
void setup() {
  // Khởi động Serial Monitor để kiểm tra dữ liệu
  Serial.begin(115200);
  BTSerial.begin(9600); // Thiết lập baud rate cho module Bluetooth JDY-24M
  // Khởi tạo cảm biến MAX30100
  if (!pox.begin()) {
    for (;;);
  }
  // Cấu hình dòng LED IR để tăng độ ổn định
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
  // Đăng ký hàm callback để xử lý sự kiện phát hiện nhịp tim
  pox.setOnBeatDetectedCallback(onBeatDetected);
  Serial.println("Module Master đang hoạt động...");
}
void loop() {
  // Cập nhật dữ liệu từ cảm biến MAX30100
  pox.update();
  // Kiểm tra nếu đủ thời gian để gửi dữ liệu (1 giây)
  if (millis() - lastSendTime >= sendInterval) {
    // Đọc giá trị nhịp tim và SpO2 từ cảm biến
    float heartRate = pox.getHeartRate();
    float spO2 = pox.getSpO2();
    // Chuyển đổi giá trị nhịp tim và SpO2 sang số nguyên
    int heartRateInt = (int)heartRate;
    int spO2Int = (int)spO2;
    // Định dạng dữ liệu trước khi gửi
    String dataToSend = "temp: " + String(heartRateInt) + ", spo2: " + String(spO2Int);
    // Gửi dữ liệu qua Bluetooth đến JDY-24M
    BTSerial.println(dataToSend);
    // Hiển thị dữ liệu lên Serial Monitor để kiểm tra
    Serial.print("Gửi: ");
    Serial.println(dataToSend);
    // Cập nhật lại thời điểm gửi dữ liệu
    lastSendTime = millis();
  }
}
