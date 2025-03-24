#include <Arduino.h>
#include "BluetoothSerial.h"

#include "driver/adc.h"
#include "esp_adc_cal.h"

#define ADC_CHANNEL ADC1_CHANNEL_0  // GPIO 36 corresponds to ADC1_CHANNEL_0

//BluetoothSerial SerialBT;  // Create a Bluetooth Serial object


void configure_adc() {
    // Configure ADC width
    adc1_config_width(ADC_WIDTH_BIT_12);
    
    // Configure attenuation for proper voltage reading
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN_DB_11);
    
    // Apply ADC calibration
    esp_adc_cal_characteristics_t *adc_chars = 
        (esp_adc_cal_characteristics_t*) calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, adc_chars);
}

void send_bluetooth_data(const char *data) {
    Serial.print(data);
    Serial.print("Sent: ");
    Serial.println(data);
}

void heartbeat_task() {
    int adc_value = adc1_get_raw(ADC_CHANNEL); // Use ADC1 function correctly
    Serial.print("Raw ADC Value: ");
    Serial.println(adc_value);
    
    float voltage = adc_value * (3.3 / 4095.0);
    Serial.print("Voltage: ");
    Serial.println(voltage, 3);
    
    int heartbeat = (int)(voltage * 100);
    Serial.print("Heartbeat: ");
    Serial.println(heartbeat);
    
    delay(2000);
    }


void setup() {
    Serial.begin(115200);
    Serial.println("Bluetooth Started! Waiting for connection...");
    configure_adc();  // Initialize ADC here!
}


void loop() {
    heartbeat_task();
    
}
