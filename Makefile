CROSS_COMPILE ?= ./tools/xtensa-hifi4-gcc/bin/xtensa-hifi4-elf-

AS	  := $(CROSS_COMPILE)gcc -x assembler-with-cpp
CC	  := $(CROSS_COMPILE)gcc
CXX	  := $(CROSS_COMPILE)g++
LD	  := $(CROSS_COMPILE)ld
AR	  := $(CROSS_COMPILE)ar
OC	  := $(CROSS_COMPILE)objcopy
OD	  := $(CROSS_COMPILE)objdump
SIZE  := $(CROSS_COMPILE)size
RE    := $(CROSS_COMPILE)readelf
SP    := $(CROSS_COMPILE)strip
MKDIR := mkdir -p
CP	  := cp -af
RM	  := rm -fr
CD	  := cd
FIND  := find
Q     := @

DEPFLAGS = -MT $@ -MD -MP -MF $(basename $@).d

APP_NAME = dsp
BUILDDIR = ./build
APP := $(BUILDDIR)/$(APP_NAME).elf

IFLAGS := -I ./include

# Added for OpenCentauri dsp0 port
IFLAGS += -I ./include/hal
IFLAGS += -I ./include/osal

IFLAGS += -I ./arch
IFLAGS += -I ./arch/include
IFLAGS += -I ./arch/sun8iw20/include
IFLAGS += -I ./src

# Added for OpenCentauri dsp0 port
IFLAGS += -I ./hal

IFLAGS += -I ./kernel/portable
IFLAGS += -I ./include/freertos
IFLAGS += -I ./include/freertos/private
IFLAGS += -I ./benchmark
IFLAGS += -I ./benchmark/coremark
IFLAGS += -I ./benchmark/coremark/xtensa

# Open-source FreeRTOS Default Defines
DFLAGS := -DXT_BOARD  -DXT_TIMER_INDEX=0 -DXT_USE_SWPRI -DSTANDALONE=1 
DFLAGS += -DXTUTIL_NO_OVERRIDE 
DFLAGS += -DMAIN_HAS_NOARGC
DFLAGS += -DPERFORMANCE_RUN=1 -DITERATIONS=23000

# Added for OpenCentauri dsp0 port
# Target Options
DFLAGS += -DCONFIG_ARCH_SUN8IW20 -DCONFIG_ARCH_PLATFORM="sun8iw20" -DCONFIG_EVB_PLATFORM
DFLAGS += -DCONFIG_EVB_PLATFORM -DCONFIG_CORE_DSP0 -DCONFIG_CORE_ID="dsp0" 
DFLAGS += -DCONFIG_LSP_NORMAL_START -DCONFIG_LSP_DIR="default"
DFLAGS += -DCONFIG_SOC_SUN20IW1 # Clock, think this is right. Not CONFIG_ARCH_SUN20IW2
# Kernel Options
DFLAGS += -DCONFIG_KERNEL_FREERTOS -DCONFIG_KERNEL_XTENSA_V1_7
DFLAGS += -DCONFIG_KERNEL_VERSION_DIR="FreeRTOS_xtensa_v1.7" -DCONFIG_MEMMANG_HEAP_4
DFLAGS += -DCONFIG_PORT_XCC_XTENSA -DCONFIG_PORT_ARCH_DIR="xtensa" -DCONFIG_PORT_XEA2
# Clock Devices
DFLAGS += -DCONFIG_DRIVERS_SUNXI_CLK
# CCMU Devices
DFLAGS += -DCONFIG_DRIVERS_CCMU -DCONFIG_DRIVERS_SUNXI_CCU
# UART Devices
DFLAGS += -DCONFIG_DRIVERS_UART
# Board related device drivers
DFLAGS += -DCONFIG_DRIVERS_BOARD
# HAL OSAL API
DFLAGS += -DCONFIG_HAL_OSAL
# AW Sound Compenents Only For Dsp
DFLAGS += -DCONFIG_COMPONENTS_AW_ALSA_RPAF
DFLAGS += -DCONFIG_COMPONENTS_AW_ALSA_RPAF_COMPONENT
DFLAGS += -DCONFIG_COMPONENTS_AW_ALSA_RPAF_REMOTE_AMP=0
DFLAGS += -DCONFIG_COMPONENTS_AW_ALSA_RPAF_READ_CH=1
DFLAGS += -DCONFIG_COMPONENTS_AW_ALSA_RPAF_WRITE_CH=1
DFLAGS += -DCONFIG_OEMHEAD
# Backtrace Support
DFLAGS += -DCONFIG_DEBUG_BACKTRACE
# Linux Debug Support
DFLAGS += -DCONFIG_PM_CLIENT_DSP_WAITI
# DSPFREQ option features
DFLAGS += -DCONFIG_AW_DSPFREQ
# dump reg option features
DFLAGS += -DCONFIG_AW_JTAG_DEBUG
DFLAGS += -DCONFIG_MEMTESTER
# Performance Testing
DFLAGS += -DCONFIG_AW_PERF_MEM_ACC
# Miscellaneous Support
DFLAGS += -DCONFIG_BITOPS_FLS
# Thirdparty components
DFLAGS += -DCONFIG_COMPONENTS_THIRDPARTY
# Xtensa components
DFLAGS += -DCONFIG_COMPONENTS_XTENSA
DFLAGS += -DCONFIG_COMPONENTS_XTENSA_LIBGLOSS
# FreeRTOS components
DFLAGS += -DCONFIG_COMPONENTS_FREERTOS
DFLAGS += -DCONFIG_COMPONENTS_FREERTOS_CLI
# Supported commands
DFLAGS += -DCONFIG_FREERTOS_CLI_CMD_MEM_RW
DFLAGS += -DCONFIG_FREERTOS_CLI_CMD_FREE
# Algorithm common
DFLAGS += -DCONFIG_COMPONENTS_ALGO_COMMON
DFLAGS += -DCONFIG_COMPONENTS_ALGO_GENERATE
DFLAGS += -DCONFIG_COMPONENTS_MSGBOX_DEMO
# Projects options
DFLAGS += -DCONFIG_PROJECT_R528
DFLAGS += -DCONFIG_PROJECT_DIR="r528"

