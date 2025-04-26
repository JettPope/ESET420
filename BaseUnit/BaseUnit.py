import asyncio
import struct
import numpy as np
import matplotlib.pyplot as plt
import matplotlib as mpl
from bleak import BleakScanner, BleakClient
import time

UUID = "00002b18-0000-1000-8000-00805f9b34fb"

# Plotting parameters
SAMPLE_INTERVAL = 0.05
BUFFER_SIZE = int(1 / SAMPLE_INTERVAL * 100)

# Buffers
mother_ecg = np.zeros(BUFFER_SIZE)
baby_ecg = np.zeros(BUFFER_SIZE)
time_axis = np.linspace(-BUFFER_SIZE * SAMPLE_INTERVAL, 0, BUFFER_SIZE)
MOTHER_THRESHOLD = 0.6
BABY_THRESHOLD = 0.06
MAX_BEATS_TRACKED = 27  # Sliding window
mother_beat_times = []
baby_beat_times = []
last_mother_val = 0
last_baby_val = 0

# Matplotlib setup
mpl.rcParams['toolbar'] = 'None'
plt.ion()

fig, (ax_mother, ax_baby) = plt.subplots(2, 1, figsize=(10, 6), sharex=True)
fig.canvas.header_visible = False
fig.canvas.footer_visible = False

# permanent text sections
status_text = fig.suptitle("Connecting to On-Body Device...", fontsize=24, y=0.98)
heart_rate_text = fig.text(0.5, 0.9, "", ha='center', va='center', fontsize=18, color='black')


line_mother, = ax_mother.plot(time_axis, mother_ecg, label="Mother ECG", color="red")
line_baby, = ax_baby.plot(time_axis, baby_ecg, label="Baby ECG", color="blue")

plt.get_current_fig_manager().full_screen_toggle()

# Mother ECG plot setup
ax_mother.set_xlim(-10, 0)
ax_mother.set_ylabel("Amplitude")
ax_mother.set_xlabel("Time (s)")
ax_mother.set_title("Mother ECG")
ax_mother.grid(True)
ax_mother.xaxis.set_tick_params(labelbottom=True)

# Baby ECG plot setup
ax_baby.set_xlim(-10, 0)
ax_baby.set_ylabel("Amplitude")
ax_baby.set_title("Baby ECG")
ax_baby.set_xlabel("Time (s)")
ax_baby.grid(True)

plt.tight_layout(rect=[0, 0, 1, 0.95])
plt.pause(SAMPLE_INTERVAL)

def ecg_handler(sender, data):
    global mother_ecg, baby_ecg
    global mother_beat_times, baby_beat_times
    global last_mother_val, last_baby_val

    try:
        if len(data) >= 8:
            mother_val, sum_val = struct.unpack('ff', data[:8])
            baby_val = sum_val - mother_val

            # Update circular buffer
            mother_ecg = np.roll(mother_ecg, -1)
            mother_ecg[-1] = mother_val
            baby_ecg = np.roll(baby_ecg, -1)
            baby_ecg[-1] = baby_val

            # Heartbeat detection
            t_now = time.time()
            if last_mother_val < MOTHER_THRESHOLD <= mother_val:
                mother_beat_times.append(t_now)
                if len(mother_beat_times) > MAX_BEATS_TRACKED:
                    mother_beat_times.pop(0)

            if last_baby_val < BABY_THRESHOLD <= baby_val:
                baby_beat_times.append(t_now)
                if len(baby_beat_times) > MAX_BEATS_TRACKED:
                    baby_beat_times.pop(0)

            last_mother_val = mother_val
            last_baby_val = baby_val

    except Exception as e:
        print(f"[Handler Error] Failed to parse data: {e}")

def compute_bpm(timestamps):
    if len(timestamps) < 2:
        return 0
    intervals = np.diff(timestamps)
    avg_interval = np.mean(intervals)
    if avg_interval == 0:
        return 0
    return 60.0 / avg_interval

def update_plot():
    line_mother.set_ydata(mother_ecg)
    line_baby.set_ydata(baby_ecg)
    ax_mother.set_ylim(mother_ecg.min() - 0.1, mother_ecg.max() + 0.2)
    ax_baby.set_ylim(max(0, baby_ecg.min() - 0.02), baby_ecg.max() + 0.02)

    bpm_mother = compute_bpm(mother_beat_times)
    bpm_baby = compute_bpm(baby_beat_times)

    heart_rate_text.set_text(f"Mother BPM: {bpm_mother:.1f}   Baby BPM: {bpm_baby:.1f}")

    plt.pause(SAMPLE_INTERVAL / 10)

def save_and_close():
    try:
        file = open(f"/media/BJAP/B352DRIVE/ECGDATA{len(mother_ecg)}.csv", "x")
        file.write("Time,MotherECG,FetusECG\n")
        for x in range(len(mother_ecg)):
            file.write(f"{x},{mother_ecg[x]},{baby_ecg[x]}")
        file.close()
    except Exception as e:
        status_text.set_text("Correct Drive Not Connected, Please Use Only The Provided Drive")
        plt.pause(0.1)

butt = mpl.widgets.Button(ax_mother, 'Save Data',color="red")
butt.on_clicked(save_and_close())

async def main():
    global status_text

    while True:
        try:
            status_text.set_text("Scanning for On-Body Device...")
            plt.pause(0.1)
            devices = await BleakScanner.discover()
            esp32_device = next((dev for dev in devices if "ADC" in dev.name), None)

            if not esp32_device:
                status_text.set_text("On-Body Device Not Found. Retrying...")
                print("ESP32 not found. Retrying in 1s...")
                await asyncio.sleep(1)
                continue

            print(f"Found ESP32: {esp32_device.address}")
            status_text.set_text("Connecting to On-Body Device...")
            plt.pause(0.1)

            async with BleakClient(esp32_device.address) as client:
                print("Connected!")
                status_text.set_text("Connected to On-Body Device")
                plt.pause(0.1)
                #plt.draw()
#                try:
                #while(True):
                await client.start_notify(UUID, ecg_handler)

                while await client.is_connected():
                    update_plot()
                    await asyncio.sleep(0.01)  # 20 FPS plot update rate
                await asyncio.Event().wait()
                print("Disconnected from On-Body device")
#                await asyncio.sleep(1)
#                except Exception as e:
#                    print(f"[Reconnect] Connection lost or failed: {e}")
#                    status_text.set_text("Disconnected. Reconnecting to On-Body Device...")
#                    plt.pause(0.1)
#                    await asyncio.sleep(3)
                #while True:
                    #await asyncio.sleep(1)

        except Exception as e:
            print(f"[Reconnect] Connection lost or failed: {e}")
            status_text.set_text("Disconnected. Reconnecting to On-Body Device...")
            plt.pause(0.1)
            await asyncio.sleep(3)

asyncio.run(main())
