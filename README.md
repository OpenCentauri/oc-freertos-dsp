# OC-FreeRTOS-DSP

Updated for the OpenCentauri project, had to make mods related to the r528 SoC to get it to run on our board!

Based on this [amazing work by YuzukiHD](https://github.com/YuzukiHD/FreeRTOS-HIFI4-DSP)!

## How to use:

1. Prerequisite: Be running OpenCentauri v0.0.4 or later, and have a serial UART hooked up at 115200!
1. Install the compiler linked below to tools/xtensa-hifi4-gcc/
1. Run `make` to compile
1. Copy to your printer USB stick: `scp build/dsp.elf carbon:/boot/exUDISK/`
1. Set a boot delay for your uBoot (SSH'd into CC mainboard): `fw_setenv bootdelay 10`
1. Add a uBoot command to start the new DSP firmware from your USB stick (fat32 partition 1):
    ```
    fw_setenv dsp_oc 'usb start;usb dev 0;fatload usb 0:1 43000000 dsp.elf;bootr 43000000 0 0'
    ```
1. Reboot your centauri, press the Any key to interrupt boot when prompted, and enter `run dsp_oc`
1. To boot Linux after that, enter `run setargs_mmc boot_normal`.

## Todo:
1. Port Klipper MCU firmware to this FreeRTOS environment
1. Possibly enable GPIO drivers if they don't exist/work (examples in upstream Allwinner sources)
1. Establish comms between the DSP and the Linux Processing System, possibly via MsgBox API

# Original FreeRTOS-HIFI4-DSP

> ⚠ NOT official RTOS SDK and Compiler, **NOT Support DSP feature**

FreeRTOS for Cadence Tensilica HIFI 4 DSP, With GCC Compiler

## About Compiler
Cadence Tensilica HIFI 4 use Xtensa Xplorer and XCC for development. This Project does not support XCC or Xtensa Xplorer, We use GCC for Compile.

## How to build

1. Clone codes

```
git clone https://github.com/YuzukiHD/FreeRTOS-HIFI4-DSP.git
```

2. Download [Compiler](https://github.com/YuzukiHD/FreeRTOS-HIFI4-DSP/releases/download/Toolchains/xtensa-hifi4-dsp.tar.gz) and unzip it to tools

3. `make`

## Firmware Loader

### SyterKit

SyterKit provide hifi4 loader: https://github.com/YuzukiHD/SyterKit/tree/main/board/100ask-d1-h/load_hifi4

Here is a U-Boot Driver to load firmware to HIFI4 DSP  
- https://github.com/YuzukiHD/FreeRTOS-HIFI4-DSP/tree/master/host/uboot-driver/dsp