CFLAGS  := -Wa,--longcalls -static -O2  -Wall -mtext-section-literals  -fno-inline-functions
CFLAGS  += -ffunction-sections -fdata-sections  -mlongcalls  $(DFLAGS) $(IFLAGS)
CFLAGS  += -std=c99 -Wextra -Wa,--no-generate-flix -mtarget-align 
CFLAGS  += -Wno-unused-variable -Wno-unused-function

LDFLAGS := -nostdlib -Wl,--gc-sections -Wl,--defsym=__prefctl_default=0x144 
LDFLAGS += -Wl,--defsym=__memctl_default_post=1
LDFLAGS += -Wl,--abi-windowed
LDFLAGS += -Wl,-Map,$(BUILDDIR)/dsp.map
LDFLAGS += -Wl,--script link.ld

LIBS =  -L ./lib/  -lxtutil  -lhandler-reset -lc -lgloss -lhal -lm -lgcc -lc

# Added for OpenCentauri dsp0 port
APP_SRC := src/main

APP_SRC += src/klipper_r528/bus/msgboxx
APP_SRC += src/klipper_r528/bus/share_space
APP_SRC += src/klipper_r528/bus/usb_bulk
APP_SRC += src/klipper_r528/bus/uart
APP_SRC += src/klipper_r528/hal_call/adccmds
APP_SRC += src/klipper_r528/hal_call/endstop
APP_SRC += src/klipper_r528/hal_call/gpiocmds
APP_SRC += src/klipper_r528/hal_call/i2ccmds
APP_SRC += src/klipper_r528/hal_call/initial_pins
APP_SRC += src/klipper_r528/hal_call/pwmcmds
APP_SRC += src/klipper_r528/hal_call/sensor_adxl345
APP_SRC += src/klipper_r528/hal_call/spi_software
APP_SRC += src/klipper_r528/hal_call/spicmds
APP_SRC += src/klipper_r528/hal_call/thermocouple
APP_SRC += src/klipper_r528/hal_call/tmcuart

APP_SRC += src/klipper_r528/printer/basecmd
APP_SRC += src/klipper_r528/printer/command
APP_SRC += src/klipper_r528/printer/compile_time_request
APP_SRC += src/klipper_r528/printer/debugcmds
APP_SRC += src/klipper_r528/printer/pulse_counter
APP_SRC += src/klipper_r528/printer/scheder
APP_SRC += src/klipper_r528/printer/stepper
APP_SRC += src/klipper_r528/printer/trsync
APP_SRC += src/klipper_r528/printer/neopixel
APP_SRC += src/klipper_r528/ui/buttons
APP_SRC += src/klipper_r528/ui/lcd_hd44780
APP_SRC += src/klipper_r528/ui/lcd_st7920

APP_SRC += src/klipper_r528/generic/alloc
APP_SRC += src/klipper_r528/generic/armcm_reset
APP_SRC += src/klipper_r528/generic/armcm_timer
APP_SRC += src/klipper_r528/generic/armcm_irq
APP_SRC += src/klipper_r528/generic/crc16_ccitt
APP_SRC += src/klipper_r528/generic/usb_cdc

