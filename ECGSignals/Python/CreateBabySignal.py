import numpy as np
import scipy.signal
import matplotlib.pyplot as plt

# Parameters
file_name = r"C:\PC\Documents\School\24Fall\ESET 419\ECG Signals\tam_022503_racap.dat"
new_file_name = r"C:\PC\Documents\School\24Fall\ESET 419\ECG Signals\BabyECG.dat"
scan_rate = 1000  # Hz
channels = ["Output Filtered", "ECG", "D/A Output", "Digital Output 4", "Digital Output 6"]
num_channels = len(channels)

# Define the duration to read
duration = 550  # seconds
samples_to_read = scan_rate * duration

data_type = np.float64  # DATATYPE FLOAT64
bytes_per_sample = np.dtype(data_type).itemsize
bytes_to_read = samples_to_read * num_channels * bytes_per_sample

# Read the original data
with open(file_name, "rb") as f:
    data = np.frombuffer(f.read(bytes_to_read), dtype=data_type)

# Reshape data into 2D (time, channels)
data = data.reshape(-1, num_channels)

# Extract ECG channel
ecg_data = data[:, 1]

# Modify ECG signal
ecg_data_scaled = ecg_data / 16.845  # Reduce amplitude by 16.845x

# Resample ECG to increase heart rate by 1.5786x
new_length = int(len(ecg_data_scaled) / 1.5786)
ecg_data_resampled = scipy.signal.resample(ecg_data_scaled, new_length)

# Time shift parameters
time_offset = 0.5  # Shift by 0.5 seconds (adjust as needed)
sample_offset = int(time_offset * scan_rate)  # Convert to sample count

# Circularly shift ECG data in time
ecg_data_shifted = np.roll(ecg_data_resampled, sample_offset)


# Pad or trim other channels to match new length
new_data = np.zeros((new_length, num_channels), dtype=data_type)
for i in range(num_channels):
    if i == 1:
        new_data[:, i] = ecg_data_shifted  # Replace ECG channel
    else:
        new_data[:, i] = np.interp(
            np.linspace(0, len(data) - 1, new_length),
            np.arange(len(data)),
            data[:, i]
        )  # Resample other channels

# Save the modified data to a new binary file
with open(new_file_name, "wb") as f:
    f.write(new_data.tobytes())

print(f"Modified ECG file saved to: {new_file_name}")

# Plot the modified ECG signal
time_new = np.arange(new_length) / scan_rate
plt.figure(figsize=(10, 6))
plt.plot(time_new, new_data[:, 1], label="Modified ECG", color="blue")
plt.title("Baby ECG Signal")
plt.xlabel("Time (s)")
plt.ylabel("Amplitude")
plt.legend()
plt.grid(True)
plt.xlim(0, 10)
plt.show()
