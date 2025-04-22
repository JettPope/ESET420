import numpy as np
import matplotlib.pyplot as plt

# Parameters from the .hdr file
file_name = r"C:\PC\Documents\School\25 Spring (A WHOOP)\ESET 420\Code\ESET420\ECGSignals\OriginalECGRacapMom.dat" # change to reflect path to raw data file
scan_rate = 1000  # Hz
channels = ["Output Filtered", "ECG", "D/A Output", "Digital Output 4", "Digital Output 6"]
num_channels = len(channels) # 5 channels (length of channels array)

# Define the duration to read and calculate the number of samples
duration = 550  # seconds
samples_to_read = scan_rate * duration

data_type = np.float64 # DATATYPE FLOAT64 DO NOT CHANGE
bytes_per_sample = np.dtype(data_type).itemsize
bytes_to_read = samples_to_read * num_channels * bytes_per_sample

with open(file_name, "rb") as f:
    data = np.frombuffer(f.read(bytes_to_read), dtype=data_type)

# Reshape data into a 2D array (rows: time, columns: channels)
data = data.reshape(-1, num_channels)

# Extract the ECG channel (index 1 based on the header file)
ecg_data = data[:, 1]

# Create a time axis
time = np.arange(ecg_data.size) / scan_rate

# Plot the ECG data
plt.figure(figsize=(10, 6))
plt.plot(time, ecg_data, label="ECG", color="red")
plt.title("ECG Signal")
plt.xlabel("Time (s)")
plt.ylabel("Amplitude")
plt.legend()
plt.grid(True)
plt.xlim(0, 10)
plt.show()
