// ===========================================================================
// main.ino — Sleep Apnea Detection System (ESP8266 + MAX30102)
// ===========================================================================
// Real-time SpO2 and heart-rate monitoring with on-device ML inference.
// Hosts a Wi-Fi Access Point and serves live JSON data to a web dashboard.
// Triggers a buzzer alert when the risk score exceeds safe thresholds.
// ===========================================================================

// ---------------------------------------------------------------------------
// Libraries
// ---------------------------------------------------------------------------
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include "model.h"

// ---------------------------------------------------------------------------
// Wi-Fi Access Point Configuration
// ---------------------------------------------------------------------------
const char* AP_SSID = "Apnea_Monitor";
const char* AP_PASS = "12345678";

// ---------------------------------------------------------------------------
// Peripheral Instances
// ---------------------------------------------------------------------------
ESP8266WebServer server(80);
MAX30105 particleSensor;
Eloquent::ML::Port::LinearRegression ml_model;

// ---------------------------------------------------------------------------
// Pin Definitions & Buffer Settings
// ---------------------------------------------------------------------------
#define BUZZER_PIN D8
#define BUFFER_SIZE 100

// ---------------------------------------------------------------------------
// Sensor Data Buffers
// ---------------------------------------------------------------------------
uint32_t irBuffer[BUFFER_SIZE];
uint32_t redBuffer[BUFFER_SIZE];
int bufferIndex = 0, samplesCollected = 0;

// ---------------------------------------------------------------------------
// Algorithm Outputs
// ---------------------------------------------------------------------------
int32_t spo2, heartRate;
int8_t validSPO2, validHeartRate;

// ---------------------------------------------------------------------------
// Running Averages & State
// ---------------------------------------------------------------------------
float avgBPM = 0, avgSPO2 = 0, previousSPO2 = 98.0;
float sumBPM = 0, sumSPO2 = 0;
int readingsInWindow = 0, alertStatus = 0;
unsigned long lastReadTime = 0, fiveSecTimer = 0;

// ===========================================================================
// JSON Endpoint Handler
// ===========================================================================
void handleData() {
    String json = "{\"bpm\":" + String(avgBPM) + ",\"spo2\":" +
        String(avgSPO2) + ",\"status\":" + String(alertStatus) + "}";
    server.send(200, "application/json", json);
}

// ===========================================================================
// Setup
// ===========================================================================
void setup() {
    Serial.begin(115200);
    pinMode(BUZZER_PIN, OUTPUT);
    Wire.begin(D2, D1); // I2C Pins

    WiFi.softAP(AP_SSID, AP_PASS); // Start Access Point

    if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
        while (1) yield(); // Prevent Watchdog Crash if sensor fails
    }
    particleSensor.setup(0x1F, 4, 2, 400, 215, 4096);

    server.on("/data", handleData);
    server.begin();
    fiveSecTimer = millis();
}

// ===========================================================================
// Main Loop
// ===========================================================================
void loop() {
    yield(); // Feed the Watchdog Timer
    server.handleClient();

    // ----- 1. Data Collection -----
    if (millis() - lastReadTime >= 10) {
        lastReadTime = millis();
        particleSensor.check();
        if (particleSensor.available()) {
            redBuffer[bufferIndex] = particleSensor.getRed();
            irBuffer[bufferIndex] = particleSensor.getIR();
            bufferIndex = (bufferIndex + 1) % BUFFER_SIZE;
            samplesCollected++;
            particleSensor.nextSample();
        }
    }

    // ----- 2. Algorithm Processing -----
    if (samplesCollected >= BUFFER_SIZE) {
        yield();
        maxim_heart_rate_and_oxygen_saturation(irBuffer, BUFFER_SIZE,
            redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
        yield();
        if (validSPO2 && validHeartRate && spo2 > 50) {
            sumBPM += heartRate;
            sumSPO2 += spo2;
            readingsInWindow++;
        }
        samplesCollected = 0;
    }

    // ----- 3. Machine Learning Inference & Alerting (Every 5 seconds) -----
    if (millis() - fiveSecTimer >= 5000) {
        if (readingsInWindow > 0) {
            avgBPM = sumBPM / readingsInWindow;
            avgSPO2 = sumSPO2 / readingsInWindow;
            float spo2Drop = previousSPO2 - avgSPO2;
            previousSPO2 = avgSPO2;

            float input[3] = { (float)avgBPM, (float)avgSPO2, spo2Drop };
            yield();
            float risk_score = ml_model.predict(input); // Edge AI Evaluation
            yield();

            // Threshold Logic
            if (risk_score >= 75.0) alertStatus = 1;
            else if (avgBPM < 50) alertStatus = 2;
            else if (avgSPO2 < 85) alertStatus = 3;
            else alertStatus = 0;

            digitalWrite(BUZZER_PIN, (alertStatus > 0) ? HIGH : LOW);
        }
        sumBPM = 0; sumSPO2 = 0; readingsInWindow = 0;
        fiveSecTimer = millis();
    }
}