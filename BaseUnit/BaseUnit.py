import asyncio
import struct
import numpy as np
import matplotlib.pyplot as plt
from bleak import BleakScanner, BleakClient

# Custom service/char UUIDs
SERVICE_UUID = "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
CHARACTERISTIC_UUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e"

BUFFER_SIZE = 200
SAMPLE_INTERVAL = 0.05

mother_ecg = np.zeros(BUFFER_SIZE)
baby_ecg = np.zeros(BUFFER_SIZE)
time_axis = np.linspace(-BUFFER_SIZE * SAMPLE_INTERVAL, 0, BUFFER_SIZE)

plt.ion()
fig, (ax_mother, ax_baby) = plt.subplots(2, 1, figsize=(10, 6), sharex=True)
temp_text = fig.suptitle("Skin Temperature: -- �C", fontsize=14)

line_mother, = ax_mother.plot(time_axis, mother_ecg, label="Mother ECG", color="red")
line_baby, = ax_baby.plot(time_axis, baby_ecg, label="Baby ECG", color="blue")

ax_mother.set_ylabel("Amplitude")
ax_mother.set_title("Mother ECG")
ax_mother.grid(True)

ax_baby.set_ylabel("Amplitude")
ax_baby.set_title("Baby ECG (Calculated)")
ax_baby.set_xlabel("Time (s)")
ax_baby.grid(True)

ax_mother.set_xlim(-10, 0)
ax_baby.set_xlim(-10, 0)
plt.tight_layout(rect=[0, 0, 1, 0.95])

def update_plot(mother_val, baby_val, temp):
    global mother_ecg, baby_ecg
    mother_ecg = np.roll(mother_ecg, -1)
    baby_ecg = np.roll(baby_ecg, -1)
    mother_ecg[-1] = mother_val
    baby_ecg[-1] = baby_val

    line_mother.set_ydata(mother_ecg)
    line_baby.set_ydata(baby_ecg)
    temp_text.set_text(f"Skin Temperature: {temp:.2f} �C")

    ax_mother.set_ylim(mother_ecg.min() - 0.05, mother_ecg.max() + 0.05)
    ax_baby.set_ylim(baby_ecg.min() - 0.05, baby_ecg.max() + 0.05)
    plt.pause(0.001)

def notification_handler(_, data):
    # Unpack 3 floats
    mother_val, sum_val, temp = struct.unpack('fff', data)
    baby_val = sum_val - mother_val
    print(f"Received: M={mother_val:.2f} S={sum_val:.2f} T={temp:.2f}")
    update_plot(mother_val, baby_val, temp)

async def main():
    print("Scanning for ESP32...")
    devices = await BleakScanner.discover()
    esp32 = next((d for d in devices if "ADC-BLE" in d.name), None)

    if not esp32:
        print("ESP32 not found.")
        return

    print(f"Connecting to {esp32.address}...")
    async with BleakClient(esp32.address) as client:
        print("Connected! Starting notifications...")
        await client.start_notify(CHARACTERISTIC_UUID, notification_handler)

        while True:
            await asyncio.sleep(1)  # Keeps the loop alive

asyncio.run(main())
