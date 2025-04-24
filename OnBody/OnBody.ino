#include <Arduino.h>
#include <ArduinoBLE.h>

#include "driver/adc.h"
#include "esp_adc_cal.h"
// Temp Sensor Libs
#include <OneWire.h>
#include <DallasTemperature.h>

#define ADC_CHANNEL1 ADC1_CHANNEL_0  // GPIO 36 corresponds to ADC1_CHANNEL_0
#define ADC_CHANNEL2 ADC1_CHANNEL_1  // GPIO 36 corresponds to ADC1_CHANNEL_0

// bluetooth address is 74:4d:bd:a1:ab:65
//BluetoothSerial SerialBT;  // Create a Bluetooth Serial object

// Data wire is plugged into GPIO3 on the Arduino
#define ONE_WIRE_BUS 3

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

BLEService ADCService("180D");

BLEFloatCharacteristic TempF("2A6E", BLERead | BLENotify);
BLEFloatCharacteristic V1("2B18", BLERead | BLENotify);
BLEFloatCharacteristic V2("2B19", BLERead | BLENotify);

void BLESetup(){
  if (!BLE.begin()) { 
    //Serial.println("starting BLE failed!");

    //while (1);
  }
  BLE.setDeviceName("ADC");
  BLE.setLocalName("ADC");
  BLE.setAdvertisedService(ADCService); // add the service UUID
  ADCService.addCharacteristic(TempF); // add the battery level characteristic
  ADCService.addCharacteristic(V1);
  ADCService.addCharacteristic(V2);
  BLE.addService(ADCService); // Add the battery service
  TempF.writeValue(0); // set initial value for this characteristic
  V1.writeValue(0);
  V2.writeValue(0);
  BLE.advertise();
 // BLE.setConnectable(true);
  //Serial.println("BLE Started");
}

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
    int adc_value1 = adc1_get_raw(ADC_CHANNEL1); // Use ADC1 function correctly
    int adc_value2 = adc1_get_raw(ADC_CHANNEL2); // Use ADC1 function correctly
    //Serial.print("Raw_ADC1_Value:");
    //Serial.print(adc_value1);
   // Serial.print(",");
    float voltage1 = adc_value1 * (3.3 / 4095.0);
    //Serial.print("Voltage_1:");
    //Serial.print(voltage1, 3);
    V1.writeValue(voltage1);
    //Serial.print(",");
    //Serial.print("Raw_ADC2_Value:");
    //Serial.print(adc_value2);
    //Serial.print(",");
    float voltage2 = adc_value2 * (3.3 / 4095.0);
    //Serial.print("Voltage_2:");
    //Serial.println(voltage2, 3);
    V2.writeValue(voltage2);
    
    /*int heartbeat = (int)(voltage * 100);
    Serial.print("Heartbeat: ");
    Serial.println(heartbeat);*/
    
    //delay(1000);
    }

void temp_task(){
  // call sensors.requestTemperatures() to issue a global temperature 
  // request to all devices on the bus
  //Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  //Serial.println("DONE");
  // After we got the temperatures, we can print them here.
  // We use the function ByIndex, and as an example get the temperature from the first sensor only.
  float temp = sensors.getTempFByIndex(0);

  // Check if reading was successful
  if(true || (temp != float(-196.60)))
  {
    Serial.print("Temp:");
    Serial.print(temp);
    TempF.writeValue(temp);
    Serial.print(",");
  } 
  else
  {
    Serial.print("TempError");
    TempF.writeValue(temp);
    sensors.begin();
  }
}

void setup() {
    //Serial.begin(115200);
    //Serial.println("Bluetooth Started! Waiting for connection...");
    configure_adc();  // Initialize ADC here!
    // Start up the Temp Sensor library
    sensors.begin();
    temp_task();
    BLESetup();
    //Serial.print("Local address is: ");
    //Serial.println(BLE.address());
}

void loop() {
  BLEDevice central = BLE.central();
  central.connected();
  if (central) {
    //Serial.print("Connected to central: ");
    // print the central's BT address:
    //Serial.println(central.address());
    // turn on the LED to indicate the connection:
    digitalWrite(LED_BUILTIN, HIGH);

    // check the battery level every 200ms
    // while the central is connected:
    while (central.connected()) {
      heartbeat_task();
      //temp_task();
    }
    // when the central disconnects, turn off the LED:
    
   // Serial.print("Disconnected from central: ");
    //Serial.println(central.address());
  }
  digitalWrite(LED_BUILTIN, LOW);
  //Serial.print("Local address is: ");

  //Serial.println(BLE.address());
}
