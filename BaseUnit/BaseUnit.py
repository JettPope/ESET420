import asyncio
import struct
import numpy as np
import matplotlib.pyplot as plt
import matplotlib as mpl
from bleak import BleakScanner, BleakClient

UUID = "00002b18-0000-1000-8000-00805f9b34fb"

# Plotting parameters
SAMPLE_INTERVAL = 0.05
BUFFER_SIZE = int(1 / SAMPLE_INTERVAL * 10)

# Buffers
mother_ecg = np.zeros(BUFFER_SIZE)
baby_ecg = np.zeros(BUFFER_SIZE)
time_axis = np.linspace(-BUFFER_SIZE * SAMPLE_INTERVAL, 0, BUFFER_SIZE)

# Matplotlib setup
mpl.rcParams['toolbar'] = 'None'
plt.ion()

fig, (ax_mother, ax_baby) = plt.subplots(2, 1, figsize=(10, 6), sharex=True)
fig.canvas.header_visible = False
fig.canvas.footer_visible = False

status_text = fig.suptitle("Connecting to On-Body Device...", fontsize=14)
line_mother, = ax_mother.plot(time_axis, mother_ecg, label="Mother ECG", color="red")
line_baby, = ax_baby.plot(time_axis, baby_ecg, label="Baby ECG", color="blue")

plt.get_current_fig_manager().full_screen_toggle()

# Mother ECG plot setup
ax_mother.set_xlim(-10, 0)
ax_mother.set_ylabel("Amplitude")
ax_mother.set_xlabel("Time (s)")
ax_mother.set_title("Mother ECG")
ax_mother.grid(True)

# Baby ECG plot setup
ax_baby.set_xlim(-10, 0)
ax_baby.set_ylabel("Amplitude")
ax_baby.set_title("Baby ECG")
ax_baby.set_xlabel("Time (s)")
ax_baby.grid(True)

plt.tight_layout(rect=[0, 0, 1, 0.95])
plt.pause(SAMPLE_INTERVAL)

# Notification handler
#def ecg_handler(sender, data):
#    global mother_ecg, baby_ecg
#
#    if len(data) >= 8:
#        mother_val, sum_val = struct.unpack('ff', data)
#        baby_val = sum_val - mother_val
#
#        mother_ecg = np.roll(mother_ecg, -1)
#        mother_ecg[-1] = mother_val
#
#        baby_ecg = np.roll(baby_ecg, -1)
#        baby_ecg[-1] = baby_val

        #update_plot()
def ecg_handler(sender, data):
    global mother_ecg, baby_ecg

    try:
        if len(data) >= 8:
            mother_val, sum_val = struct.unpack('ff', data[:8])
            baby_val = sum_val - mother_val

            mother_ecg = np.roll(mother_ecg, -1)
            mother_ecg[-1] = mother_val

            baby_ecg = np.roll(baby_ecg, -1)
            baby_ecg[-1] = baby_val
    except Exception as e:
        print(f"[Handler Error] Failed to parse data: {e}")


def update_plot():
    line_mother.set_ydata(mother_ecg)
    line_baby.set_ydata(baby_ecg)
    ax_mother.set_ylim(mother_ecg.min() - 0.1, mother_ecg.max() + 0.1)
    ax_baby.set_ylim(baby_ecg.min() - 0.1, baby_ecg.max() + 0.1)
    #plt.pause(SAMPLE_INTERVAL / 10)
    plt.canvas.draw()
#    await asyncio.sleep(1)

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
