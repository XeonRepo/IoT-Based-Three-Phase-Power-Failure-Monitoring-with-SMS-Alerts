#include <ZMPT101B.h>
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>

// Phase sensor definitions
#define SENSITIVITY 500.0
#define PHASE_R_PIN A0    // Phase R (Red)
#define PHASE_Y_PIN A1    // Phase Y (Yellow)  
#define PHASE_B_PIN A2    // Phase B (Blue)

// LED Pin definitions
#define LED_R 1      // Red LED for Phase R
#define LED_Y 2      // Yellow LED for Phase Y  
#define LED_B 3      // Blue LED for Phase B

// Voltage thresholds
#define LOW_VOLTAGE_THRESHOLD 100.0
#define POWER_CUT_THRESHOLD 80.0

// LCD pin configuration
LiquidCrystal lcd(13, 12, 11, 10, 9, 8);

// GSM module configuration (SoftwareSerial)
SoftwareSerial gsmSerial(7, 6); // RX, TX

// NodeMCU communication (SoftwareSerial) - UPDATED PINS
SoftwareSerial nodeSerial(4, 5); // RX, TX - Connected to NodeMCU D2,D3

// Create voltage sensor objects for each phase
ZMPT101B voltageSensorR(PHASE_R_PIN);
ZMPT101B voltageSensorY(PHASE_Y_PIN);
ZMPT101B voltageSensorB(PHASE_B_PIN);

String message;
// Variables for voltage readings
float voltageR = 0.0;
float voltageY = 0.0;
float voltageB = 0.0;
unsigned long previousMillis = 0;
unsigned long previousLEDMillis = 0;
const long interval = 1000; // Update interval in milliseconds
const long ledInterval = 500; // LED blink interval

String msg2="R Phase Cut";
String msg3="Y Phase Cut";
String msg4="B Phase Cut";
// SMS alert variables
bool smsSentR = false;
bool smsSentY = false; 
bool smsSentB = false;
bool powerCutSmsSent = false;
String phoneNumbers[5] = {
  "+918318107447", // Replace with actual numbers
  "",
  "", 
  "",
  ""
};

// LED state variables
bool ledRState = false;
bool ledYState = false;
bool ledBState = false;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  
  // Initialize GSM serial
  gsmSerial.begin(9600);
  
  // Initialize NodeMCU serial - UPDATED PINS
  nodeSerial.begin(9600);
  
  // Set sensitivity for all sensors
  voltageSensorR.setSensitivity(SENSITIVITY);
  voltageSensorY.setSensitivity(SENSITIVITY);
  voltageSensorB.setSensitivity(SENSITIVITY);
  
  // Initialize LED pins
  pinMode(LED_R, OUTPUT);
  pinMode(LED_Y, OUTPUT);
  pinMode(LED_B, OUTPUT);
  
  // Initialize LCD
  lcd.begin(16, 2);
  lcd.print("3-Phase Monitor");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  
  // Initialize GSM module
  initializeGSM();
  
  // Turn on all LEDs briefly to test
  digitalWrite(LED_R, HIGH);
  digitalWrite(LED_Y, HIGH);
  digitalWrite(LED_B, HIGH);
  delay(1000);
  digitalWrite(LED_R, LOW);
  digitalWrite(LED_Y, LOW);
  digitalWrite(LED_B, LOW);
  
  delay(2000);
  lcd.clear();
  
  Serial.println("3-Phase Voltage Monitoring with GSM Started");
  Serial.println("Phase_R(V)\tPhase_Y(V)\tPhase_B(V)");
  Serial.println("----------------------------------------");
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Read voltages at specified interval
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    // Read RMS voltages from all three phases
    voltageR = voltageSensorR.getRmsVoltage();
    voltageY = voltageSensorY.getRmsVoltage();
    voltageB = voltageSensorB.getRmsVoltage();
    
    // Display on LCD
    displayVoltagesOnLCD();
    
    // Print to Serial Monitor
    printToSerial();
    
    // Update LED indicators
    updateLEDs();
    
    // Check voltage conditions and send alerts
    checkVoltageConditions();
    
    // Send data to NodeMCU for web display
    sendToNodeMCU();
  }
  
  // Handle LED blinking for low voltage conditions
  if (currentMillis - previousLEDMillis >= ledInterval) {
    previousLEDMillis = currentMillis;
    handleLEDBlinking();
  }
  
  // Check for GSM responses
  checkGSMResponse();
  
  // Check for NodeMCU commands
  checkNodeMCUCommands();
  
  delay(100);
}

