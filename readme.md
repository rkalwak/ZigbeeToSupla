# Environment configuration
Go to :
C:\Users\USER\\.platformio\platforms\espressif32\boards
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

You need to have zigbee_zczr_custom.csv file available in location:

```
C:\Users\USER\.platformio\packages\framework-arduinoespressif32@src-862afffe99ca01146a6a0c843009d37a\tools\partitions
```

Copy priv_auth_data_example.h as priv_auth_data.h and fill in the credentials

# How it works?

## Network join

Register new Supla channel when new device joins network
- consider type 
Save it in ESP memory with ID of device, keep them ordered as they joined so there will be no conflicts when new device joins

## State change

Find device by id and apply it state

