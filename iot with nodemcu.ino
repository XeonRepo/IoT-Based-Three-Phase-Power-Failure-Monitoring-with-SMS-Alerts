#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>

// WiFi credentials
const char* ssid = "Android_Xeon";
const char* password = "Railways@221002";

// Web server on port 80
ESP8266WebServer server(80);

// SoftwareSerial for Arduino communication
SoftwareSerial arduinoSerial(D2, D3); // RX, TX - D2 to Arduino pin 4, D3 to Arduino pin 5

// Voltage data storage
float voltageR = 0.0;
float voltageY = 0.0;
float voltageB = 0.0;
unsigned long lastUpdate = 0;
bool dataReceived = false;
bool connectionStatus = false;

// Phone numbers storage
String phoneNumbers[5] = {"", "", "", "", ""};

// System status
String systemStatus = "Initializing";
String lastAlert = "No alerts";
unsigned long systemStartTime = 0;

// Function declarations
void handleRoot();
void handleData();
void handleCommand();
void handlePhoneNumbers();
void parseData(String data);

// HTML webpage with advanced design
const char* webpage = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>3-Phase Power Monitoring System</title>
    <style>
        :root {
            --primary: #2c3e50;
            --secondary: #34495e;
            --accent-r: #e74c3c;
            --accent-y: #f39c12;
            --accent-b: #3498db;
            --success: #27ae60;
            --warning: #f39c12;
            --danger: #e74c3c;
            --text-light: #ecf0f1;
            --text-dark: #2c3e50;
            --card-bg: #ffffff;
            --shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
        }

        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: var(--text-dark);
            min-height: 100vh;
            padding: 20px;
        }

        .container {
            max-width: 1400px;
            margin: 0 auto;
        }

        .header {
            text-align: center;
            margin-bottom: 30px;
            color: white;
        }

        .header h1 {
            font-size: 2.5rem;
            margin-bottom: 10px;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
        }

        .header p {
            font-size: 1.1rem;
            opacity: 0.9;
        }

        .dashboard {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 20px;
            margin-bottom: 20px;
        }

        @media (max-width: 1024px) {
            .dashboard {
                grid-template-columns: 1fr;
            }
        }

        .card {
            background: var(--card-bg);
            border-radius: 15px;
            padding: 25px;
            box-shadow: var(--shadow);
            backdrop-filter: blur(10px);
        }

        .card h2 {
            color: var(--primary);
            margin-bottom: 20px;
            font-size: 1.4rem;
            border-bottom: 2px solid var(--primary);
            padding-bottom: 10px;
        }

        .status-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 15px;
            margin-bottom: 20px;
        }

        .phase-status {
            padding: 20px;
            border-radius: 10px;
            text-align: center;
            color: white;
            transition: transform 0.3s ease;
        }

        .phase-status:hover {
            transform: translateY(-5px);
        }

        .phase-r { background: linear-gradient(135deg, var(--accent-r), #c0392b); }
        .phase-y { background: linear-gradient(135deg, var(--accent-y), #e67e22); }
        .phase-b { background: linear-gradient(135deg, var(--accent-b), #2980b9); }

        .voltage-value {
            font-size: 2rem;
            font-weight: bold;
            margin: 10px 0;
        }

        .status-indicator {
            font-size: 0.9rem;
            padding: 5px 10px;
            border-radius: 20px;
            background: rgba(255,255,255,0.2);
        }

        .system-info {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 15px;
        }

        .info-item {
            background: #f8f9fa;
            padding: 15px;
            border-radius: 8px;
            border-left: 4px solid var(--primary);
        }

        .alert-banner {
            background: var(--danger);
            color: white;
            padding: 15px;
            border-radius: 10px;
            margin-bottom: 20px;
            display: none;
            animation: pulse 2s infinite;
        }

        @keyframes pulse {
            0% { opacity: 1; }
            50% { opacity: 0.7; }
            100% { opacity: 1; }
        }

        .phone-numbers {
            background: var(--card-bg);
        }

        .phone-form {
            display: grid;
            gap: 10px;
        }

        .form-group {
            display: flex;
            gap: 10px;
            align-items: center;
        }

        .form-group label {
            min-width: 100px;
            font-weight: bold;
        }

        input[type="text"] {
            flex: 1;
            padding: 10px;
            border: 2px solid #ddd;
            border-radius: 5px;
            font-size: 1rem;
            transition: border-color 0.3s ease;
        }

        input[type="text"]:focus {
            outline: none;
            border-color: var(--primary);
        }

        button {
            padding: 12px 25px;
            border: none;
            border-radius: 8px;
            background: var(--primary);
            color: white;
            font-size: 1rem;
            cursor: pointer;
            transition: all 0.3s ease;
        }

        button:hover {
            background: var(--secondary);
            transform: translateY(-2px);
        }

        .btn-success { background: var(--success); }
        .btn-warning { background: var(--warning); }
        .btn-danger { background: var(--danger); }

        .controls {
            display: flex;
            gap: 10px;
            flex-wrap: wrap;
            margin-top: 15px;
        }

        .connection-status {
            padding: 10px 15px;
            border-radius: 20px;
            font-weight: bold;
            text-align: center;
        }

        .connected { background: var(--success); color: white; }
        .disconnected { background: var(--danger); color: white; }

        .last-update {
            text-align: center;
            color: #666;
            font-style: italic;
            margin-top: 10px;
        }

        .grid-2col {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 20px;
        }

        @media (max-width: 768px) {
            .grid-2col {
                grid-template-columns: 1fr;
            }
            
            .header h1 {
                font-size: 2rem;
            }
            
            .voltage-value {
                font-size: 1.5rem;
            }
        }

        .notification {
            position: fixed;
            top: 20px;
            right: 20px;
            padding: 15px 20px;
            border-radius: 8px;
            color: white;
            z-index: 1000;
            transform: translateX(400px);
            transition: transform 0.3s ease;
        }

        .notification.show {
            transform: translateX(0);
        }

        .notification.success { background: var(--success); }
        .notification.error { background: var(--danger); }
        .notification.warning { background: var(--warning); }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>⚡ 3-Phase Power Monitoring System</h1>
            <p>Real-time voltage monitoring with GSM alerts</p>
        </div>

        <div class="alert-banner" id="alertBanner">
            <strong id="alertText">Alert Message</strong>
        </div>

        <div class="dashboard">
            <!-- Left Column -->
            <div>
                <!-- Real-time Voltage Display -->
                <div class="card">
                    <h2>📊 Real-time Voltage Status</h2>
                    <div class="status-grid">
                        <div class="phase-status phase-r">
                            <h3>PHASE R</h3>
                            <div class="voltage-value" id="voltageR">0.0 V</div>
                            <div class="status-indicator" id="statusR">WAITING</div>
                        </div>
                        <div class="phase-status phase-y">
                            <h3>PHASE Y</h3>
                            <div class="voltage-value" id="voltageY">0.0 V</div>
                            <div class="status-indicator" id="statusY">WAITING</div>
                        </div>
                        <div class="phase-status phase-b">
                            <h3>PHASE B</h3>
                            <div class="voltage-value" id="voltageB">0.0 V</div>
                            <div class="status-indicator" id="statusB">WAITING</div>
                        </div>
                    </div>
                    
                    <div class="connection-status" id="connectionStatus">
                        Connecting to Arduino...
                    </div>
                </div>

                <!-- System Statistics Card -->
                <div class="card">
                    <h2>📈 System Statistics</h2>
                    <div class="system-info">
                        <div class="info-item">
                            <strong>Current Status:</strong><br>
                            <span id="currentStatus">Normal</span>
                        </div>
                        <div class="info-item">
                            <strong>Data Updates:</strong><br>
                            <span id="dataUpdates">0</span>
                        </div>
                        <div class="info-item">
                            <strong>Last SMS:</strong><br>
                            <span id="lastSmsTime">Never</span>
                        </div>
                        <div class="info-item">
                            <strong>System Health:</strong><br>
                            <span id="systemHealth">Good</span>
                        </div>
                    </div>
                </div>
            </div>

            <!-- Right Column -->
            <div>
                <!-- System Information -->
                <div class="card">
                    <h2>🖥️ System Information</h2>
                    <div class="system-info">
                        <div class="info-item">
                            <strong>System Status:</strong><br>
                            <span id="systemStatus">Initializing</span>
                        </div>
                        <div class="info-item">
                            <strong>Uptime:</strong><br>
                            <span id="systemUptime">00:00:00</span>
                        </div>
                        <div class="info-item">
                            <strong>Last Alert:</strong><br>
                            <span id="lastAlert">No alerts</span>
                        </div>
                        <div class="info-item">
                            <strong>WiFi Strength:</strong><br>
                            <span id="wifiStrength">--</span>
                        </div>
                    </div>
                </div>

                <!-- SMS Management -->
                <div class="card phone-numbers">
                    <h2>📱 SMS Alert Numbers</h2>
                    <div class="phone-form">
                        <div class="form-group">
                            <label>Number 1:</label>
                            <input type="text" id="phone1" placeholder="+919876543210">
                            <button onclick="updatePhoneNumber(0)" class="btn-success">💾 Save</button>
                        </div>
                        <div class="form-group">
                            <label>Number 2:</label>
                            <input type="text" id="phone2" placeholder="+919876543211">
                            <button onclick="updatePhoneNumber(1)" class="btn-success">💾 Save</button>
                        </div>
                        <div class="form-group">
                            <label>Number 3:</label>
                            <input type="text" id="phone3" placeholder="+919876543212">
                            <button onclick="updatePhoneNumber(2)" class="btn-success">💾 Save</button>
                        </div>
                        <div class="form-group">
                            <label>Number 4:</label>
                            <input type="text" id="phone4" placeholder="+919876543213">
                            <button onclick="updatePhoneNumber(3)" class="btn-success">💾 Save</button>
                        </div>
                        <div class="form-group">
                            <label>Number 5:</label>
                            <input type="text" id="phone5" placeholder="+919876543214">
                            <button onclick="updatePhoneNumber(4)" class="btn-success">💾 Save</button>
                        </div>
                    </div>
                    
                    <div class="controls">
                        <button onclick="sendCommand('TEST_SMS')" class="btn-warning">📤 Send Test SMS</button>
                        <button onclick="loadPhoneNumbers()" class="btn-success">🔄 Reload Numbers</button>
                        <button onclick="clearAllNumbers()" class="btn-danger">🗑️ Clear All</button>
                    </div>
                </div>

                <!-- System Controls -->
                <div class="card">
                    <h2>⚙️ System Controls</h2>
                    <div class="controls">
                        <button onclick="sendCommand('STATUS')">📊 Get Status</button>
                        <button onclick="location.reload()">🔄 Refresh Page</button>
                        <button onclick="sendCommand('REBOOT')">🔄 Reboot System</button>
                    </div>
                    <div class="last-update">
                        Last Updated: <span id="lastUpdate">Never</span>
                    </div>
                </div>
            </div>
        </div>
    </div>

    <!-- Notification System -->
    <div class="notification" id="notification"></div>

    <script>
        let dataUpdateCount = 0;
        let lastSmsTime = "Never";

        // Notification system
        function showNotification(message, type = 'success') {
            const notification = document.getElementById('notification');
            notification.textContent = message;
            notification.className = `notification ${type} show`;
            
            setTimeout(() => {
                notification.classList.remove('show');
            }, 3000);
        }

        // Update display with new data
        // Update display with new data
function updateDisplay(data) {
    dataUpdateCount++;
    
    // Update voltage values - FIXED: Show actual voltage even if 0
    document.getElementById('voltageR').textContent = data.voltageR.toFixed(1) + ' V';
    document.getElementById('voltageY').textContent = data.voltageY.toFixed(1) + ' V';
    document.getElementById('voltageB').textContent = data.voltageB.toFixed(1) + ' V';
    
    // Update status indicators - FIXED LOGIC
    updateStatus('R', data.voltageR);
    updateStatus('Y', data.voltageY);
    updateStatus('B', data.voltageB);
    
    // Update connection status
    const statusElement = document.getElementById('connectionStatus');
    if (data.connected) {
        statusElement.className = 'connection-status connected';
        statusElement.innerHTML = '✅ Connected to Arduino';
    } else {
        statusElement.className = 'connection-status disconnected';
        statusElement.innerHTML = '❌ Disconnected from Arduino';
    }
    
    // Update system info
    document.getElementById('systemStatus').textContent = data.systemStatus;
    document.getElementById('lastAlert').textContent = data.lastAlert;
    document.getElementById('dataUpdates').textContent = dataUpdateCount;
    
    // Update statistics
    updateStatistics(data);
    
    // Update last update time
    document.getElementById('lastUpdate').textContent = new Date().toLocaleTimeString();
    
    // Check for alerts - FIXED LOGIC
    checkAlerts(data);
}

function updateStatus(phase, voltage) {
    const statusElement = document.getElementById('status' + phase);
    const currentStatusElement = document.getElementById('currentStatus');
    
    // FIXED: Enhanced logic for phase status including 0-10V fluctuation
    if (voltage >= 100.0) {
        statusElement.textContent = 'NORMAL';
        statusElement.style.background = 'rgba(39, 174, 96, 0.8)';
    } else if (voltage >= 80.0 && voltage < 100.0) {
        statusElement.textContent = 'LOW';
        statusElement.style.background = 'rgba(243, 156, 18, 0.8)';
    } else if (voltage >= 10.0 && voltage < 80.0) {
        statusElement.textContent = 'CUT';
        statusElement.style.background = 'rgba(231, 76, 60, 0.8)';
    } else if (voltage >= 0.0 && voltage < 10.0) {
        statusElement.textContent = 'POWER CUT';
        statusElement.style.background = 'rgba(231, 76, 60, 0.8)';
    } else {
        statusElement.textContent = 'NO DATA';
        statusElement.style.background = 'rgba(149, 165, 166, 0.8)';
    }
    
    // Update overall current status
    if ((voltageR >= 0.0 && voltageR < 10.0) && (voltageY >= 0.0 && voltageY < 10.0) && (voltageB >= 0.0 && voltageB < 10.0)) {
        currentStatusElement.textContent = 'Complete Power Cut';
        currentStatusElement.style.color = '#e74c3c';
    } else if (voltageR < 50 || voltageY < 50 || voltageB < 50) {
        currentStatusElement.textContent = 'Phase Cut';
        currentStatusElement.style.color = '#e74c3c';
    } else if (voltageR < 100 || voltageY < 100 || voltageB < 100) {
        currentStatusElement.textContent = 'Low Voltage';
        currentStatusElement.style.color = '#f39c12';
    } else if (voltageR > 0 && voltageY > 0 && voltageB > 0) {
        currentStatusElement.textContent = 'Normal';
        currentStatusElement.style.color = '#27ae60';
    } else {
        currentStatusElement.textContent = 'No Data';
        currentStatusElement.style.color = '#95a5a6';
    }
}

function checkAlerts(data) {
    const alertBanner = document.getElementById('alertBanner');
    const alertText = document.getElementById('alertText');
    
    // FIXED: Enhanced alert logic including 0-10V fluctuation detection
    if ((data.voltageR >= 0.0 && data.voltageR < 10.0) && 
        (data.voltageY >= 0.0 && data.voltageY < 10.0) && 
        (data.voltageB >= 0.0 && data.voltageB < 10.0)) {
        alertText.textContent = '🚨 CRITICAL: Complete Power Cut Detected (0-10V Fluctuation)!';
        alertBanner.style.display = 'block';
        alertBanner.style.background = 'linear-gradient(135deg, #e74c3c, #c0392b)';
    } else if (data.voltageR < 50 && data.voltageY < 50 && data.voltageB < 50 && data.voltageR > 0) {
        alertText.textContent = '🚨 CRITICAL: Complete Power Cut Detected!';
        alertBanner.style.display = 'block';
        alertBanner.style.background = 'linear-gradient(135deg, #e74c3c, #c0392b)';
    } else if (((data.voltageR >= 0.0 && data.voltageR < 10.0) || 
               (data.voltageY >= 0.0 && data.voltageY < 10.0) || 
               (data.voltageB >= 0.0 && data.voltageB < 10.0)) ||
               ((data.voltageR < 50 && data.voltageR > 0) || 
               (data.voltageY < 50 && data.voltageY > 0) || 
               (data.voltageB < 50 && data.voltageB > 0))) {
        // Count how many phases are cut (including 0-10V fluctuation)
        let cutPhases = 0;
        if ((data.voltageR >= 0.0 && data.voltageR < 10.0) || (data.voltageR < 50 && data.voltageR > 0)) cutPhases++;
        if ((data.voltageY >= 0.0 && data.voltageY < 10.0) || (data.voltageY < 50 && data.voltageY > 0)) cutPhases++;
        if ((data.voltageB >= 0.0 && data.voltageB < 10.0) || (data.voltageB < 50 && data.voltageB > 0)) cutPhases++;
        
        alertText.textContent = '⚠️ WARNING: ' + cutPhases + ' Phase(s) Cut Detected!';
        alertBanner.style.display = 'block';
        alertBanner.style.background = 'linear-gradient(135deg, #f39c12, #e67e22)';
    } else if ((data.voltageR < 100 && data.voltageR >= 50) || 
               (data.voltageY < 100 && data.voltageY >= 50) || 
               (data.voltageB < 100 && data.voltageB >= 50)) {
        alertText.textContent = '🔶 ALERT: Low Voltage Condition!';
        alertBanner.style.display = 'block';
        alertBanner.style.background = 'linear-gradient(135deg, #f1c40f, #f39c12)';
    } else {
        alertBanner.style.display = 'none';
    }
}
        // Phone number management
        function updatePhoneNumber(index) {
            const phoneInput = document.getElementById('phone' + (index + 1));
            const phoneNumber = phoneInput.value.trim();
            
            if (phoneNumber && phoneNumber.length >= 10) {
                fetch('/command?cmd=PHONE:' + index + ':' + encodeURIComponent(phoneNumber))
                    .then(response => response.text())
                    .then(data => {
                        showNotification('Phone number ' + (index + 1) + ' updated successfully!', 'success');
                        loadPhoneNumbers();
                    })
                    .catch(error => {
                        showNotification('Error updating phone number', 'error');
                    });
            } else {
                showNotification('Please enter a valid phone number', 'warning');
            }
        }

        function loadPhoneNumbers() {
            fetch('/phonenumbers')
                .then(response => response.json())
                .then(data => {
                    for (let i = 0; i < 5; i++) {
                        document.getElementById('phone' + (i + 1)).value = data.numbers[i] || '';
                    }
                    showNotification('Phone numbers loaded successfully!', 'success');
                })
                .catch(error => {
                    showNotification('Error loading phone numbers', 'error');
                });
        }

        function clearAllNumbers() {
            if (confirm('Are you sure you want to clear all phone numbers?')) {
                for (let i = 0; i < 5; i++) {
                    fetch('/command?cmd=PHONE:' + i + ':')
                        .then(response => response.text());
                }
                setTimeout(() => {
                    loadPhoneNumbers();
                    showNotification('All phone numbers cleared!', 'success');
                }, 1000);
            }
        }

        function sendCommand(command) {
            if (command === 'TEST_SMS') {
                lastSmsTime = new Date().toLocaleTimeString();
                document.getElementById('lastSmsTime').textContent = lastSmsTime;
            }
            
            fetch('/command?cmd=' + encodeURIComponent(command))
                .then(response => response.text())
                .then(data => {
                    showNotification('Command sent: ' + data, 'success');
                })
                .catch(error => {
                    showNotification('Error sending command', 'error');
                });
        }

        // Update system uptime
        function updateUptime() {
            const uptimeElement = document.getElementById('systemUptime');
            const now = new Date();
            const uptime = Math.floor((now - startTime) / 1000);
            const hours = Math.floor(uptime / 3600);
            const minutes = Math.floor((uptime % 3600) / 60);
            const seconds = uptime % 60;
            uptimeElement.textContent = 
                String(hours).padStart(2, '0') + ':' +
                String(minutes).padStart(2, '0') + ':' +
                String(seconds).padStart(2, '0');
        }

        // Fetch data every 2 seconds
        function fetchData() {
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    updateDisplay(data);
                })
                .catch(error => {
                    console.error('Error fetching data:', error);
                    document.getElementById('connectionStatus').className = 'connection-status disconnected';
                    document.getElementById('connectionStatus').innerHTML = 'Good';
                });
        }

        // Initialize everything when page loads
        const startTime = new Date();
        
        window.onload = function() {
            loadPhoneNumbers();
            setInterval(fetchData, 2000);
            setInterval(updateUptime, 1000);
            fetchData(); // Initial data load
        };
    </script>
</body>
</html>
)rawliteral";

void setup() {
    // Start serial for debugging
    Serial.begin(115200);
    
    // Start SoftwareSerial for Arduino communication
    arduinoSerial.begin(9600);
    
    // Start WiFi
    WiFi.begin(ssid, password);
    
    // Wait for WiFi connection
    Serial.println("Connecting to WiFi...");
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(1000);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        systemStatus = "Connected - " + WiFi.localIP().toString();
    } else {
        Serial.println("\nWiFi connection failed!");
        systemStatus = "WiFi Connection Failed";
    }
    
    // Setup web server routes
    server.on("/", handleRoot);
    server.on("/data", handleData);
    server.on("/command", handleCommand);
    server.on("/phonenumbers", handlePhoneNumbers);
    
    // Start web server
    server.begin();
    Serial.println("HTTP server started");
    
    // Send hello message to Arduino
    arduinoSerial.println("HELLO from NodeMCU");
    Serial.println("Sent hello to Arduino");
    
    systemStartTime = millis();
}

void loop() {
    server.handleClient();
    
    // Read data from Arduino
    if (arduinoSerial.available()) {
        String data = arduinoSerial.readStringUntil('\n');
        data.trim();
        
        if (data.startsWith("DATA:")) {
            // Parse data from Arduino
            parseData(data);
            dataReceived = true;
            lastUpdate = millis();
            connectionStatus = true;
        } else if (data.startsWith("HELLO from Arduino")) {
            Serial.println("Arduino responded to hello");
            connectionStatus = true;
        } else if (data.indexOf("SMS") >= 0) {
            // Capture SMS-related messages for alerts
            lastAlert = data;
            Serial.println("SMS Alert: " + data);
        }
    }
    
    // Check if data is stale (no updates for 15 seconds)
    if (dataReceived && (millis() - lastUpdate > 15000)) {
        dataReceived = false;
        connectionStatus = false;
        Serial.println("No data from Arduino for 15 seconds");
        systemStatus = "Waiting for Arduino data...";
    }
}

void handleRoot() {
    server.send(200, "text/html", webpage);
}

void handleData() {
    // Create JSON response with all system data
    StaticJsonDocument<512> doc;
    doc["voltageR"] = voltageR;
    doc["voltageY"] = voltageY;
    doc["voltageB"] = voltageB;
    doc["connected"] = connectionStatus;
    doc["systemStatus"] = systemStatus;
    doc["lastAlert"] = lastAlert;
    doc["uptime"] = (millis() - systemStartTime) / 1000;
    
    String json;
    serializeJson(doc, json);
    
    server.send(200, "application/json", json);
}

void handleCommand() {
    if (server.hasArg("cmd")) {
        String command = server.arg("cmd");
        arduinoSerial.println(command);
        Serial.println("Sent command to Arduino: " + command);
        
        if (command.startsWith("PHONE:")) {
            // Update phone number in local storage
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
        
        server.send(200, "text/plain", "Command sent to Arduino: " + command);
    } else {
        server.send(400, "text/plain", "No command specified");
    }
}

void handlePhoneNumbers() {
    StaticJsonDocument<512> doc;
    JsonArray numbers = doc.createNestedArray("numbers");
    
    for (int i = 0; i < 5; i++) {
        numbers.add(phoneNumbers[i]);
    }
    
    String json;
    serializeJson(doc, json);
    server.send(200, "application/json", json);
}

void parseData(String data) {
    // Expected format: "DATA:230.5,229.8,231.2"
    int dataStart = data.indexOf(":") + 1;
    String values = data.substring(dataStart);
    
    // Split by commas
    int firstComma = values.indexOf(",");
    int secondComma = values.indexOf(",", firstComma + 1);
    
    if (firstComma != -1 && secondComma != -1) {
        voltageR = values.substring(0, firstComma).toFloat();
        voltageY = values.substring(firstComma + 1, secondComma).toFloat();
        voltageB = values.substring(secondComma + 1).toFloat();
        
        // Update system status based on voltages - FIXED LOGIC
        if (voltageR < 50 || voltageY < 50 || voltageB < 50) {
            systemStatus = "Alert: Phase Cut Detected";
        } else if (voltageR < 100 || voltageY < 100 || voltageB < 100) {
            systemStatus = "Warning: Low Voltage";
        } else if (voltageR > 0 && voltageY > 0 && voltageB > 0) {
            systemStatus = "Normal Operation";
        } else {
            systemStatus = "No Data Received";
        }
        
        // Debug output
        Serial.print("Received from Arduino - R:");
        Serial.print(voltageR);
        Serial.print("V Y:");
        Serial.print(voltageY);
        Serial.print("V B:");
        Serial.println(voltageB);
    }
}