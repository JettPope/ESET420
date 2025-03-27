#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <driver/adc.h>
#include <esp_log.h>
#include <esp_err.h>
#include <esp_bt.h>
#include <esp_bt_device.h>
#include <esp_bt_main.h>
#include <esp_spp_api.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define SPP_SERVER_NAME "ESP32_HEARTBEAT"
#define ADC_CHANNEL ADC1_CHANNEL_0  // GPIO 36 on ESP32

static const char *TAG = "ESP32_BT";
static uint32_t spp_handle = 0;

// Bluetooth SPP Callback
static void esp_spp_callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
    switch (event) {
        case ESP_SPP_SRV_OPEN_EVT: // Client connected
            spp_handle = param->open.handle;
            ESP_LOGI(TAG, "Bluetooth Connected!");
            break;
        case ESP_SPP_CLOSE_EVT: // Client disconnected
            spp_handle = 0;
            ESP_LOGI(TAG, "Bluetooth Disconnected");
            break;
        default:
            break;
    }
}

// Configure ADC
void configure_adc() {
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN_DB_11);
}

// Send Data via Bluetooth
void send_bluetooth_data(const char *data) {
    if (spp_handle != 0) {
        esp_spp_write(spp_handle, strlen(data), (uint8_t *)data);
        ESP_LOGI(TAG, "Sent: %s", data);
    }
}

// Heartbeat Monitoring Task
void heartbeat_task(void *param) {
    configure_adc();
    
    while (1) {
        int adc_value = adc1_get_raw(ADC_CHANNEL);
        float voltage = adc_value * (3.3 / 4095.0);
        int heartbeat = (int)(voltage * 100);  // Convert voltage to BPM

        char buffer[50];
        snprintf(buffer, sizeof(buffer), "Heartbeat: %d BPM\n", heartbeat);
        send_bluetooth_data(buffer);

        vTaskDelay(pdMS_TO_TICKS(2000)); // Send every 2 seconds
    }
}

// Initialize Bluetooth SPP
void bluetooth_init() {
    esp_err_t ret = esp_bt_controller_mem_release(ESP_BT_MODE_BLE); // Release BLE memory if not used
    if (ret) {
        ESP_LOGE(TAG, "Bluetooth memory release failed: %s", esp_err_to_name(ret));
        return;
    }

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(TAG, "Bluetooth controller init failed: %s", esp_err_to_name(ret));
        return;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT);
    if (ret) {
        ESP_LOGE(TAG, "Bluetooth enable failed: %s", esp_err_to_name(ret));
        return;
    }

    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(TAG, "Bluedroid init failed: %s", esp_err_to_name(ret));
        return;
    }

    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(TAG, "Bluedroid enable failed: %s", esp_err_to_name(ret));
        return;
    }

    ret = esp_spp_register_callback(esp_spp_callback);
    if (ret) {
        ESP_LOGE(TAG, "SPP callback registration failed: %s", esp_err_to_name(ret));
        return;
    }

    ret = esp_spp_init(ESP_SPP_MODE_CB);
    if (ret) {
        ESP_LOGE(TAG, "SPP init failed: %s", esp_err_to_name(ret));
        return;
    }

    esp_spp_start_srv(ESP_SPP_SEC_AUTHENTICATE, ESP_SPP_ROLE_SLAVE, 0, SPP_SERVER_NAME);
}

// Main application
void app_main() {
    bluetooth_init();
    xTaskCreate(heartbeat_task, "heartbeat_task", 4096, NULL, 5, NULL);
}
