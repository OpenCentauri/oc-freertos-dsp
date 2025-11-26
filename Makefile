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
IFLAGS += -I ./include/xtensa
IFLAGS += -I ./arch
IFLAGS += -I ./arch/include
IFLAGS += -I ./src
IFLAGS += -I ./kernel/portable
IFLAGS += -I ./include/freertos
IFLAGS += -I ./include/freertos/private
IFLAGS += -I ./benchmark
IFLAGS += -I ./benchmark/coremark
IFLAGS += -I ./benchmark/coremark/xtensa
IFLAGS += -I ./rtos-hal/include
IFLAGS += -I ./rtos-hal/include/hal
IFLAGS += -I ./rtos-hal/include/osal
IFLAGS += -I ./rtos-hal/hal/source

DFLAGS := -DXT_BOARD -DXT_USE_SWPRI -DSTANDALONE=1 
DFLAGS := -DXT_TIMER_INDEX=0
DFLAGS += -DXTUTIL_NO_OVERRIDE 
DFLAGS += -DMAIN_HAS_NOARGC
DFLAGS += -DPERFORMANCE_RUN=1 -DITERATIONS=23000
# Enable use SW timers
#DFLAGS += -DconfigUSE_TIMERS
# RTOS HAL configuration
DFLAGS += -DCONFIG_KERNEL_FREERTOS
DFLAGS += -DCONFIG_CORE_DSP0
DFLAGS += -DCONFIG_ARCH_SUN8IW20

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

APP_SRC := src/main
APP_SRC += src/sharespace
APP_SRC += src/log
APP_SRC += src/gpio_toggle_example
#APP_SRC += src/hal_msgbox src/msgboxx src/share_space
#APP_SRC += src/rpmsg

# OSAL (Operating System Abstraction Layer) sources
OSAL_SRC := src/osal/hal_mem
OSAL_SRC += src/osal/hal_interrupt
OSAL_SRC += src/osal/hal_atomic
OSAL_SRC += src/osal/hal_cache
OSAL_SRC += src/osal/hal_sem
OSAL_SRC += src/osal/hal_mutex
OSAL_SRC += src/osal/hal_queue
OSAL_SRC += src/osal/hal_timer
OSAL_SRC += src/osal/hal_thread

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

# RTOS HAL source files
RTOS_HAL_SRC := rtos-hal/hal/source/common/dma_alloc
RTOS_HAL_SRC += rtos-hal/hal/source/gpio/hal_gpio
RTOS_HAL_SRC += rtos-hal/hal/source/gpio/sun8iw20/gpio-sun8iw20
# Add more HAL modules as needed:
# RTOS_HAL_SRC += rtos-hal/hal/source/msgbox/msgbox_sx/msgbox_sx
# RTOS_HAL_SRC += rtos-hal/hal/source/msgbox/msgbox_sx/msgbox_adapt
# RTOS_HAL_SRC += rtos-hal/hal/source/msgbox/msgbox_sx/hal_msgbox_sx
# RTOS_HAL_SRC += rtos-hal/hal/source/uart/hal_uart
# RTOS_HAL_SRC += rtos-hal/hal/source/timer/hal_timer

SRC := $(APP_SRC)
SRC += $(OEM_SRC)
SRC += $(STARTUP_SRC)
SRC += $(KERNEL_SRC)
SRC += $(BENCHMARK_SRC)
SRC += $(OSAL_SRC)
SRC += $(RTOS_HAL_SRC)

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
	$(Q)$(MKDIR) $(BUILDDIR)/src
	$(Q)$(MKDIR) $(BUILDDIR)/src/osal
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
	$(Q)$(MKDIR) $(BUILDDIR)/rtos-hal/hal/source/common
	$(Q)$(MKDIR) $(BUILDDIR)/rtos-hal/hal/source/msgbox/msgbox_sx
	$(Q)$(MKDIR) $(BUILDDIR)/rtos-hal/hal/source/uart
	$(Q)$(MKDIR) $(BUILDDIR)/rtos-hal/hal/source/gpio
	$(Q)$(MKDIR) $(BUILDDIR)/rtos-hal/hal/source/gpio/sun8iw20
	$(Q)$(MKDIR) $(BUILDDIR)/rtos-hal/hal/source/timer

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
	scp build/dsp.elf carbon-devu:/mnt/exUDISK/

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
