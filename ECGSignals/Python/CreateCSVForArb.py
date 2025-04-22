import numpy as np

# Parameters from the .hdr file
file_name = r"C:\PC\Documents\School\25 Spring (A WHOOP)\ESET 420\Code\ESET420\ECGSignals\OriginalECGRacapMom.dat" # change to reflect path to raw data file
scan_rate = 1000  # Hz
channels = ["Output Filtered", "ECG", "D/A Output", "Digital Output 4", "Digital Output 6"]
num_channels = len(channels) # 5 channels (length of channels array)

# channels[1] is ECG channel and is the one we need

data_type = np.float64 # DATATYPE FLOAT64 DO NOT CHANGE

with open(file_name, "rb") as f:
    data = np.frombuffer(f.read(), dtype=data_type)

# We need to count the total number of points and format the points according to the Arb's expected format and export to a CSV
# It should be something like (point number, channels[1]) for each point