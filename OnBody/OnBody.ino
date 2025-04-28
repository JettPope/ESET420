#include <Arduino.h>
#include <ArduinoBLE.h>

#include "driver/adc.h"
#include "esp_adc_cal.h"

#define ADC_CHANNEL1 ADC1_CHANNEL_0  // GPIO 1 corresponds to ADC1_CHANNEL_0
#define ADC_CHANNEL2 ADC1_CHANNEL_1  // GPIO 2 corresponds to ADC1_CHANNEL_1

BLEService ADCService("180D");

BLECharacteristic ecgData("2B18", BLERead | BLENotify, sizeof(float) * 2, true); // 8 bytes

void BLESetup(){
  if (!BLE.begin()) { 
    
  }
  BLE.setDeviceName("ADC");
  BLE.setLocalName("ADC");
  BLE.setAdvertisedService(ADCService); // add the service UUID
  ADCService.addCharacteristic(ecgData);
  BLE.addService(ADCService); // Add the service
  BLE.advertise();
}

// ADC config
void configure_adc() {
    // Configure ADC width
    adc1_config_width(ADC_WIDTH_BIT_12);
    
    // Configure attenuation for proper voltage reading
    adc1_config_channel_atten(ADC_CHANNEL1, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC_CHANNEL2, ADC_ATTEN_DB_11);
    
    // Apply ADC calibration
    esp_adc_cal_characteristics_t *adc_chars = 
        (esp_adc_cal_characteristics_t*) calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, adc_chars);
}

void heartbeat_task() {
  int adc_value1 = adc1_get_raw(ADC_CHANNEL1);
  int adc_value2 = adc1_get_raw(ADC_CHANNEL2);

  float voltage1 = adc_value1 * (3.3 / 4095.0);
  float voltage2 = adc_value2 * (3.3 / 4095.0);

  uint8_t buffer[8];
  memcpy(&buffer[0], &voltage1, sizeof(float));
  memcpy(&buffer[4], &voltage2, sizeof(float));
  Serial.print("2,");
  Serial.print(voltage1);
  Serial.print(",0,");
  Serial.println(voltage2);
  ecgData.writeValue(buffer, 8);
}

void setup() {
    Serial.begin(115200);
    configure_adc();  // Initialize ADC here!
    BLESetup();
}

void loop() {
  BLEDevice central = BLE.central();
  central.connected();
  if (central) {
    // while the central is connected:
    while (central.connected()) {
      heartbeat_task();
      //temp_task();
    }
    // when the central disconnects
  }
}
