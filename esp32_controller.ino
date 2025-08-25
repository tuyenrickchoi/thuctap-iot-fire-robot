
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <iostream>
#include <sstream>
#include <ESP32Servo.h>

#define TILT_PIN 15
#define UART_TX 1
#define UART_BAUDRATE 9600
#define PAN_PIN 13

const int PWMFreq = 1000;
const int PWMResolution = 8;
const int PWMSpeedChannel = 2;

const char *ssid = "Trinh Trương";
const char *password = "123456789";

// ------ STRUCTURES ------
struct MOTOR_PINS {
    int pinEn;
    int pinIN1;
    int pinIN2;
};

std::vector<MOTOR_PINS> motorPins = {
    {14, 2, 4}, // RIGHT_MOTOR
    {16, 0, 12} // LEFT_MOTOR
};

#define RIGHT_MOTOR 0
#define LEFT_MOTOR 1
#define FORWARD 1
#define BACKWARD -1
#define STOP 0
#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4

AsyncWebServer server(80);
AsyncWebSocket wsCarInput("/CarInput");

Servo tiltServo;
Servo panServo;

const char *htmlHomePage PROGMEM = R"HTMLHOMEPAGE(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
.arrows { font-size:30px; color:red; }
td.button {
    background:black; border-radius:25%; box-shadow: 5px 5px #888;
}
td.button:active {
    transform: translate(5px,5px); box-shadow: none;
}
.slider {
    width: 100%; height: 15px; background: #d3d3d3; border-radius: 5px;
    outline: none; opacity: 0.7; transition: opacity .2s;
}
.slider::-webkit-slider-thumb, .slider::-moz-range-thumb {
    width: 25px; height: 25px; background: red; border-radius: 50%;
    cursor: pointer;
}
.pump-btn {
    background: #1976d2; color: #fff; border: none; border-radius: 8px;
    padding: 12px 24px; font-size: 18px; cursor: pointer; margin: 16px 0;
    box-shadow: 2px 2px 8px #888; transition: background 0.2s;
}
.pump-btn:hover { background: #0d47a1; }
</style>
</head>
<body align="center">
<table style="width:400px;margin:auto;" CELLSPACING=10>
<tr><td></td><td class="button" 
    ontouchstart='sendButtonInput("MoveCar","3")' 
    ontouchend='sendButtonInput("MoveCar","0")' 
    onmousedown='sendButtonInput("MoveCar","3")' 
    onmouseup='sendButtonInput("MoveCar","0")'><span 
    class="arrows">&#8679;</span></td><td></td></tr>
<tr>
<td class="button" 
    ontouchstart='sendButtonInput("MoveCar","2")' 
    ontouchend='sendButtonInput("MoveCar","0")' 
    onmousedown='sendButtonInput("MoveCar","2")' 
    onmouseup='sendButtonInput("MoveCar","0")'><span 
    class="arrows">&#8678;</span></td>
<td></td>
<td class="button" 
    ontouchstart='sendButtonInput("MoveCar","1")' 
    ontouchend='sendButtonInput("MoveCar","0")' 
    onmousedown='sendButtonInput("MoveCar","1")' 
    onmouseup='sendButtonInput("MoveCar","0")'><span 
    class="arrows">&#8680;</span></td>
</tr>
<tr><td></td><td class="button" 
    ontouchstart='sendButtonInput("MoveCar","1")' 
    ontouchend='sendButtonInput("MoveCar","0")' 
    onmousedown='sendButtonInput("MoveCar","4")' 
    onmouseup='sendButtonInput("MoveCar","0")'><span 
    class="arrows">&#8681;</span></td><td></td></tr>
<tr><td colspan=3>
<button class="pump-btn" onclick="turnOnPump()">PUMP ON</button>
<button class="pump-btn" onclick="turnOffPump()">PUMP OFF</button>
<div id="pump-status"></div>
</td></tr>
<tr><td align="left"><b>Speed:</b></td><td 
    colspan=2><input type="range" min="0" max="255" value="150" 
    class="slider" id="Speed" 
    oninput='sendButtonInput("Speed",value)'></td></tr>
<tr><td align="left"><b>Tilt:</b></td><td 
    colspan=2><input type="range" min="0" max="180" value="90" 
    class="slider" id="Tilt" 
    oninput='sendButtonInput("Tilt",value)'></td></tr>
<tr><td><b>Pan:</b></td><td colspan=2><input 
    type="range" min="0" max="180" value="90" class="slider" 
    id="Pan" oninput='sendButtonInput("Pan",value)'></td></tr>
</table>
<script>
var websocketCarInput;
function initWebSocket() {
    websocketCarInput = new WebSocket("ws://" + window.location.hostname + "/CarInput");
    websocketCarInput.onopen = function() {
        sendButtonInput("Speed", document.getElementById("Speed").value);
        sendButtonInput("Tilt", document.getElementById("Tilt").value);
        sendButtonInput("Pan", document.getElementById("Pan").value);
    };
    websocketCarInput.onclose = function() {
        setTimeout(initWebSocket, 2000);
    };
}
function sendButtonInput(key, value) {
    websocketCarInput.send(key + "," + value);
}
function turnOnPump() {
    fetch("/on").then(r => r.text()).then(msg => {
        document.getElementById('pump-status').innerText = msg;
    });
}
function turnOffPump() {
    fetch("/off").then(r => r.text()).then(msg => {
        document.getElementById('pump-status').innerText = msg;
    });
}
window.onload = initWebSocket;
</script>
</body>
</html>
)HTMLHOMEPAGE";

