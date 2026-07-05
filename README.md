# 🫁 Sleep Apnea Detection System

A real-time, wearable sleep apnea monitoring system built on the **ESP8266** microcontroller and **MAX30102** pulse oximeter sensor. It uses an on-device **Linear Regression** model (Edge AI) to predict a continuous apnea risk score from live SpO2, heart-rate, and SpO2-drop readings — no cloud connection required.

---

## ✨ Features

| Feature | Description |
|---|---|
| **Real-Time Monitoring** | Continuously reads SpO2 and BPM from the MAX30102 sensor every 10 ms |
| **Edge AI Inference** | Runs a trained Linear Regression model directly on the ESP8266 |
| **Wi-Fi Dashboard** | Creates a local Access Point and serves live JSON data at `/data` |
| **Buzzer Alerts** | Triggers an audible alarm when the risk score exceeds safe thresholds |
| **Synthetic Data Pipeline** | Includes scripts to generate training data and export models for MCU deployment |

---

## 📁 Project Structure

```
sleep-apnea-detection-system/
├── main.ino         # ESP8266 firmware — sensor reading, ML inference, web server
├── model.h          # Exported Linear Regression weights (C++ header)
├── synthetic.py     # Generates synthetic physiological data (CSV)
├── train.py         # Trains the model and exports it via micromlgen
└── README.md
```

---

## 🔧 Hardware Requirements

| Component | Purpose |
|---|---|
| **ESP8266** (NodeMCU / Wemos D1 Mini) | Wi-Fi-enabled microcontroller |
| **MAX30102** Pulse Oximeter Sensor | Measures SpO2 and heart rate via IR/Red LEDs |
| **Buzzer** | Audible alert on apnea detection (connected to pin **D8**) |
| **Jumper Wires & Breadboard** | Prototyping connections |

### Wiring

| MAX30102 Pin | ESP8266 Pin |
|---|---|
| SDA | D2 |
| SCL | D1 |
| VCC | 3.3 V |
| GND | GND |

| Buzzer Pin | ESP8266 Pin |
|---|---|
| + | D8 |
| − | GND |

---

## 🛠️ Software Prerequisites

### Arduino IDE (Firmware)

- [Arduino IDE](https://www.arduino.cc/en/software) with **ESP8266 Board Package** installed
- Required Arduino libraries:
  - `ESP8266WiFi`
  - `ESP8266WebServer`
  - `Wire`
  - [SparkFun MAX30105](https://github.com/sparkfun/SparkFun_MAX3010x_Sensor_Library) (includes `MAX30105.h` and `spo2_algorithm.h`)

### Python (Model Training)

- Python 3.8+
- Required packages:
  ```
  pandas
  numpy
  scikit-learn
  micromlgen
  ```

---

## 🚀 Getting Started

### 1. Generate Synthetic Training Data

```bash
python synthetic.py
```

This creates `sleep_apnea_regression_data.csv` with **10,000** rows of simulated physiological readings and corresponding risk scores.

### 2. Train the Model & Export

```bash
python train.py
```

This trains a `LinearRegression` model on the CSV data and exports the learned weights to `model.h` — a C++ header file ready for the ESP8266.

### 3. Flash the Firmware

1. Open `main.ino` in the **Arduino IDE**.
2. Select your ESP8266 board under **Tools → Board**.
3. Click **Upload**.
4. The device will create a Wi-Fi Access Point named **`Apnea_Monitor`** (password: `12345678`).

### 4. View Live Data

Connect to the `Apnea_Monitor` Wi-Fi network, then open a browser and navigate to:

```
http://192.168.4.1/data
```

The endpoint returns a JSON response:

```json
{
  "bpm": 72.5,
  "spo2": 96.3,
  "status": 0
}
```

| `status` | Meaning |
|---|---|
| `0` | Normal — no alert |
| `1` | High risk score (≥ 75) |
| `2` | Bradycardia (BPM < 50) |
| `3` | Severe hypoxemia (SpO2 < 85) |

---

## 🧠 How It Works

```
MAX30102 Sensor
       │
       ▼
┌──────────────┐     ┌──────────────────┐     ┌───────────────┐
│ Data Collect  │────▶│  SpO2 Algorithm  │────▶│  ML Inference │
│ (10 ms loop)  │     │  (Maxim Library)  │     │  (every 5 s)  │
└──────────────┘     └──────────────────┘     └───────┬───────┘
                                                      │
                                           ┌──────────┴──────────┐
                                           │                     │
                                      ┌────▼────┐         ┌─────▼─────┐
                                      │ Buzzer  │         │ JSON API  │
                                      │ Alert   │         │  /data    │
                                      └─────────┘         └───────────┘
```

1. **Data Collection** — The MAX30102 sensor is sampled every 10 ms; IR and Red LED readings fill a 100-sample ring buffer.
2. **SpO2 Algorithm** — Once the buffer is full, Maxim's `spo2_algorithm` computes heart rate and SpO2.
3. **ML Inference** — Every 5 seconds, averaged BPM, SpO2, and SpO2-drop are fed into the on-device Linear Regression model to produce a continuous risk score (0–100).
4. **Alerting** — If the risk score exceeds 75, BPM drops below 50, or SpO2 falls below 85, the buzzer sounds and the alert status is updated.

---

## 📊 Risk Score Formula

The synthetic data generator uses the following medical heuristic to derive the risk score:

```
risk_score = (100 − SpO2) × 1.5 + (BPM − 60) × 0.2 + SpO2_drop × 2.0 + noise
```

The trained model learns to approximate this relationship from the data, with Gaussian noise (σ = 3) added during generation to prevent overfitting.

---

## ⚠️ Disclaimer

> This project is built for **educational and prototyping purposes only**. It is **not** a certified medical device and should **not** be used for clinical diagnosis or treatment decisions.

---

## 📄 License

This project is open source. Feel free to use, modify, and distribute.