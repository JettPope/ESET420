#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <esp_system.h>
#include <driver/adc.h>
#include <esp_log.h>

// Bluetooth device (adjust for HC-05/HC-06)
#define BT_DEVICE "/dev/rfcomm0"
#define ADC_CHANNEL ADC1_CHANNEL_0  // GPIO 36 on ESP32

// Function to configure ADC
void configure_adc() {
    adc1_config_width(ADC_WIDTH_BIT_12);  // 12-bit resolution
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN_DB_11);
}

// Function to open Bluetooth connection
int open_bluetooth_connection(const char *device) {
    int bt_fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (bt_fd == -1) {
        perror("Failed to open Bluetooth device");
        return -1;
    }

    struct termios options;
    tcgetattr(bt_fd, &options);
    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);

    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_cflag |= (CLOCAL | CREAD);

    tcsetattr(bt_fd, TCSANOW, &options);
    return bt_fd;
}

// Function to send data via Bluetooth
void send_data(int bt_fd, const char *data) {
    if (write(bt_fd, data, strlen(data)) < 0) {
        perror("Failed to send data");
    } else {
        printf("Sent: %s\n", data);
    }
}

int main() {
    configure_adc();

    int bt_fd = open_bluetooth_connection(BT_DEVICE);
    if (bt_fd == -1) {
        return 1;
    }

    while (1) {
        int adc_value = adc1_get_raw(ADC_CHANNEL); // Read ADC value
        float voltage = adc_value * (3.3 / 4095.0); // Convert to voltage
        int heartbeat = (int)(voltage * 100);  // Convert to BPM (adjust scaling)

        char buffer[50];
        snprintf(buffer, sizeof(buffer), "Heartbeat: %d BPM\n", heartbeat);
        send_data(bt_fd, buffer);

        sleep(2);  // Send every 2 seconds
    }

    close(bt_fd);
    return 0;
}
