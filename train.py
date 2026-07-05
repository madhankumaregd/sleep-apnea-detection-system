# ============================================================================
# train.py — Model Training & Export for Sleep Apnea Detection
# ============================================================================
# Trains a Linear Regression model on the synthetic CSV data and exports
# the learned weights as a C++ header file (model.h) using micromlgen,
# ready for deployment on an ESP8266 microcontroller.
# ============================================================================

import pandas as pd
from sklearn.linear_model import LinearRegression
from micromlgen import port

# --- Load dataset ---
print("Loading data...")
df = pd.read_csv('sleep_apnea_regression_data.csv')

# Define Features (X) and Target (y)
X = df[['bpm', 'spo2', 'spo2_drop']]
y = df['risk_score']

# --- Train model ---
print("Training model...")
regressor = LinearRegression()
regressor.fit(X, y)

# --- Export to C++ header for on-device inference ---
print("Exporting to C++ header file...")
with open('model.h', 'w') as f:
    f.write(port(regressor))
print("SUCCESS! Model exported as model.h!")