#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>



// WiFi access point config

const char* ap_ssid = "Compost Monitor C1";
const char* ap_password = "password3141";

int sensor_pin = 12;
int red_led = 13;
int yellow_led = 14;
int green_led = 15;

float PL_CH4 = 0.5; // Permissible Methane Value
int sensorVal;

unsigned long yellow_millis = 0;
const long yellow_b_interval = 6000;   // duration for YELLOW LED to blink

unsigned long green_millis = 0;
const long green_b_interval = 6000;

unsigned long previous_millis = 0;

unsigned long blink_previous_millis = 0;
const long blink_interval = 500; // 500ms blink speed

bool blinkState = false;

WebServer server(80);

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
   <meta charset="UTF-8">
   <meta name="viewport" content="width=device-width, initial-scale=1.0">
   <title>Compost Bin Monitor</title>
   <style>
      body { font-family: Arial, sans-serif; margin: 20px; background-color: #f4f4f4; color: #333; }
      h1 {color: #2c3e50; text-align: center;}
      .data-box { border: 2px solid #333; background-color: #fff; padding: 20px; width: 250px; border-radius: 5px; margin-top: 15px;}
      .gas-level { font-size: 32px; font-weight: bold; margin-top: 10px; color: #d35400; }
      .status-text { font-size: 14px; margin-top: 10px; color: #7f8c8d; }
   </style>
</head>
<body>
  <h1>Organic 360</h1>
  <div class="data-box">
    <p>Live Sensor Value:</p>
    <div class="gas-level">
      <p>Methane: <span id="sensor-val">--</span></p>
      <div id="status" class="status-text">Connecting...</div>
    </div>
  </div>
      

<script>
    async function fetchGasData() {
         try { const response = await fetch('/api/data');
              const data = await response.json();
              document.getElementById('sensor-val').innerText = data.sensor_value;
              document.getElementById('status').innerText = "Status: Online";
        } catch (error) {
            console.error("Error fetching data:", error);
            document.getElementById('status').innerText = "Status: Offline";
        }
     }
     setInterval(fetchGasData, 30000);
</script>

</body>
</html>
)rawliteral";

void handleRoot() {
 server.send(200, "text/html", INDEX_HTML);
}

void handleApiData() {
 StaticJsonDocument<128> doc;
 doc["sensor_value"] = sensorVal;

 String jsonOutput;
 serializeJson(doc, jsonOutput);
 server.send(200, "application/json", jsonOutput);
}


void setup() {
 Serial.begin(115200);
 
 WiFi.softAP(ap_ssid, ap_password);

 Serial.println("\n--- ESp32 Access Point Started ---");
 Serial.print("SSID: ");
 Serial.println(ap_ssid);
 Serial.print("AP IP Adress: ");
 Serial.println(WiFi.softAPIP()); 

 server.on("/", handleRoot);
 server.on("/api/data", handleApiData);
 server.begin();
 
 pinMode(sensor_pin, INPUT);
 pinMode(yellow_led, OUTPUT);
 pinMode(green_led, OUTPUT);
 pinMode(red_led, OUTPUT);

 digitalWrite(green_led, LOW);
 digitalWrite(yellow_led, LOW);
 digitalWrite(red_led, LOW); 
}


void loop(){
  server.handleClient();
 
  unsigned long current_millis = millis();

  sensorVal = digitalRead(sensor_pin);

if (current_millis - previous_millis >= blink_interval) {
  previous_millis = current_millis;
  blinkState = !blinkState; // Toggle state every 500ms

} 

  if (sensorVal >= PL_CH4*2) {
    
    blink_red();  // add a seperate function to make red blink
    digitalWrite(yellow_led, LOW);
    digitalWrite(green_led, LOW);

  } else if (sensorVal < PL_CH4*2 && sensorVal > PL_CH4*1.75){
    digitalWrite(red_led, HIGH);

  }else if (sensorVal < PL_CH4*1.75 && sensorVal >= PL_CH4*1.5 && (current_millis - yellow_millis >= yellow_b_interval)){
    yellow_millis = current_millis;
    blink_yellow();  //add function to make yellow led blink
    digitalWrite(red_led, LOW);
    digitalWrite(green_led, LOW);

  }else if (sensorVal < PL_CH4*1.5 && sensorVal >= PL_CH4*1.25){
    digitalWrite(yellow_led, HIGH);

  }else if (sensorVal < PL_CH4*1.25 && sensorVal > PL_CH4*1 && (current_millis - green_millis >= green_b_interval)) {
    green_millis = current_millis;
    blink_green();
    digitalWrite(red_led, LOW);
    digitalWrite(yellow_led, LOW);

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
