# OpenCentauri FreeRTOS DSP Firmware

This repository contains a modified version of [FreeRTOS-HIFI4-DSP](https://github.com/YuzukiHD/FreeRTOS-HIFI4-DSP) adapted for the OpenCentauri project, specifically for the r528 SoC.

## How to Use

### Prerequisites

1.  OpenCentauri v0.0.4 or later.
2.  Serial UART connection at 115200 baud.
3.  Install the [compiler](https://github.com/YuzukiHD/FreeRTOS-HIFI4-DSP/releases/download/Toolchains/xtensa-hifi4-dsp.tar.gz) to `tools/xtensa-hifi4-gcc/`.

### Method 1: Serial Console Deployment

1.  Compile the firmware:
    ```bash
    make
    ```
2.  Copy the compiled firmware to your printer's USB stick:
    ```bash
    scp build/dsp.elf carbon:/boot/exUDISK/
    ```
3.  Set a boot delay in uBoot (via SSH on the mainboard):
    ```bash
    fw_setenv bootdelay 10
    ```
4.  Add a uBoot command to start the new DSP firmware from the USB stick:
    ```bash
    fw_setenv dsp_oc 'usb start;usb dev 0;fatload usb 0:1 43000000 dsp.elf;bootr 43000000 0 0'
    ```
5.  Reboot your Centauri, interrupt the boot process when prompted, and enter:
    ```
    run dsp_oc
    ```
6.  To boot into Linux afterward, enter:
    ```
    run setargs_mmc boot_normal
    ```

### Method 2: Alternate Deployment (No Serial Console)

This method sets the custom DSP firmware as the default, booting from the external USB drive.

1.  Build `dsp.elf` using the instructions above and copy it to `/mnt/exUDISK`.
2.  Ensure `/mnt/exUDISK` is a `vfat` partition mounted from `/dev/sda1`. You can verify with:
    ```bash
    mount | grep sda1
    # Expected output:
    # /dev/sda1 on /mnt/exUDISK type vfat (rw,relatime,fmask=0022,dmask=0022,codepage=437,iocharset=utf8,shortname=mixed,utf8,errors=remount-ro)
    ```
3.  **Backup your current uBoot settings!**
    ```bash
    fw_printenv > uboot_backup.txt
    ```
4.  Run these commands to change the default boot process:
    ```bash
    fw_setenv bootdelay 4
    fw_setenv dsp_oc 'usb start;usb dev 0;fatload usb 0:1 43000000 dsp.elf;bootr 43000000 0 0;sleep 2'
    fw_setenv bootcmd "run setargs_mmc dsp_oc boot_normal"
    ```
5.  Reboot. The device should now boot using `dsp.elf` from the USB drive.

#### Rollback

-   If the device doesn't boot, you can copy a stock `dsp.elf` to the USB drive and reboot.
-   To restore the default boot from eMMC, run:
    ```bash
    fw_setenv bootcmd "run setargs_mmc boot_dsp0 boot_normal"
    ```

## Todo

- [ ] Port Klipper MCU firmware to this FreeRTOS environment.
- [ ] Enable GPIO drivers if they don't already exist/work (examples in upstream Allwinner sources).
- [ ] Establish communication between the DSP and the Linux Processing System, possibly via the MsgBox API.

---

# Original FreeRTOS-HIFI4-DSP

> ⚠ **NOTE:** This is not an official RTOS SDK or Compiler and does **NOT** support all DSP features.

This is FreeRTOS for the Cadence Tensilica HIFI 4 DSP, using the GCC Compiler.

## About the Compiler

The Cadence Tensilica HIFI 4 typically uses Xtensa Xplorer and XCC for development. This project uses GCC instead.

## How to Build

1.  Clone the repository:
    ```bash
    git clone https://github.com/YuzukiHD/FreeRTOS-HIFI4-DSP.git
    ```
2.  Download the [Compiler](https://github.com/YuzukiHD/FreeRTOS-HIFI4-DSP/releases/download/Toolchains/xtensa-hifi4-dsp.tar.gz) and unzip it into the `tools` directory.
3.  Run the make command:
    ```bash
    make
    ```

## Firmware Loader

### SyterKit

SyterKit provides a hifi4 loader: [https://github.com/YuzukiHD/SyterKit/tree/main/board/100ask-d1-h/load_hifi4](https://github.com/YuzukiHD/SyterKit/tree/main/board/100ask-d1-h/load_hifi4)

### U-Boot Driver

A U-Boot driver is available to load firmware to the HIFI4 DSP: [uboot-driver/dsp](https://github.com/YuzukiHD/FreeRTOS-HIFI4-DSP/tree/master/host/uboot-driver/dsp)