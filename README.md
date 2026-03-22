# SitLess

**An AI-Driven Sedentary Monitoring System with a 3-DOF Robotic Arm**

SitLess is an AIoT system that detects prolonged sitting using an ESP32 microcontroller, ultrasonic sensor, and a trained Random Forest classifier. When sitting is detected for too long, alerts escalate — from a buzzer beep (30s) to a physical robotic arm tap (60s). A Streamlit dashboard provides real-time monitoring and manual controls.

> Built as a second-year individual project for PDE2226 Systems Integration & Machine Learning — BEng (Honours) Computer Systems Engineering, Middlesex University Dubai.

---

<p align="center">
  <img src="docs/sitlessgif2.gif" alt="SitLess Demo" width="600">
</p>

<p align="center">
  <img src="docs/sitless%20full%20setup%20enclosure%20closed.jpeg" alt="Full Setup" width="500">
</p>

<p align="center">
  <img src="docs/sitless%20full%20setup%20enclosure%20open.jpeg" alt="Inside View" width="500">
</p>

<p align="center">
  <img src="docs/enclousre%20full%20setup%20open%20top%20down.jpeg" alt="Enclosure Top-Down" width="500">
</p>

---

## How It Works

```
HC-SR04 Sensor → ESP32 → Serial → Python Backend → Random Forest → Alert System
                                                                    ├── Buzzer (30s)
                                                                    └── Robotic Arm Tap (60s)
```

1. The **ESP32** reads ultrasonic distance every 200ms and sends it over serial
2. The **Python backend** collects readings into sliding windows (10 samples, step 5) and extracts features
3. A **Random Forest classifier** predicts the activity state: `active`, `sedentary`, or `not present`
4. If sedentary for too long, **escalating alerts** trigger — first a buzzer beep, then a physical tap from the 3-DOF arm
5. A **Streamlit dashboard** shows real-time state, distance, inactivity timer, and provides manual controls

---

## Machine Learning

| Detail | Value |
|---|---|
| Model | Random Forest (100 estimators) |
| Test Accuracy | **85.1%** |
| Dataset | 1,473 samples → 234 feature windows |
| Window Size | 10 samples (step 5) ≈ 2 seconds |
| Classes | Active, Sedentary, Not Present |

### Features Extracted Per Window

| Feature | Description | Importance |
|---|---|---|
| `rate_of_change` | Mean absolute difference between consecutive readings | **34.5%** |
| `std_distance` | Standard deviation of distances | **26.8%** |
| `range_distance` | Max − Min distance in window | **22.6%** |
| `mean_distance` | Average distance | 16.1% |

The top features (rate of change and standard deviation) capture **movement variability** — the key signal distinguishing active vs sedentary behaviour.

---

## Hardware

| Component | Purpose |
|---|---|
| ESP-32D | Microcontroller |
| HC-SR04 | Ultrasonic distance sensor |
| PCA9685 | 16-channel PWM servo driver |
| 3× MG996R | Servo motors (base, shoulder, elbow) |
| Piezo buzzer | Audio alert |
| 5V 3A DC supply | Power |
| 3D-printed PLA enclosure | Housing (FreeCAD files included) |

**Total hardware cost: approximately 200 AED (~$55 USD)**

---

## Repository Structure

```
SitLess/
├── Sitless_Final_Arduino_Code.ino   # ESP32 firmware (Arduino IDE)
├── Python & CSV & Graphs/
│   ├── live_prediction.ipynb        # ML backend — serial + prediction + alerts
│   ├── dashboard.py                 # Streamlit web dashboard
│   ├── data_collection.ipynb        # Raw data collection from sensor
│   ├── data_cleaning.ipynb          # Cleaning & preprocessing
│   ├── extract_features.ipynb       # Sliding window feature extraction
│   ├── model_training.ipynb         # Model training & evaluation
│   ├── *.csv                        # Datasets (raw + processed)
│   └── *.png                        # Output charts & visualisations
├── FreeCAD files/                   # CAD files for enclosure & robotic arm
└── README.md
```

---

## Getting Started

### Prerequisites

- Python 3.8+
- Arduino IDE
- ESP32 board package installed in Arduino IDE

### Install Dependencies

```bash
pip install pyserial pandas numpy scikit-learn joblib streamlit matplotlib
```

### 1. Flash the ESP32

Open `Sitless_Final_Arduino_Code.ino` in Arduino IDE and upload to your ESP32 (connected on COM4 by default).

### 2. Start the ML Backend

```bash
# Open Jupyter and run:
jupyter notebook "Python & CSV & Graphs/live_prediction.ipynb"
```

### 3. Launch the Dashboard

```bash
# In a separate terminal:
streamlit run "Python & CSV & Graphs/dashboard.py"
```

The backend and dashboard communicate via shared JSON files (`state.json` and `commands.json`).

---

## Alert Escalation Logic

```
Sedentary detected → Timer starts
                     ├── 0–30s:  Monitoring (no alert)
                     ├── 30s:    Buzzer beep
                     └── 60s:    Robotic arm tap
                     
Active for 5s → Timer resets
```

---

## Custom 3-DOF Robotic Arm

The robotic arm was designed from scratch in FreeCAD and 3D-printed in PLA. It has three degrees of freedom — base rotation, shoulder lift, and elbow extension — driven by three MG996R servo motors through a PCA9685 PWM driver.

The arm's tap motion was tuned manually using incremental stepping to find reliable angles:

| Joint | Rest Position | Tap Position |
|---|---|---|
| Base | 180° | 40° |
| Shoulder | 90° | 150° |
| Elbow | 90° | 40° |

On a tap command, the servos move sequentially — base rotates, shoulder raises, elbow extends — then retract in reverse order. The motion is deliberate enough to get your attention without being aggressive.

The `FreeCAD files/` folder contains all the CAD models for both the arm and the sensor enclosure (which holds the HC-SR04 at the front with the ESP32 mounted inside).

---

## Future Improvements

- Multi-user data collection for better generalisation
- Bluetooth/Wi-Fi to replace USB serial
- Inverse kinematics to replace the current predefined angle movements with dynamic positioning
- Historical data logging and wellness trend tracking
- Additional sensors (PIR, second ultrasonic) for better accuracy
- On-device inference for privacy

---

## Built With

- **Hardware:** ESP32, HC-SR04, PCA9685, MG996R servos
- **ML:** scikit-learn (Random Forest), pandas, NumPy
- **Dashboard:** Streamlit
- **CAD:** FreeCAD
- **Firmware:** Arduino (C++)

---

## License

This project was developed for academic purposes. Feel free to use it as reference or inspiration for your own projects.

---

## Author

**Saif Ali Syed** — BEng (Honours) Computer Systems Engineering, Middlesex University Dubai  
[LinkedIn](https://www.linkedin.com/in/saif-ali-syed-a87aa534a/) <!-- Replace with your actual LinkedIn URL -->
