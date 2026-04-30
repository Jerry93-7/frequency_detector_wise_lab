# frequency_detector_wise_lab
This is a frequency detector that I got to run using the wasmtime engine on esp32c6_devkitc.  

## ESP32C6 Devkitc
The esp32 directory was placed under the zephyrproject/zephyr/apps/ directory (I created the "apps" directory under the zephyr directory myself).

### Pins for ESP32:
Setup one of the adc capable GPIO pins in the board overlay file.  Also the secondary uart is necessary for transmitting rr bytes back to 
the laptop host device.  Any WIFI stuff in bridge.c did not work out and can be removed.

### 

