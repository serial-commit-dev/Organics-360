#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// WiFi access point config
const char* ap_ssid = "Compost Monitor C1";
const char* ap_password = "password3141";

int sensor_pin = 34;

int red_led = 13;
int yellow_led = 14;
int green_led = 15;
int buzzer_pin = 27;

float PL_CH4 = 480; // Permissible Methane Value
float sensorVal;

unsigned long yellow_millis = 0;
const long yellow_b_interval = 6000;   // duration for YELLOW LED to blink

unsigned long green_millis = 0;
const long green_b_interval = 6000;

unsigned long previous_millis = 0;

unsigned long blink_previous_millis = 0;
const long blink_interval = 500; // 500ms blink speed

unsigned long buzz_millis = 0;
const long buzz_interval = 2000;

String gas_status = "";

bool blinkState = false;
bool buzzState = false;

WebServer server(80);

// Function prototypes
void blink_red();
void blink_yellow();
void blink_green();

void handleApiData() {
  StaticJsonDocument<128> doc;
  doc["sensor_value"] = sensorVal;
  doc["gas_status"] = gas_status;

  String jsonOutput;
  serializeJson(doc, jsonOutput);
  
  // Allow the tablet server to access this API endpoint across origins
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", jsonOutput);
}

void setup() {
  Serial.begin(115200);
  
  WiFi.softAP(ap_ssid, ap_password);

  Serial.println("\n--- ESP32 Access Point Started ---");
  Serial.print("SSID: ");
  Serial.println(ap_ssid);
  Serial.print("AP IP Address: ");
  Serial.println(WiFi.softAPIP()); 

  server.on("/api/data", handleApiData);
  server.begin();
  
  pinMode(sensor_pin, INPUT);
  pinMode(yellow_led, OUTPUT);
  pinMode(green_led, OUTPUT);
  pinMode(red_led, OUTPUT);
  pinMode(buzzer_pin, OUTPUT);

  digitalWrite(green_led, LOW);
  digitalWrite(yellow_led, LOW);
  digitalWrite(red_led, LOW); 
  digitalWrite(buzzer_pin, LOW);
}

void loop(){
  server.handleClient();
  
  unsigned long current_millis = millis();
  sensorVal = analogRead(sensor_pin);

  if (current_millis - previous_millis >= blink_interval) {
    previous_millis = current_millis;
    blinkState = !blinkState;  // Toggle state every 500ms
  } 

  if (current_millis - buzz_millis >= buzz_interval){
     buzz_millis = current_millis;
     buzzState = !buzzState;
  }   

  if (sensorVal >= PL_CH4*2) {
    blink_red();  
    digitalWrite(buzzer_pin, buzzState ? HIGH : LOW);
    gas_status = "Hazard";
    digitalWrite(yellow_led, LOW);
    digitalWrite(green_led, LOW);

  } else if (sensorVal < PL_CH4*2 && sensorVal > PL_CH4*1.75){
    digitalWrite(red_led, HIGH);
    gas_status = "Danger";

  } else if (sensorVal < PL_CH4*1.75 && sensorVal >= PL_CH4*1.5 && (current_millis - yellow_millis >= yellow_b_interval)){
    yellow_millis = current_millis;
    blink_yellow();  
    gas_status = "Critical";
    digitalWrite(red_led, LOW);
    digitalWrite(green_led, LOW);

  } else if (sensorVal < PL_CH4*1.5 && sensorVal >= PL_CH4*1.25){
    digitalWrite(yellow_led, HIGH);
    gas_status = "Borderline 2";

  } else if (sensorVal < PL_CH4*1.25 && sensorVal > PL_CH4*1 && (current_millis - green_millis >= green_b_interval)) {
    green_millis = current_millis;
    blink_green();
    gas_status = "Borderline 1";
    digitalWrite(red_led, LOW);
    digitalWrite(yellow_led, LOW);

  } else if (sensorVal < PL_CH4) {
     digitalWrite(green_led, HIGH);
     gas_status = "Safe";
  }
}

void blink_red(){
  digitalWrite(red_led, blinkState ? HIGH : LOW);
  digitalWrite(yellow_led, LOW);
  digitalWrite(green_led, LOW);
}

void blink_yellow(){
  digitalWrite(yellow_led, blinkState ? HIGH : LOW);
  digitalWrite(red_led, LOW);
  digitalWrite(green_led, LOW);
}

void blink_green(){
  digitalWrite(green_led, blinkState ? HIGH : LOW);
  digitalWrite(yellow_led, LOW);
  digitalWrite(green_led, LOW); 
}