APP_SRC += src/klipper_r528/board/adc
APP_SRC += src/klipper_r528/board/chipid
APP_SRC += src/klipper_r528/board/gpio
APP_SRC += src/klipper_r528/board/hard_pwm
APP_SRC += src/klipper_r528/board/i2c

APP_SRC += src/klipper_r528/board/spi
APP_SRC += src/klipper_r528/board/watchdog

# Added for OpenCentauri dsp0 port
HAL_SRC :=
# RTC HAL Objects
HAL_SRC += hal/rtc/hal_rtc
HAL_SRC += hal/rtc/rtc-lib
# GPIO HAL Objects
HAL_SRC += hal/gpio/hal_gpio
HAL_SRC += hal/gpio/sun8iw20/gpio-sun8iw20
# CCMU HAL Objects (Clock)
HAL_SRC += hal/ccmu/hal_clk
HAL_SRC += hal/ccmu/hal_reset
# ONLY include these if -DCONFIG_SOC_SUN20IW1, which I just added
HAL_SRC += hal/ccmu/sunxi-ng/ccu-sun8iw20
HAL_SRC += hal/ccmu/sunxi-ng/ccu-sun8iw20-r
HAL_SRC += hal/ccmu/sunxi-ng/ccu-sun8iw20-rtc
# General Purpose ADC HAL Objects
HAL_SRC += hal/gpadc/hal_gpadc
# Timer HAL Objects
HAL_SRC += hal/timer/hal_htimer
HAL_SRC += hal/timer/sunxi_htimer
# MsgBox HAL Objects
#HAL_SRC += hal/msgbox/msgbox_amp/msgbox_amp
HAL_SRC += hal/msgbox/msgbox_sx/hal_msgbox_sx
HAL_SRC += hal/msgbox/msgbox_sx/msgbox_sx
HAL_SRC += hal/msgbox/msgbox_sx/msgbox_adapt
# MORE TO DO HERE CLOCK SUBDIRS BUT SKIPPING FOR NOW

BENCHMARK_SRC := benchmark/linpack-pc
BENCHMARK_SRC += benchmark/dhry_1
BENCHMARK_SRC += benchmark/dhry_2
BENCHMARK_SRC += benchmark/coremark/core_list_join
BENCHMARK_SRC += benchmark/coremark/core_main
BENCHMARK_SRC += benchmark/coremark/core_matrix
BENCHMARK_SRC += benchmark/coremark/core_state
BENCHMARK_SRC += benchmark/coremark/core_util
BENCHMARK_SRC += benchmark/coremark/xtensa/core_portme

STARTUP_SRC += arch/crt1
STARTUP_SRC += arch/intlevel-set
STARTUP_SRC += arch/board-init
STARTUP_SRC += arch/rtos-help
#Kconfig*  Makefile  cpufreq.c  cpufreq.h  include/  init-sun8iw20.c*  lsp/  mmu.c  orig/
#STARTUP_SRC += arch/init-sun8iw20
#STARTUP_SRC += arch/cpufreq
#STARTUP_SRC += arch/mmu

OEM_SRC += oemhead/oemhead

KERNEL_SRC := kernel/FreeRTOS/event_groups
KERNEL_SRC += kernel/FreeRTOS/list
KERNEL_SRC += kernel/FreeRTOS/queue
KERNEL_SRC += kernel/FreeRTOS/stream_buffer
KERNEL_SRC += kernel/FreeRTOS/tasks
KERNEL_SRC += kernel/FreeRTOS/timers
KERNEL_SRC += kernel/MemMang/heap_4
KERNEL_SRC += kernel/portable/nmi-handler
KERNEL_SRC += kernel/portable/port
KERNEL_SRC += kernel/portable/portasm
KERNEL_SRC += kernel/portable/portclib
KERNEL_SRC += kernel/portable/xtensa_init
KERNEL_SRC += kernel/portable/xtensa_context
KERNEL_SRC += kernel/portable/xtensa_intr
KERNEL_SRC += kernel/portable/xtensa_intr_asm
KERNEL_SRC += kernel/portable/xtensa_overlay_os_hook
KERNEL_SRC += kernel/portable/xtensa_vectors

SRC := $(APP_SRC)
SRC += $(HAL_SRC)
SRC += $(OEM_SRC)
SRC += $(STARTUP_SRC)
SRC += $(KERNEL_SRC)
SRC += $(BENCHMARK_SRC)

LIB_PIECES = $(SRC)

