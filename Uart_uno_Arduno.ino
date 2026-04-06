#include <SoftwareSerial.h>

#define SENSOR_PIN 2
#define BUZZER_PIN 3
#define RELAY_PIN 4
#define SPRINKLER_START_DELAY 1000

SoftwareSerial espSerial(8, 9); // RX từ ESP32 TX

unsigned long fireDetectedTime = 0;
bool isFire = false;
unsigned long ignoreSensorEndTime = 0;
bool isTemporarilyIgnoringSensor = false;
bool shouldBypassSprinklerDelay = false;

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(SENSOR_PIN, INPUT);  
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(RELAY_PIN, LOW); // Tắt máy bơm ban đầu
  analogWrite(BUZZER_PIN, 0);   // Tắt còi ban đầu

  Serial.begin(9600);         
  espSerial.begin(9600);      

  Serial.println(" Arduino Uno sẵn sàng!");
  Serial.println(" Đang chờ lệnh từ ESP32...");
}

void loop() {
  if (espSerial.available()) {
    String command = espSerial.readStringUntil('\n');
    command.trim();

    Serial.println("[UART] Nhận lệnh từ ESP32: " + command);

    if (command == "OFF") {
      digitalWrite(RELAY_PIN, LOW);  
      analogWrite(BUZZER_PIN, 0);    
      isTemporarilyIgnoringSensor = true;
      ignoreSensorEndTime = millis() + 3000; 
      shouldBypassSprinklerDelay = true; 
      Serial.println(">>> Đã TẮT máy bơm & còi (OFF). Bỏ qua cảm biến 3 giây.");
    }

    else if (command == "ON") {
      digitalWrite(RELAY_PIN, HIGH);   // Bật máy bơm
      analogWrite(BUZZER_PIN, 40);     // Bật còi nhẹ
      isTemporarilyIgnoringSensor = true;
      ignoreSensorEndTime = millis() + 3000; // Bỏ qua cảm biến 3s để tránh can thiệp ngay
      shouldBypassSprinklerDelay = false;
      Serial.println(">>> Đã BẬT máy bơm thủ công (ON). Bỏ qua cảm biến 3 giây.");
    }
  }

 
  if (isTemporarilyIgnoringSensor) {
    if (millis() >= ignoreSensorEndTime) {
      isTemporarilyIgnoringSensor = false;
      Serial.println("[TIMER] Thời gian bỏ qua cảm biến đã hết.");
    } else {
      return; 
    }
  }

  int sensorValue = digitalRead(SENSOR_PIN);

  if (sensorValue == LOW) { 
    analogWrite(BUZZER_PIN, 40);

    if (shouldBypassSprinklerDelay) {
      digitalWrite(RELAY_PIN, HIGH);
      Serial.println("[💧] Lửa còn phát hiện sau OFF → bật máy bơm lại!");
      isFire = true;
      fireDetectedTime = millis();
      shouldBypassSprinklerDelay = false;
    } else {
      if (!isFire) {
        fireDetectedTime = millis();
        isFire = true;
        Serial.println("Phát hiện lửa → bắt đầu đếm trễ...");
      }

      if (millis() - fireDetectedTime >= SPRINKLER_START_DELAY) {
        digitalWrite(RELAY_PIN, HIGH);
        Serial.println("[💧] Bật máy bơm vì có lửa quá thời gian trễ!");
      }
    }
  } else {
    if (isFire) Serial.println("[✔] Không còn lửa → tắt máy bơm");
    analogWrite(BUZZER_PIN, 0);
    digitalWrite(RELAY_PIN, LOW);
    isFire = false;
    shouldBypassSprinklerDelay = false;
  }
}
