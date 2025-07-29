#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>

#define RIGHT 5
#define LEFT 18

#define FRONT 4
#define REAR 16
#define SPEED 17

#define PWM_SPEED_CHANNEL 0
#define PWM_STEER_CHANNEL 1

#define PWM_FREQ 500
#define PWM_RESOLUTION 8

// Replace with your network credentials
const char* ssid = "SSID";
const char* password = "PASSWORD";

WebServer server(80);

bool ledState = false;

String readFile(const char* path) {
    File file = SPIFFS.open(path, "r");
    if (!file) {
        Serial.println("Failed to open file for reading");
        return "";
    }
    
    String content = file.readString();
    file.close();
    return content;
}

double getJsonValue(const String& json, const String& key) {
    String searchKey = "\"" + key + "\":";
    int keyStart = json.indexOf(searchKey);
    
    if (keyStart == -1) {
        Serial.println("Key '" + key + "' not found in JSON");
        return NAN; // Return NaN if key not found
    }
    
    int valueStart = keyStart + searchKey.length();
    
    // Skip whitespace
    while (valueStart < json.length() && (json.charAt(valueStart) == ' ' || json.charAt(valueStart) == '\t')) {
        valueStart++;
    }
    
    // Find the end of the value
    int valueEnd = valueStart;
    bool inString = false;
    
    // Handle string values (skip quotes)
    if (json.charAt(valueStart) == '"') {
        inString = true;
        valueStart++; // Skip opening quote
        valueEnd = json.indexOf('"', valueStart); // Find closing quote
    } else {
        // Handle numeric values
        while (valueEnd < json.length()) {
            char c = json.charAt(valueEnd);
            if (c == ',' || c == '}' || c == ']' || c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                break;
            }
            valueEnd++;
        }
    }
    
    if (valueEnd == -1) {
        Serial.println("Invalid JSON format for key '" + key + "'");
        return NAN;
    }
    
    String valueStr = json.substring(valueStart, valueEnd);
    valueStr.trim();
    
    Serial.println("Extracted value for '" + key + "': " + valueStr);
    
    return valueStr.toDouble();
}

bool hasJsonKey(const String& json, const String& key) {
    String searchKey = "\"" + key + "\":";
    return json.indexOf(searchKey) != -1;
}

void handleRoot() {
    String html = readFile("/index.html");
    if (html.length() > 0) {
        server.send(200, "text/html", html);
    } else {
        server.send(500, "text/plain", "Failed to read HTML file");
    }
}

void handleControls() {
    if (server.method() == HTTP_POST) {
        String body = server.arg("plain");
        Serial.println("Received POST data: " + body);

        double forwardValue = 0, rearValue = 0, steeringValue = 0;
        
        // Get the forward value from JSON
        if (hasJsonKey(body, "forward")) {
            forwardValue = getJsonValue(body, "forward");
            if (isnan(forwardValue)) {
                forwardValue = 0; // Default to 0 if invalid
            }
        } else {
            server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing forward key\"}");
            return;
        }

        // Get the rear value from JSON
        if (hasJsonKey(body, "backward")) {
            rearValue = getJsonValue(body, "backward");
            if (isnan(rearValue)) {
                rearValue = 0; // Default to 0 if invalid
            }
        } else {
            server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing backward key\"}");
            return;
        }

        // Get the steering value from JSON
        if (hasJsonKey(body, "steering")) {
            steeringValue = getJsonValue(body, "steering");
            if (isnan(steeringValue)) {
                steeringValue = 0; // Default to 0 if invalid
            }
            Serial.println("Steering value: " + String(steeringValue));
        } else {
            server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing steering key\"}");
            return;
        }

        // Calculate the steering direction
        bool isSteeringLeft = steeringValue < -0.5;
        bool isSteeringRight = steeringValue > 0.5;

        digitalWrite(RIGHT, isSteeringRight ? HIGH : LOW);
        digitalWrite(LEFT, isSteeringLeft ? HIGH : LOW);

        // Calculate the movement direction and speed
        double speed = forwardValue - rearValue;
        bool isForward = speed > 0.1;
        bool isBackward = speed < -0.1;

        digitalWrite(FRONT, isForward ? HIGH : LOW);
        digitalWrite(REAR, isBackward ? HIGH : LOW);

        speed = abs(speed);

        ledcWrite(PWM_SPEED_CHANNEL, (int)(speed * 255));

        server.send(200, "application/json", "{\"status\":\"success\"}");
    } else {
        server.send(405, "text/plain", "Method Not Allowed");
    }
}

void handleStatus() {
    server.send(200, "text/plain", ledState ? "ON" : "OFF");
}

void handleGPIOData() {
    // Read current GPIO states and PWM values
    int frontState = digitalRead(FRONT);
    int rearState = digitalRead(REAR);

    int rightState = digitalRead(RIGHT);
    int leftState = digitalRead(LEFT);
    
    // Get PWM duty cycle (0-255 range, convert to percentage)
    int speedDutyCycle = ledcRead(PWM_SPEED_CHANNEL);
    float speedPercentage = (speedDutyCycle / 255.0) * 100.0;
    
    // Create JSON response
    String jsonResponse = "{"
        "\"pins\": {"
            "\"FRONT\": {"
                "\"pin\": " + String(FRONT) + ","
                "\"state\": \"" + String(frontState ? "HIGH" : "LOW") + "\""
            "},"
            "\"REAR\": {"
                "\"pin\": " + String(REAR) + ","
                "\"state\": \"" + String(rearState ? "HIGH" : "LOW") + "\""
            "},"
            "\"SPEED\": {"
                "\"pin\": " + String(SPEED) + ","
                "\"state\": \"PWM\","
                "\"duty_cycle\": " + String(speedPercentage, 1) +
            "},"
            "\"RIGHT\": {"
                "\"pin\": " + String(RIGHT) + ","
                "\"state\": \"" + String(rightState ? "HIGH" : "LOW") + "\""
            "},"
            "\"LEFT\": {"
                "\"pin\": " + String(LEFT) + ","
                "\"state\": \"" + String(leftState ? "HIGH" : "LOW") + "\""
            "}"
        "}"
    "}";
    
    server.send(200, "application/json", jsonResponse);
}

void setup() {
    Serial.begin(115200);

    ledcSetup(PWM_SPEED_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(SPEED, PWM_SPEED_CHANNEL);

    pinMode(FRONT, OUTPUT);
    pinMode(REAR, OUTPUT);

    pinMode(RIGHT, OUTPUT);
    pinMode(LEFT, OUTPUT);
    
    // Initialize SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }
    
    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    
    Serial.println();
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    // Define web server routes
    server.on("/", handleRoot);
    server.on("/controls", HTTP_POST, handleControls);
    server.on("/status", handleStatus);
    server.on("/gpiodata", handleGPIOData);
    
    // Start server
    server.begin();
    Serial.println("Web server started!");
    Serial.print("Open http://");
    Serial.print(WiFi.localIP());
    Serial.println(" in your browser");
}

void loop() {
    server.handleClient();
}