void updateLEDs() {
  // Update Phase R LED
  if (voltageR >= 100.0) {
    digitalWrite(LED_R, HIGH); // Solid ON for normal voltage
  } else if (voltageR >= 80.0 && voltageR < 100.0) {
    // Blinking handled in handleLEDBlinking()
  } else {
    digitalWrite(LED_R, LOW); // OFF for power cut
  }
  
  // Update Phase Y LED
  if (voltageY >= 100.0) {
    digitalWrite(LED_Y, HIGH); // Solid ON for normal voltage
  } else if (voltageY >= 80.0 && voltageY < 100.0) {
    // Blinking handled in handleLEDBlinking()
  } else {
    digitalWrite(LED_Y, LOW); // OFF for power cut
  }
  
  // Update Phase B LED
  if (voltageB >= 100.0) {
    digitalWrite(LED_B, HIGH); // Solid ON for normal voltage
  } else if (voltageB >= 80.0 && voltageB < 100.0) {
    // Blinking handled in handleLEDBlinking()
  } else {
    digitalWrite(LED_B, LOW); // OFF for power cut
  }
}

void handleLEDBlinking() {
  // Toggle LED states for blinking
  ledRState = !ledRState;
  ledYState = !ledYState;
  ledBState = !ledBState;
  
  // Phase R blinking for low voltage
  if (voltageR >= 80.0 && voltageR < 100.0) {
    digitalWrite(LED_R, ledRState);
  }
  
  // Phase Y blinking for low voltage
  if (voltageY >= 80.0 && voltageY < 100.0) {
    digitalWrite(LED_Y, ledYState);
  }
  
  // Phase B blinking for low voltage
  if (voltageB >= 80.0 && voltageB < 100.0) {
    digitalWrite(LED_B, ledBState);
  }
}

void displayVoltagesOnLCD() {
  lcd.clear();
  
  // Display Phase R and Phase Y on first line
  lcd.setCursor(0, 0);
  lcd.print("R:");
  lcd.print(voltageR, 1);
  lcd.print("V");
  
  lcd.setCursor(8, 0);
  lcd.print("Y:");
  lcd.print(voltageY, 1);
  lcd.print("V");
  
  // Display Phase B and Status on second line
  lcd.setCursor(0, 1);
  lcd.print("B:");
  lcd.print(voltageB, 1);
  lcd.print("V");
  
  // Display status indicator
  lcd.setCursor(10, 1);
  if (isPowerCut()) {
    lcd.print("CUT");
  } else if (isLowVoltage()) {
    lcd.print("LOW");
  } else {
    lcd.print("OK");
  }
}

void printToSerial() {
  Serial.print("R:");
  Serial.print(voltageR);
  Serial.print("\tY:");
  Serial.print(voltageY);
  Serial.print("\tB:");
  Serial.print(voltageB);
  
  if (isPowerCut()) {
    Serial.println("\t*** POWER CUT DETECTED ***");
  } else if (isLowVoltage()) {
    Serial.println("\t*** LOW VOLTAGE DETECTED ***");
  } else {
    Serial.println("\tNormal");
  }
}

void checkVoltageConditions() {
  // Check for power cut (all phases below threshold)
  if (isPowerCut()) {
    if (!powerCutSmsSent) {
      message = "POWER CUT ALERT! All phases cut.";
      sendAlertSMS(message);
      powerCutSmsSent = true;
      // Reset individual phase flags
      smsSentR = false;
      smsSentY = false;
      smsSentB = false;
    }
    return;
  }
  
  // Reset power cut flag if power is restored
  if (powerCutSmsSent && !isPowerCut()) {
    powerCutSmsSent = false;
    message = "POWER RESTORED! All phases normal.";
    sendAlertSMS(message);
  }
  
  // Check individual phases for low voltage
  if (voltageR < LOW_VOLTAGE_THRESHOLD && voltageR > 10) {
    if (!smsSentR) {
      
      sendAlertSMS(msg2);
      smsSentR = true;
    }
  } else if (smsSentR && voltageR >= LOW_VOLTAGE_THRESHOLD) {
    // Voltage restored for Phase R
    smsSentR = false;
    message = "PHASE R (RED) VOLTAGE RESTORED!";
    sendAlertSMS(message);
  }
  
  if (voltageY < LOW_VOLTAGE_THRESHOLD && voltageY > 10) {
    if (!smsSentY) {
      
      sendAlertSMS(msg3);
      smsSentY = true;
    }
  } else if (smsSentY && voltageY >= LOW_VOLTAGE_THRESHOLD) {
    // Voltage restored for Phase Y
    smsSentY = false;
    message = "PHASE Y (YELLOW) VOLTAGE RESTORED!";
    sendAlertSMS(message);
  }
  
  if (voltageB < LOW_VOLTAGE_THRESHOLD && voltageB > 10) {
    if (!smsSentB) {

      sendAlertSMS(msg4);
      smsSentB = true;
    }
  } else if (smsSentB && voltageB >= LOW_VOLTAGE_THRESHOLD) {
    // Voltage restored for Phase B
    smsSentB = false;
    message = "PHASE B (BLUE) VOLTAGE RESTORED!";
    sendAlertSMS(message);
  }
}

