# ============================================================================
# synthetic.py — Synthetic Data Generator for Sleep Apnea Detection
# ============================================================================
# Generates a CSV dataset of simulated physiological signals (SpO2, BPM,
# SpO2 drop) with a derived continuous risk score for regression training.
# ============================================================================

import pandas as pd
import numpy as np


def generate_apnea_csv(filename="sleep_apnea_regression_data.csv", samples=10000):
    """
    Generate synthetic sleep apnea regression data and save to CSV.

    Parameters
    ----------
    filename : str
        Output CSV file path.
    samples : int
        Number of data rows to generate.
    """
    data = []
    print(f"Generating {samples} rows of continuous physiological data...")

    for _ in range(samples):
        # --- Randomised physiological readings ---
        spo2 = np.random.uniform(75, 100)
        bpm = np.random.uniform(50, 120)
        spo2_drop = np.random.uniform(0, 10)

        # Calculate a continuous "Risk Score" (0-100) based on medical heuristics
        risk_score = (100 - spo2) * 1.5 + (bpm - 60) * 0.2 + (spo2_drop) * 2.0

        # Add normal distribution noise to prevent overfitting
        noise = np.random.normal(0, 3)
        risk_score += noise
        risk_score = max(0.0, min(risk_score, 100.0))

        data.append([round(bpm, 2), round(spo2, 2), round(spo2_drop, 2), round(risk_score, 2)])

    # --- Build DataFrame and export ---
    df = pd.DataFrame(data, columns=['bpm', 'spo2', 'spo2_drop', 'risk_score'])
    df.to_csv(filename, index=False)
    print(f"Success! File saved as: {filename}")


# --- Entry point ---
generate_apnea_csv()