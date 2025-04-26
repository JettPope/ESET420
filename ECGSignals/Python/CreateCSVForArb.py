import numpy as np

new_file_name = r"C:\Users\bjdpc\Downloads\ESET420-main\ESET420-main\ECGSignals\ECGSum.csv"

# Parameters from the .hdr file
file_name = r"C:\Users\bjdpc\Downloads\ESET420-main\ESET420-main\ECGSignals\SumECGRacapMomPlusBaby.dat" # change to reflect path to raw data file
scan_rate = 1000  # Hz
channels = ["Output Filtered", "ECG", "D/A Output", "Digital Output 4", "Digital Output 6"]
num_channels = len(channels) # 5 channels (length of channels array)

# Define the duration to read and calculate the number of samples
duration = 1  # seconds
samples_to_read = scan_rate * duration

data_type = np.float64 # DATATYPE FLOAT64 DO NOT CHANGE
bytes_per_sample = np.dtype(data_type).itemsize
bytes_to_read = samples_to_read * num_channels * bytes_per_sample

with open(file_name, "rb") as f:
    data = np.frombuffer(f.read(bytes_to_read), dtype=data_type)

# We need to count the total number of points and format the points according to the Arb's expected format and export to a CSV
# It should be something like (point number, channels[1]) for each point
dataR = data.reshape(-1,num_channels)

ecg_data = dataR[:, 1]

pointNum = ecg_data.size

print(pointNum)
i = 0
with open(new_file_name, "w") as f:
    f.write(f"data length,{pointNum}\nfrequency,0.500000\namp,1.250000\noffset,0.000000\nphase,0.000000\n\n\n\n\n\n\n\nxpos,value")
    for i in range(pointNum):
        f.write(f"{i},{ecg_data[i]}\n")

print("Done")
