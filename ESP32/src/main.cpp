#include <WiFi.h>
#include <ThingSpeak.h>
#include <SoftwareSerial.h>

#define SIM_RX 26
#define SIM_TX 27

SoftwareSerial sim800(SIM_RX, SIM_TX);

const char* ssid = "";
const char* password = "";

unsigned long channelID = "d" ;
const char* writeAPIKey = "";
WiFiClient client;

String phone = "+91";
unsigned long lastCloud =0;
unsigned long lastSMS =0;
String lastFault = "";

void setup(){
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);
  sim800.begin(9600);

  delay(1000);

  //sim800 init
  sim800.println("AT");
  delay(500);
  sim800.println("AT+CMGF=1");
  delay(500);

  // WiFi
  WiFi.begin(ssid,password);
  Serial.print("WiFi: ");
  int t = 0;
  while (WiFi.status() != WL_CONNECTED && t < 30){
    delay(500); Serial.print("."); t++;
  }
  
  Serial.println(WiFi.status() == WL_CONNECTED ? "OK" : "FAIL");
  ThingSpeak.begin(client);
  
}
  

void loop(){
  if (Serial.available()){
    String data = Serial.readStringUntil('\n');
    if(data.startsWith("DATA:")){
      processData(data.substring(5));
    }
  }
  //WiFi reconnect
  if (WiFi.status() != WL_CONNECTED){
    WiFi.reconnect();
    delay(5000);
  }
  delay(50);
}

void processData(String payload){
  int c1 = payload.indexOf(',');
  int c2 = payload.indexOf(',', c1+1);
  int c3 = payload.indexOf(',', c2+1);

  if(c1 < 0 || c2 <0 || c3 < 0) return;

  float vR =payload.substring(0,c1).toFloat();
  float vY =payload.substring(c1+1, c2).toFloat();
  float vB =payload.substring(c2+1,c3).toFloat();
  String fault = payload.substring(c3+1);
  fault.trim();

  //ThingSpeaks every 20 sec

  if (millis() - lastCloud > 20000 && WiFi.status() == WL_CONNECTED){
    ThingSpeak.setField(1, vR);
    ThingSpeak.setField(2, vY);
    ThingSpeak.setField(3, vB);
    ThingSpeak.setField(4, fault.length() > 0 ? 1:0);

    int r = ThingSpeak.writeFields(channelID, writeAPIKey);
    Serial.println(r == 200 ? "Cloud OK" : "Cloud Err:" + String(r));
    lastCloud = millis();
  }

  //SMS on new fault (60 sec gap)

  if (fault.length() > 0 && fault != lastFault && (millis()-lastSMS > 60000)){
    String msg = "FAULT: " + fault + " R=" + String(vR,0) + " Y=" + String(vY,0) + " B=" + String(vB,0);
    sendSMS(phone, msg);
    lastSMS = millis();
  }

  //SMS on new fault (60 sec gap)
  if (fault.length() == 0 && lastFault.length() > 0){
    sendSMS(phone, "POWER RESTORED. All Phases normal.");
  }
  lastFault = fault;
}

void sendSMS(String num, String msg){
  sim800.println("At+CMGF=1");
  delay(500);
  sim800.print("AT+CMGS=\"");
  sim800.print(num);
  sim800.println("\"");
  delay(1000);
  sim800.print(msg);
  delay(300);
  sim800.write(26);
  delay(3000);
  Serial.println("SMS Sent");
}