bool isPowerCut() {
  return (voltageR < POWER_CUT_THRESHOLD && voltageY < POWER_CUT_THRESHOLD && voltageB < POWER_CUT_THRESHOLD);
}

bool isLowVoltage() {
  return (voltageR < LOW_VOLTAGE_THRESHOLD || voltageY < LOW_VOLTAGE_THRESHOLD || voltageB < LOW_VOLTAGE_THRESHOLD);
}

void initializeGSM() {
  Serial.println("Initializing GSM Module...");
  lcd.clear();
  lcd.print("GSM Initializing");
  
  // Reset GSM module
  delay(5000); // Wait for GSM module to power up
  
  // Clear any pending data
  while(gsmSerial.available()) {
    gsmSerial.read();
  }
  
  Serial.println("Testing GSM communication...");
  
  // Test with simple AT command
  if (sendATCommand("AT", "OK", 10000)) {
    Serial.println("GSM Communication OK");
    
    // Continue with other commands
    if (sendATCommand("AT+CPIN?", "READY", 10000)) {
      Serial.println("SIM Card Ready");
    } else {
      Serial.println("SIM Card Issue");
    }
    
    sendATCommand("AT+CMGF=1", "OK", 5000); // Set SMS text mode
    sendATCommand("AT+CNMI=2,2,0,0,0", "OK", 5000); // Set SMS notification
    
    Serial.println("GSM Module Ready");
    lcd.clear();
    lcd.print("GSM Ready");
  
  }
  delay(1000);
}

bool sendATCommand(String command, String expectedResponse, int timeout) {
  Serial.println("Sending: " + command);
  gsmSerial.println(command);
  
  unsigned long startTime = millis();
  String response = "";
  
  while (millis() - startTime < timeout) {
    if (gsmSerial.available()) {
      char c = gsmSerial.read();
      response += c;
      Serial.write(c); // Print GSM responses to serial
      
      if (response.indexOf(expectedResponse) != -1) {
        Serial.println("\nCommand OK");
        return true;
      }
    }
  }
  
  Serial.println("\nTimeout - No response: " + command);
  Serial.println("Received: " + response);
  return false;
}

void sendAlertSMS(String message) {
  Serial.println("Sending SMS: " + message);
  
  for (int i = 0; i < 5; i++) {
    if (phoneNumbers[i] != "") {
      sendSMS(phoneNumbers[i], message);
      delay(2000); // Delay between sending to different numbers
    }
  }
}

void sendSMS(String phoneNumber, String message) {
  gsmSerial.println("AT+CMGS=\"" + phoneNumber + "\"");
  delay(1000);
  gsmSerial.print(message);
  delay(500);
  gsmSerial.write(26); // CTRL+Z to send message
  delay(2000);
  
  Serial.println("SMS sent to: " + phoneNumber);
}

void checkGSMResponse() {
  if (gsmSerial.available()) {
    String response = gsmSerial.readString();
    Serial.println("GSM Response: " + response);
  }
}

void sendToNodeMCU() {
  // Send data to NodeMCU in a robust format
  String data = "DATA:" + String(voltageR, 1) + "," + String(voltageY, 1) + "," + String(voltageB, 1);
  nodeSerial.println(data);
  
  // Debug output
  Serial.print("Sent to NodeMCU: ");
  Serial.println(data);
}

void checkNodeMCUCommands() {
  if (nodeSerial.available()) {
    String command = nodeSerial.readStringUntil('\n');
    command.trim();
    
    Serial.print("Received from NodeMCU: ");
    Serial.println(command);
    
    if (command == "HELLO from NodeMCU") {
      nodeSerial.println("HELLO from Arduino");
      Serial.println("Responded to NodeMCU hello");
    }
    else if (command.startsWith("PHONE:")) {
      // Handle phone number updates
      int firstColon = command.indexOf(':');
      int secondColon = command.indexOf(':', firstColon + 1);
      
      if (firstColon != -1 && secondColon != -1) {
        int index = command.substring(firstColon + 1, secondColon).toInt();
        String phone = command.substring(secondColon + 1);
        
        if (index >= 0 && index < 5) {
          phoneNumbers[index] = phone;
          Serial.println("Updated phone " + String(index) + ": " + phone);
        }
      }
    }
    else if (command == "TEST_SMS") {
      String testMessage = "TEST SMS: System Working! R:" + String(voltageR,1) + "V";
      sendAlertSMS(testMessage);
      Serial.println("Test SMS sent");
    }
  }
}