LIB_OBJS = $(LIB_PIECES:%=$(BUILDDIR)/%.o)
LIB_DEPS = $(LIB_PIECES:%=$(BUILDDIR)/%.d)
LIB_OBJS += lib/crti.o
LIB_OBJS += lib/crtbegin.o
LIB_OBJS += lib/crtend.o
LIB_OBJS += lib/crtn.o

all: clean builddir $(APP)

builddir:
	$(Q)$(MKDIR) $(BUILDDIR)
	$(Q)$(MKDIR) $(BUILDDIR)/src/klipper_r528/board
	$(Q)$(MKDIR) $(BUILDDIR)/src/klipper_r528/bus
	$(Q)$(MKDIR) $(BUILDDIR)/src/klipper_r528/generic
	$(Q)$(MKDIR) $(BUILDDIR)/src/klipper_r528/hal_call
	$(Q)$(MKDIR) $(BUILDDIR)/src/klipper_r528/printer
	$(Q)$(MKDIR) $(BUILDDIR)/src/klipper_r528/ui
	$(Q)$(MKDIR) $(BUILDDIR)/hal/gpio/sun8iw20
	$(Q)$(MKDIR) $(BUILDDIR)/hal/rtc
	$(Q)$(MKDIR) $(BUILDDIR)/hal/ccmu
	$(Q)$(MKDIR) $(BUILDDIR)/hal/ccmu/sunxi-ng
	$(Q)$(MKDIR) $(BUILDDIR)/hal/gpadc
	$(Q)$(MKDIR) $(BUILDDIR)/hal/timer
	$(Q)$(MKDIR) $(BUILDDIR)/hal/msgbox/msgbox_sx
	$(Q)$(MKDIR) $(BUILDDIR)/arch
	$(Q)$(MKDIR) $(BUILDDIR)/oemhead
	$(Q)$(MKDIR) $(BUILDDIR)/kernel/FreeRTOS
	$(Q)$(MKDIR) $(BUILDDIR)/kernel/portable
	$(Q)$(MKDIR) $(BUILDDIR)/kernel/MemMang
	$(Q)$(MKDIR) $(BUILDDIR)/application
	$(Q)$(MKDIR) $(BUILDDIR)/benchmark
	$(Q)$(MKDIR) $(BUILDDIR)/benchmark/coremark
	$(Q)$(MKDIR) $(BUILDDIR)/benchmark/coremark/xtensa
	$(Q)$(MKDIR) $(BUILDDIR)/output

$(APP): include/version.h $(LIB_OBJS)
	$(Q)echo [LD] LINKING $@
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS) $(LIB_OBJS) $(LIBS) -o $(APP)
	$(Q)echo [SP] STRIP $@
	$(Q)$(SP) -s $(APP)
	$(Q)echo [OD] OBJDUMP $@
	$(Q)$(OD) -D $(APP) > $(BUILDDIR)/$(APP_NAME).dis
	$(Q)echo [RE] READELF $@
	$(Q)$(RE) -a $(APP) > $(BUILDDIR)/$(APP_NAME).readelf
	$(Q)echo -e '\033[0;31;1m'
	$(Q)$(SIZE) $(APP)
	$(Q)echo -e '\033[0;32;1m'
	$(Q)echo Build $(APP) Successful
	$(Q)echo -e '\033[0m'

$(BUILDDIR)/%.o: %.S
	$(Q)echo [AS] $<
	$(Q)$(CC) -c $(CFLAGS) -o $@ $<

$(BUILDDIR)/%.o: %.c
	$(Q)echo [CC] $<
	$(Q)$(CC) -c $(CFLAGS) -o $@ $<

install:
	scp build/dsp.elf carbon2:/mnt/exUDISK/

clean:
	$(Q)rm -rf $(BUILDDIR)

#create head file
include/version.h:
	$(Q)$(call filechk,version)

define filechk
	$(Q)set -e;                             \
	echo '  CHK     $@';                    \
	mkdir -p $(dir $@);                     \
	$(filechk_$(1)) > $@.tmp;               \
	if [ -r $@ ] && cmp -s $@ $@.tmp; then  \
		rm -f $@.tmp;                       \
	else                                    \
		echo '  UPD     $@';                \
		mv -f $@.tmp $@;                    \
	fi
endef

define filechk_version
(echo "/*";\
	echo " * THIS IS CREATE WITH VERSION CHK";\
	echo " * DO NOT CHANGE IT MANUAL";\
	echo " */";\
	echo ;\
	echo "#ifndef _SUB_VER_";\
	echo "#define _SUB_VER_";\
	echo ;\
	echo "#define SUB_VER \"`scripts/setlocalversion`\"";\
	echo ;\
	echo "#endif /* _SUB_VER_ */";)
endef
