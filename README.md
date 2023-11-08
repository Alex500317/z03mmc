# Zigbee 3.0 Firmware for original LYWSD03MMC Sensor

This repository forked from [devbis/z03mmc](https://github.com/devbis/z03mmc) (!)

Актуальную версию смотрите в [devbis/z03mmc](https://github.com/devbis/z03mmc)!


 **An alternative version flash the Zigbee firmware in Xiaomi LYWSD03MMC in TelinkMiFlasher from version 6.7.**

To flash the Zigbee firmware, use a Google Chrome, Microsoft Edge or Opera Browser.
	1. Go to the [Over-the-air Webupdater Page TelinkMiFlasher.html](https://pvvx.github.io/ATC_MiThermometer/TelinkMiFlasher.html)
	2. If using Android, Windows, Linux: Ensure you enabled "experimental web platform features". Therefore copy the according link (i.e. chrome://flags/#enable-experimental-web-platform-features for Chrome), open a new browser tab, paste the copied URL. Now sten the _Experimental Web Platform features_ flag to _Enabled_. Then restart the browser.
	3. In the Telink Flasher Page: Press `Connect:`. The browser should open a popup with visible Bluetooth devices. Choose the according target device (i.e. LYWSD03MMC) to pair.
	4. After connection is established a _Do Acivation_ button appears. Press this button to start the decryption key process.
	5. Now you can press the Zigbee Firmware button to directly flash the `Zigbee Firmware`:<br>![image](https://github.com/devbis/z03mmc/assets/12629515/f0212a45-4c2c-43a1-a237-5ec358e6d0cb)<br>Alternatively you can choose a specific firmware binary (i.e. the original firmware) via the file chooser
	7. Press Start Flashing. Wait for the firmware to finish.
	8. The device should now show up in your Zigbee bridge (If joining is enabled, of course). If it doesn't, reinsert the battery and/or short the RESET and GND contacts on the board for 3 seconds.

A brief description of the installed version of the Zigbee firmware is in this [fork](https://github.com/pvvx/z03mmc).
In the future, you can update the [Zigbee firmware to the version from devbis](https://github.com/devbis/z03mmc).

---

Внимание: Без регистрации в сети Zigbee термометр не работает. Регистрация термометра в сети Zigbee производится путем замыкания контактов “reset” с "gnd" на 3 секунды или при включении питания после перепрошивки. Время регистрации для экономии питания ограничено путем отключения работы термометра через 45 сек. Потребление после отключения около 3 мкА.

Уровень готовности прошивки - Beta version. Требуются дополнительные тесты.

**Индикация на LCD экране:**

Длительно отображаемый значок “BT” обозначает потерю связи или отсутствие регистрации термометра в сети Zigbee.

Мигающий значок “BT” вызывается по команде identify.

Индикация “---“ говорит о том, что время регистрации термометра в сети вышло. Для возобновления регистрации термометра в сети требуется повторить соединение контактов “reset” с “gnd” на 3 секунды.


**Текущие дополнения:**

1. Добавлена сборка проекта с помощью ‘make’ (Windows/linux) и возможность импорта 'Existing Project' в "Telink IoT Studio".

2. Заданы интервалы по умолчанию для отчетов по температуре и влажности в 20-120 (мин-мах) секунд, reportableChange в 0.1C и 1%. 

3. Оптимизировано потребление при отключении координатора или потере связи. Выполняя _rejoin_, при нарушении связи термометр потребляет достатчно много. Следите за работой вашего координатора сети - Zigbee не любит (практически не переносит) отключений координатора.

4. Оптимизировано потребление при опросе контакта "reset".

Итоговое среднее потребление LYWSD03MC B1.4 при измерении от источника 3.3V от 14 до 26 мкА в зависимости от динамики изменений температуры и влажности. (Для сравнения: Вариант с BLE потребляет 14 мкА при default настройках и постоянной передаче всех измеренных значений каждые 10 секунд. Дальность связи в режиме BLE LongRange составляет до 1 км по прямой, для Zigbee - сотни метров.)

---

Для сборки под Linux выполнить:

```
git clone https://github.com/pvvx/z03mmc
make install
make
```

Out log:
```
Warning: Undefined symbol 'ss_apsmeSwitchKeyReq'!
Warning: Undefined symbol 'ss_apsmeTransportKeyReq'!
Warning: Undefined symbol 'tl_zbNwkBeaconPayloadUpdate'!
===================================================================
 Section|          Description| Start (hex)|   End (hex)|Used space
-------------------------------------------------------------------
 ramcode|   Resident Code SRAM|           0|        1510|    5392
    text|           Code Flash|        1510|       1F304|  122356
  rodata| Read Only Data Flash|           0|           0|       0
 cusdata|          Custom SRAM|      8452EC|      8452EC|       0
      nc|   Wasteful Area SRAM|      841510|      841600|     240
   ictag|     Cache Table SRAM|      841600|      841700|     256
  icdata|      Cache Data SRAM|      841700|      841F00|    2048
    data|       Init Data SRAM|      841F00|      8420C8|     456
     bss|        BSS Data SRAM|      8420D0|      8452EC|   12828
 irq_stk|        BSS Data SRAM|      8420D0|      8422D0|     512
    cbss| Custom BSS Data SRAM|      8452EC|      8452EC|       0
   stack|       CPU Stack SRAM|      8452EC|      850000|   44308
   flash|       Bin Size Flash|           0|       1F4CC|  128204
-------------------------------------------------------------------
Start Load SRAM : 0 (ICtag: 0x0)
Total Used SRAM : 21228 from 65536
Total Free SRAM : 240 + stack[44308] = 44548
```

Под Window, после импорта в ‘Telink IoT Studio’, выполнить “make install” или в папке проекта:
```
git clone https://github.com/devbis/tl_zigbee_sdk.git -b 3.6.8.5 --depth 1 ./SDK
```
И включить многопоточную сборку для сборки всего проекта с нуля за 2..3 сек (make -j -> 3 сек на 12-ти ядрах/24 потока).


---

This repository contains the Zigbee firmware for Xiaomi LYWSD03MMC Bluetooth temperature and humidity sensor.

## Overview

![](./assets/device.jpg)

The LYWSD03MMC is a Bluetooth temperature and humidity sensor that can be integrated into a Zigbee network using
this firmware. This repository hosts the code and related resources to flash the device and make 
it compatible with Zigbee networks.

## Features
- Full-featured firmware to convert Xiaomi LYWSD03MC device with default ZCL battery, temperature and relative humidity clusters
- Display support for known revisions
- OTA support in firmware and binaries in ZCL format for update 
- Flashable over-the-air from custom ATC firmware https://pvvx.github.io/ATC_MiThermometer/TelinkMiFlasher.html
- Flashable over SWS-UART interface using one of:

  - https://pvvx.github.io/ATC_MiThermometer/USBCOMFlashTx.html
  - https://github.com/pvvx/ATC_MiThermometer/blob/master/TLSR825xComFlasher.py 
  - https://github.com/pvvx/TLSRPGM 

## Getting Started

### Prerequisites

- Zigbee compatible hardware (e.g., Zigbee coordinator or gateway).
- Necessary tools for flashing firmware to the sensor.

### Building firmware

1. Clone TC32 toolchain according to your host OS:
    ```sh
    git clone https://github.com/devbis/tc32.git -b linux
    ```
    ```sh
    git clone https://github.com/devbis/tc32.git -b macos
    ```
    ```sh
    git clone https://github.com/devbis/tc32.git -b windows
    ```

2. Clone this repository and SDK:

    ```sh
    git clone https://github.com/devbis/z03mmc.git
    git clone https://github.com/devbis/tl_zigbee_sdk.git -b 3.6.8.5 --depth 1
   
    cd z03mmc
    ```
   
3. Configure and build:
    ```sh
    cmake -DSDK_PREFIX=$(pwd)/../tl_zigbee_sdk -DTOOLCHAIN_PREFIX=$(pwd)/../tc32 -B build .
    cmake --build build --target z03mmc.zigbee
    ```

    Firmware binary is located at `build/src/z03mmc.bin`
    The binary with OTA header is at the same folder, ending with `z03mmc.zigbee`


## Flashing from a custom bluetooth firmware (easy way for new devices)
1. Open an awesome tool from ATC_MiThermometer https://pvvx.github.io/ATC_MiThermometer/TelinkMiFlasher.html
2. Click "Connect" button and find device LYWSD03MMC, wait connection (Connected in logs)
3. On new device with stock firmware click "Do Activation" and wait some time
4. Next "Select Firmware", choose file with custom bluetooth firmware [ATC_ota_400000](./assets/ATC_ota_40000.bin), click "Start Flashing"
5. You will see in logs "Update done after NN seconds"
6. Reboot device removing battery, refresh web-page with flashing tool and connect device again (with name ATC_802190 or similar, based on mac-address). Do not change any control on page
8. Flash latest [z03mmc.bin](https://github.com/devbis/z03mmc/releases) firmware over transitional firmware to convert it to zigbee. Use https://devbis.github.io/telink-zigbee/ page if previous flasher stops because of the firmware size.

## Flashing firmware with USB to UART

### Prerequisites: 
1. TTL-USB adaptor
2. 1k-1.8k Ohm resistor
3. python3 with pyserial module installed


To flash a new firmware via an standard USB to UART adapter, simply connect the Thermometer as seen in the picture [Mi_SWS_Connection.jpg](./assets/Mi_SWS_Connection.jpg) to the USB to UART converter and run the TLSR825xComFlasher.py tool.

Example: `python3 TLSR825xComFlasher.py -p COM3 wf 0 z03mmc.bin`

Example: `python3 TLSR825xComFlasher.py -p /dev/ttyUSB0 wf 0 z03mmc.bin`

In case if the SWS pin is used by the firmware, try this sequence:
1. Power off the sensor
2. `python3 TLSR825xComFlasher.py -p <YOUR_COM_PORT> -t5000 wf 0 z03mmc.bin`
3. Now you have 5 seconds to power on the sensor
4. In case the chip has not started being flashed, run `python3 TLSR825xComFlasher.py -p <YOUR_COM_PORT> wf 0 z03mmc.bin` without the timeout again.

If the flashing fails try to increase timeouts in the script.

The UART flasher software uses the tool from https://github.com/pvvx/ATC_MiThermometer. Thanks to pvvx for the awesome work on this!

## Related Work
z03mmc is based on the original work of @pvvx, and @atc1441, who developed the initial firmware versions for bluetooth-capable device.
- https://github.com/pvvx/ATC_MiThermometer
- https://github.com/atc1441/ATC_MiThermometer

## Usage

1. Flash the firmware
2. Enable pairing mode on Zigbee coordinator
3. In case it is not joining, close the RESET and GND contacts on the board for 3 seconds to reset Zigbee settings. Replug the battery may require
4. For zigbee2mqtt you need to add custom converter if you use version 1.33.1 or earlier

## License

This project is licensed under the GNU General Public License 3.0 or later - see the [LICENSE.txt](LICENSE.txt) file for details.
