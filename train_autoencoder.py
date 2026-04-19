import pandas as pd
import numpy as np
import tensorflow as tf
from tensorflow.keras import layers, models
from sklearn.preprocessing import MinMaxScaler
import os

# 1. Load the Baseline CSV Data
print("Loading data...")
df = pd.read_csv('normal_motor_baseline.csv')

# Drop any rows with NaN values just in case
df = df.dropna()

# Extract the X, Y, Z columns
data = df[['Accel_X', 'Accel_Y', 'Accel_Z']].values

# 2. Normalize the Data
# Neural networks hate large integer variations. We scale everything between 0 and 1.
scaler = MinMaxScaler()
data_scaled = scaler.fit_transform(data)

print("\n--- NORMALIZATION VALUES FOR C++ ---")
print("You will need these arrays for your C++ Inference Thread to normalize live data:")
print(f"float min_vals[3] = {{{scaler.data_min_[0]:.2f}, {scaler.data_min_[1]:.2f}, {scaler.data_min_[2]:.2f}}};")
print(f"float scale_vals[3] = {{{scaler.scale_[0]:.6f}, {scaler.scale_[1]:.6f}, {scaler.scale_[2]:.6f}}};")
print("------------------------------------\n")

# 3. Build the Autoencoder Neural Network
# Input(3) -> Encode(8) -> Bottleneck(2) -> Decode(8) -> Output(3)
input_dim = data_scaled.shape[1]
model = models.Sequential([
    layers.Dense(8, activation='relu', input_shape=(input_dim,)),
    layers.Dense(2, activation='relu'),
    layers.Dense(8, activation='relu'),
    layers.Dense(input_dim, activation='linear')
])

model.compile(optimizer='adam', loss='mse')

# 4. Train the Model
print("Training Autoencoder...")
# We use the scaled data as BOTH the input and the target output
model.fit(data_scaled, data_scaled, epochs=20, batch_size=32, validation_split=0.1)

# 5. Calculate the Anomaly Threshold
print("\nCalculating MSE Threshold...")
predictions = model.predict(data_scaled)
# Calculate Mean Squared Error across the features
mse = np.mean(np.power(data_scaled - predictions, 2), axis=1)

# Set the threshold slightly higher than the maximum error seen during normal operation
max_normal_mse = np.max(mse)
anomaly_threshold = max_normal_mse * 1.5 

print("\n=== DEFINITION OF DONE REQUIREMENT ===")
print(f"Maximum Normal MSE: {max_normal_mse:.6f}")
print(f"Recommended Anomaly Threshold: {anomaly_threshold:.6f}")
print("======================================\n")

# 6. Convert to TensorFlow Lite Flatbuffer
print("Converting to TFLite flatbuffer...")
converter = tf.lite.TFLiteConverter.from_keras_model(model)
tflite_model = converter.convert()

with open('anomaly_model.tflite', 'wb') as f:
    f.write(tflite_model)

print("Success! anomaly_model.tflite has been generated.")