// ------ MOTOR CONTROL ------
void rotateMotor(int motorNumber, int motorDirection) {
    digitalWrite(motorPins[motorNumber].pinIN1, motorDirection == FORWARD ? HIGH : LOW);
    digitalWrite(motorPins[motorNumber].pinIN2, motorDirection == BACKWARD ? HIGH : LOW);
    if (motorDirection == STOP) {
        digitalWrite(motorPins[motorNumber].pinIN1, LOW);
        digitalWrite(motorPins[motorNumber].pinIN2, LOW);
    }
}

void moveCar(int inputValue) {
    switch (inputValue) {
        case UP: rotateMotor(RIGHT_MOTOR, BACKWARD); rotateMotor(LEFT_MOTOR, BACKWARD); break;
        case DOWN: rotateMotor(RIGHT_MOTOR, FORWARD); rotateMotor(LEFT_MOTOR, FORWARD); break;
        case LEFT: rotateMotor(RIGHT_MOTOR, BACKWARD); rotateMotor(LEFT_MOTOR, FORWARD); break;
        case RIGHT: rotateMotor(RIGHT_MOTOR, FORWARD); rotateMotor(LEFT_MOTOR, BACKWARD); break;
        default: rotateMotor(RIGHT_MOTOR, STOP); rotateMotor(LEFT_MOTOR, STOP); break;
    }
}

void setUpPinModes() {
    tiltServo.attach(TILT_PIN);
    panServo.attach(PAN_PIN);
    ledcSetup(PWMSpeedChannel, PWMFreq, PWMResolution);
    
    for (int i = 0; i < motorPins.size(); i++) {
        pinMode(motorPins[i].pinIN1, OUTPUT);
        pinMode(motorPins[i].pinIN2, OUTPUT);
        ledcAttachPin(motorPins[i].pinEn, PWMSpeedChannel);
    }
    moveCar(STOP);
}

void handleRoot(AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", htmlHomePage);
}

void handleNotFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "File Not Found");
}

void onCarInputWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_DATA) {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
            std::string payload((char *)data, len);
            std::istringstream ss(payload);
            std::string key, value;
            std::getline(ss, key, ',');
            std::getline(ss, value, ',');
            int val = atoi(value.c_str());
            
            if (key == "MoveCar") moveCar(val);
            else if (key == "Speed") ledcWrite(PWMSpeedChannel, val);
            else if (key == "Tilt") tiltServo.write(val);
            else if (key == "Pan") panServo.write(val);
        }
    } else if (type == WS_EVT_DISCONNECT) {
        moveCar(STOP);
        tiltServo.write(90);
    }
}

void setup() {
    Serial.begin(115200);
    Serial1.begin(UART_BAUDRATE, SERIAL_8N1, -1, UART_TX); // Only TX used
    
    setUpPinModes();
    
    WiFi.softAP(ssid, password);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
    
    server.on("/", HTTP_GET, handleRoot);
    server.onNotFound(handleNotFound);
    
    // Gửi lệnh OFF cho Uno
    server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial1.println("OFF");
        request->send(200, "text/plain", "Đã gửi lệnh OFF tới Uno");
    });
    
    // Gửi lệnh ON cho Uno
    server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial1.println("ON");
        request->send(200, "text/plain", "Đã gửi lệnh ON tới Uno");
    });
    
    wsCarInput.onEvent(onCarInputWebSocketEvent);
    server.addHandler(&wsCarInput);
    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    wsCarInput.cleanupClients();
}
