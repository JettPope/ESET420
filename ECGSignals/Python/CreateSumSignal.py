import numpy as np
import scipy.signal
import matplotlib.pyplot as plt

# Parameters
file_name_mother = r"C:\PC\Documents\School\25 Spring (A WHOOP)\ESET 420\Code\ESET420\ECGSignals\OriginalECGRacapMom.dat"
file_name_baby = r"C:\PC\Documents\School\25 Spring (A WHOOP)\ESET 420\Code\ESET420\ECGSignals\BabyECGRacap.dat"
new_file_name = r"C:\PC\Documents\School\25 Spring (A WHOOP)\ESET 420\Code\ESET420\ECGSignals\SumECGRacapMomPlusBaby.dat"

scan_rate = 1000  # Hz
duration_mother = 550  # seconds
num_channels = 5

# Read mother's ECG
samples_to_read = scan_rate * duration_mother
data_type = np.float64
bytes_per_sample = np.dtype(data_type).itemsize
bytes_to_read = samples_to_read * num_channels * bytes_per_sample

with open(file_name_mother, "rb") as f:
    data_mother = np.frombuffer(f.read(bytes_to_read), dtype=data_type).reshape(-1, num_channels)

ecg_mother = data_mother[:, 1]  # Extract mother's ECG

# --- 1. Estimate the mother's heart rate ---
peaks, _ = scipy.signal.find_peaks(ecg_mother, distance=scan_rate * 0.5)  # Minimum 0.5s between beats
peak_intervals = np.diff(peaks) / scan_rate  # Time intervals between peaks (in seconds)
mother_hr = 60 / np.mean(peak_intervals)  # Convert to BPM

# Ensure the mother's heart rate is within range
if not (60 <= mother_hr <= 135):
    raise ValueError(f"Mother's heart rate ({mother_hr:.2f} BPM) is outside expected range!")

print(f"Mother's estimated heart rate: {mother_hr:.2f} BPM")

# --- 2. Generate Baby's ECG with a Higher HR ---
baby_hr = mother_hr * 1.3  # Baby's HR is 30% higher than mother's
resample_factor = mother_hr / baby_hr  # Factor to adjust baby's HR

# Reduce amplitude and resample
ecg_baby = ecg_mother / 10  # Reduce amplitude
new_length = int(len(ecg_mother) * resample_factor)
ecg_baby = scipy.signal.resample(ecg_baby, new_length)

# Shift the baby’s ECG signal in time
time_offset = 0.5  # Shift baby’s ECG by 0.5 seconds
sample_offset = int(time_offset * scan_rate)
ecg_baby = np.roll(ecg_baby, sample_offset)

# Match lengths (pad or trim)
if len(ecg_baby) < len(ecg_mother):
    ecg_baby = np.pad(ecg_baby, (0, len(ecg_mother) - len(ecg_baby)), 'wrap')
else:
    ecg_baby = ecg_baby[:len(ecg_mother)]

print(f"Baby's estimated heart rate: {baby_hr:.2f} BPM")

# --- 3. Compute Summed ECG Signal ---
ecg_summed = ecg_mother + ecg_baby

# Create a new data array with summed ECG
new_data = np.copy(data_mother)
new_data[:, 1] = ecg_summed  # Replace ECG channel with summed ECG

# Save the summed ECG to a new file
with open(new_file_name, "wb") as f:
    f.write(new_data.tobytes())

print(f"Summed ECG file saved to: {new_file_name}")

# --- 4. Plot All Three Signals ---
time = np.arange(len(ecg_mother)) / scan_rate

plt.figure(figsize=(12, 6))
plt.plot(time, ecg_mother, label=f"Mother ECG ({mother_hr:.2f} BPM)", color="red", alpha=0.7)
plt.plot(time, ecg_baby, label=f"Baby ECG ({baby_hr:.2f} BPM)", color="blue", alpha=0.7)
plt.plot(time, ecg_summed, label="Summed ECG", color="purple", linestyle="dashed")

plt.title("Mother, Baby, and Summed ECG Signals")
plt.xlabel("Time (s)")
plt.ylabel("Amplitude")
plt.legend()
plt.grid(True)
plt.xlim(0, 10)  # Show only the first 10 seconds

plt.show()
