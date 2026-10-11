 int sensor_pin = 12;
int red_led = 13;
int yellow_led = 14;
int green_led = 15;

float PL_CH4 = 0.5; // Permissible Methane Value
int sensorVal;


unsigned long red_previous_millis = 0;
const long red_b_interval = 6000;   // duration set for RED LED to blink

unsigned long yellow_millis = 0;
const long yellow_b_interval = 6000;   // duration for YELLOW LED to blink

unsigned long green_millis = 0;
const long green_b_interval = 6000;

unsigned long previous_millis = 0;

unsigned long blink_previous_millis = 0;
const long blink_interval = 500; // 500ms blink speed

bool blinkState = false;




void setup() {
  pinMode(sensor_pin, INPUT);
  pinMode(yellow_led, OUTPUT);
  pinMode(green_led, OUTPUT);
  pinMode(red_led, OUTPUT);

  digitalWrite(green_led, LOW);
  digitalWrite(yellow_led, LOW);
  digitalWrite(red_led, LOW);

  
  
}

void loop(){

  unsigned long current_millis = millis();

  sensorVal = digitalRead(sensor_pin);

if (current_millis - previous_millis >= blink_interval) {
  previous_millis = current_millis;
  blinkState = !blinkState; // Toggle state every 500ms

} 

  if (sensorVal >= PL_CH4*2 && (current_millis - red_previous_millis >= red_b_interval)) {
    red_previous_millis = current_millis;
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