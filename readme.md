# Environment configuration
Go to :
C:\Users\USER\.platformio\platforms\espressif32\boards
Find file: esp32-c6-devkitc-1.json
Ensure content to be:
```json

{
  "build": {
    "core": "esp32",
    "f_cpu": "160000000L",
    "f_flash": "80000000L",
    "flash_mode": "qio",
    "mcu": "esp32c6",
    "variant": "esp32c6"
  },
  "connectivity": [
    "wifi"
  ],
  "frameworks": [
    "espidf",
    "arduino"
  ],
  "name": "Espressif ESP32-C6-DevKitC-1",
  "upload": {
    "flash_size": "4MB",
    "maximum_ram_size": 327680,
    "maximum_size": 8388608,
    "require_upload_port": true,
    "speed": 460800
  },
  "url": "https://docs.espressif.com/projects/espressif-esp-dev-kits/en/latest/esp32c6/esp32-c6-devkitc-1/index.html",
  "vendor": "Espressif"
}



```

Warning! default flash_size is 8MB but to satisfy Zigbee partition I changed to 4MB

# How it works?

## Network join

Register new Supla channel when new device joins network
- consider type 
Save it in ESP memory with ID of device, keep them ordered as they joined so there will be no conflicts when new device joins

## State change

Find device by id and apply it state
