#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <avr/wdt.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  // 0x3F bhi try karo

#define SENSOR_R A0
#define SENSOR_Y A1
#define SENSOR_B A2
#define BUZZER 2
#define LED_R 3
#define LED_Y 4
#define LED_B 5

// put function declarations here:
float V_R = 0, V_Y = 0, V_B = 0;
const float V_MIN = 180.0;
const float V_MAX = 270.0;

// Calibration factor - adjust after testing
const float CAL_FACTOR = 0.328;  // Multimeter se match karo

void setup() {
  Serial.begin(9600);
  pinMode(BUZZER, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_Y, OUTPUT);
  pinMode(LED_B, OUTPUT);
  
  lcd.init();
  lcd.backlight();
  lcd.print("3-Phase Monitor");
  delay(2000);
  lcd.clear();
  wdt_enable(WDTO_4S);
}

float readVoltage(int pin) {
  float sumSq = 0;
  int samples = 200;
  
  for (int i = 0; i < samples; i++) {
    int adc = analogRead(pin);
    int centered = adc - 512;  // Remove DC bias
    sumSq += (long)centered * centered;
    delayMicroseconds(100);
  }
  
  float rms = sqrt(sumSq / samples);
  return rms * CAL_FACTOR;
}


void loop() {
  V_R = readVoltage(SENSOR_R);
  V_Y = readVoltage(SENSOR_Y);
  V_B = readVoltage(SENSOR_B);
  
  // LCD Display
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("R:"); 
  lcd.print((int)V_R);
  lcd.print("Y:"); 
  lcd.print((int)V_Y);
  lcd.setCursor(0, 1);
  lcd.print("B:"); 
  lcd.print((int)V_B);

  //Fault detection
  String fault = "";
  if (V_R < V_MIN || V_R > V_MAX) fault += "R ";
  if (V_Y < V_MIN || V_Y > V_MAX) fault += "Y ";
  if (V_B < V_MIN || V_B > V_MAX) fault += "B ";

  if (fault.length() > 0){
    digitalWrite(BUZZER, HIGH);
    lcd.print(" FLT");
  }
  else{
    digitalWrite(BUZZER, LOW);
    lcd.print(" OK");
  }

  //LED INdicator
  digitalWrite(LED_R,(V_R >= V_MIN && V_R <= V_MAX) ? HIGH : LOW);
  digitalWrite(LED_Y,(V_Y >= V_MIN && V_Y <= V_MAX) ? HIGH : LOW);
  digitalWrite(LED_B,(V_B >= V_MIN && V_B <= V_MAX) ? HIGH : LOW);


// Send to esp32
  Serial.print("DATA: ");
  Serial.print(V_R); 
  Serial.print(",");
  Serial.print(V_Y); 
  Serial.print(",");
  Serial.print(V_B); 
  Serial.print(",");

  wdt_reset();
}

