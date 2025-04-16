import asyncio
import struct
import numpy as np
import matplotlib.pyplot as plt
from bleak import BleakScanner, BleakClient

# UUIDs
UUID_MOTHER_ECG = "00002b18-0000-1000-8000-00805f9b34fb"
UUID_SUM_ECG = "00002b19-0000-1000-8000-00805f9b34fb"
UUID_TEMP = "00002a6e-0000-1000-8000-00805f9b34fb"

CHARACTERISTIC_UUIDS = [UUID_MOTHER_ECG, UUID_SUM_ECG, UUID_TEMP]

# Plotting parameters
BUFFER_SIZE = 200  # For 10 seconds at 20 Hz
SAMPLE_INTERVAL = 0.05  # Seconds between samples (20 Hz)

# Buffers
mother_ecg = np.zeros(BUFFER_SIZE)
baby_ecg = np.zeros(BUFFER_SIZE)
time_axis = np.linspace(-BUFFER_SIZE * SAMPLE_INTERVAL, 0, BUFFER_SIZE)  # e.g., -10 to 0 seconds

# Matplotlib setup
plt.ion()
fig, (ax_mother, ax_baby) = plt.subplots(2, 1, figsize=(10, 6), sharex=True)

temp_text = fig.suptitle("Skin Temperature: -- �C", fontsize=14)
line_mother, = ax_mother.plot(time_axis, mother_ecg, label="Mother ECG", color="red")
line_baby, = ax_baby.plot(time_axis, baby_ecg, label="Baby ECG", color="blue")

# Set axes
ax_mother.set_ylim(mother_ecg.min() - 0.1, mother_ecg.max() + 0.1)
ax_baby.set_ylim(baby_ecg.min() - 0.1, baby_ecg.max() + 0.1)
plt.pause(SAMPLE_INTERVAL)
ax_mother.set_xlim(-10, 0)  # Fixed x-axis

ax_baby.set_ylabel("Amplitude")
ax_baby.set_title("Baby ECG (Calculated)")
ax_baby.set_xlabel("Time (s)")
ax_baby.grid(True)
ax_baby.set_xlim(-10, 0)  # Fixed x-axis

plt.tight_layout(rect=[0, 0, 1, 0.95])  # Leave space for suptitle

async def main():
    print("Scanning for ESP32...")

    # Scan for device
    devices = await BleakScanner.discover()
    esp32_device = next((dev for dev in devices if "ADC" in dev.name), None)

    if not esp32_device:
        print("ESP32 not found.")
        return

    print(f"Found ESP32: {esp32_device.address}")

    # Connect to device
    async with BleakClient(esp32_device.address) as client:
        print("Connected to ESP32!")

        while True:
            try:
                # Read all values
                raw_values = []
                for i, uuid in enumerate(CHARACTERISTIC_UUIDS):
                    raw = await client.read_gatt_char(uuid)
                    raw_values.append(raw)

                mother_val = struct.unpack('f', raw_values[0])[0]
                sum_val = struct.unpack('f', raw_values[1])[0]
                skin_temp = struct.unpack('f', raw_values[2])[0]

                # Calculate baby ECG
                baby_val = sum_val - mother_val

                print(f"Mother Signal: {mother_val}")
                print(f"Baby Signal: {baby_val}")
                print(f"Sum_val: {sum_val}")
                print(f"Skin Temp: {skin_temp}")

                # Update buffers
                global mother_ecg, baby_ecg
                mother_ecg = np.roll(mother_ecg, -1)
                baby_ecg = np.roll(baby_ecg, -1)
                mother_ecg[-1] = mother_val
                baby_ecg[-1] = baby_val

                # Update plots
                line_mother.set_ydata(mother_ecg)
                line_baby.set_ydata(baby_ecg)
                temp_text.set_text(f"Skin Temperature: {skin_temp:.2f} C")
                ax_mother.relim()
                ax_baby.relim()
                plt.pause(SAMPLE_INTERVAL)

                # Update axes
                ax_mother.set_ylim(mother_ecg.min() - 0.1, mother_ecg.max() + 0.1)
                ax_baby.set_ylim(baby_ecg.min() - 0.1, baby_ecg.max() + 0.1)
                plt.pause(SAMPLE_INTERVAL)

            except Exception as e:
                print(f"Error: {e}")
                break

# Run
asyncio.run(main())
