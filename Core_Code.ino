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
float sensorVal;

String gas_status = "";

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
      body { font-family: Arial, sans-serif; margin: 0px; padding: 0px; background-color:#98bf64 ; color: #333; }
      h1 {color: #FFF; text-align: center;}
      h2 {color: lightblue; text-align: center;}
      .data-box { margin: 20px; border: 2px solid #333; background-color: #fff; padding: 20px; width: 250px; border-radius: 5px; margin-top: 15px;}
      .gas-level { font-size: 32px; font-weight: bold; margin-top: 10px; color: #d35400; }
      .status-text { font-size: 14px; margin-top: 10px; color: #7f8c8d; }
      .tagline { font-size: 16px; font-weight: bold; color: darkblue; text-align: center; }
      nav {padding: 10px 20px; background-color: darkblue }
      .nav-2 {background-color: lightblue; padding:1px 1px; margin: 0px; }
      .methane {font-size: 20px}
      .info {margin: 0px; padding: 0px 22px; display: flex; gap: 8px;; padding: 10px 0; font-size: 23px; }
      
   </style>
</head>
<body>
  <header>
    <nav>
        <h2>Thiruvananthapuram Municipal Corporation</h3>
        <h1>Organics 360</h1>
       
        
    </nav>

    <div class="nav-2">
       <p class="tagline">Smart Solution For Municipal Organic Waste Corporation</p>
    </div>
</header>

  <div class="info">
    <p class="date"></p>
    <strong>MRF:</strong> Maruthoorkadavu | <strong>Ward:</strong> Kalady | <strong>Health Circle:</strong> Fort
    <div id="live-clock" style="font-weight: bold; color: #008000; font-size: 23px;">
      --
    </div>
  </div>
  
  <div class="data-box">
    <p>Live Sensor Value:</p>
    <div class="gas-level">
      <p class="methane">Methane: <span id="sensor-val">--</span></p>
      <p class="methane">HydrogenSulphide: --</p>
      <div id="status" class="status-text">Connecting...</div>
    </div>
  </div>
      

<script>
    function updateClock() {
      const now = new Date();
      // Formats date and time cleanly (e.g., Oct 11, 2026, 11:55:30 PM)
      const options = { 
        year: 'numeric', month: 'short', day: 'numeric', 
        hour: '2-digit', minute: '2-digit', second: '2-digit' 
      };
      document.getElementById('live-clock').innerText = now.toLocaleDateString('en-US', options);
    }

    // Update every second for a live clock effect
    setInterval(updateClock, 1000);
    updateClock();
    async function fetchGasData() {
         try { const response = await fetch('/api/data');
              const data = await response.json();
              document.getElementById('sensor-val').innerText = data.sensor_value;
              document.getElementById('gas-status').innerText = data.gas_status;
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

 Serial.println("\n--- ESP32 Access Point Started ---");
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
    gas_status = "Hazard";
    digitalWrite(yellow_led, LOW);
    digitalWrite(green_led, LOW);

  } else if (sensorVal < PL_CH4*2 && sensorVal > PL_CH4*1.75){
    digitalWrite(red_led, HIGH);
    gas_status = "Danger";

  }else if (sensorVal < PL_CH4*1.75 && sensorVal >= PL_CH4*1.5){
    blink_yellow();  //add function to make yellow led blink
    gas_status = "Critical";
    digitalWrite(red_led, LOW);
    digitalWrite(green_led, LOW);

  }else if (sensorVal < PL_CH4*1.5 && sensorVal >= PL_CH4*1.25){
    digitalWrite(yellow_led, HIGH);
    gas_status = "Border Line 2";

  }else if (sensorVal < PL_CH4*1.25 && sensorVal > PL_CH4*1) {
    blink_green();
    gas_status = "Border Line 1"
    digitalWrite(red_led, LOW);
    digitalWrite(yellow_led, LOW);

  }else if (sensorVal < PL_CH4) {
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
