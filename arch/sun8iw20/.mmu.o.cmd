# cannot find fixdep (scripts/build/fixdep)
# using basic dep data

arch/sun8iw20/mmu.o: arch/sun8iw20/mmu.c \
 /home/paul/sandbox/carbon/eLinuxCore/rtos-dsp/freertos/include/generated/autoconf.h \
 include/map_func.h \
 /home/paul/sandbox/carbon/xtensa-hifi4-gcc/xtensa-hifi4-elf/sys-include/stdio.h \
 /home/paul/sandbox/carbon/xtensa-hifi4-gcc/xtensa-hifi4-elf/sys-include/_ansi.h \
 /home/paul/sandbox/carbon/xtensa-hifi4-gcc/xtensa-hifi4-elf/sys-include/newlib.h \
 /home/paul/sandbox/carbon/xtensa-hifi4-gcc/xtensa-hifi4-elf/sys-include/sys/config.h \
 /home/paul/sandbox/carbon/xtensa-hifi4-gcc/xtensa-hifi4-elf/sys-include/machine/ieeefp.h \
 /home/paul/sandbox/carbon/xtensa-hifi4-gcc/xtensa-hifi4-elf/sys-include/sys/features.h \
 /home/paul/sandbox/carbon/xtensa-hifi4-gcc/xtensa-hifi4-elf/sys-include/xtensa/config/core-isa.h \
 /home/paul/sandbox/carbon/xtensa-hifi4-gcc/xtensa-hifi4-elf/sys-include/sys/cdefs.h \
 /home/paul/sandbox/carbon/xtensa-hifi4-gcc/xtensa-hifi4-elf/sys-include/machine/_default_types.h \
 /home/paul/sandbox/carbon/xtensa-hifi4-gcc/lib/gcc/xtensa-hifi4-elf/10.3.0/include/stddef.h \
 /home/paul/sandbox/carbon/xtensa-hifi4-gcc/lib/gcc/xtensa-hifi4-elf/10.3.0/include/stdarg.h \
 /home/paul/sandbox/carbon/xtensa-hifi4-gcc/xtensa-hifi4-elf/sys-include/sys/reent.h \
 /home/paul/sandbox/carbon/xtensa-hifi4-gcc/xtensa-hifi4-elf/sys-include/_ansi.h \
 /home/paul/sandbox/carbon/xtensa-hifi4-gcc/xtensa-hifi4-elf/sys-include/sys/_types.h \
 /home/paul/sandbox/carbon/xtensa-hifi4-gcc/xtensa-hifi4-elf/sys-include/machine/_types.h \
 /home/paul/sandbox/carbon/xtensa-hifi4-gcc/xtensa-hifi4-elf/sys-include/sys/lock.h \
 /home/paul/sandbox/carbon/xtensa-hifi4-gcc/xtensa-hifi4-elf/sys-include/sys/types.h \
 /home/paul/sandbox/carbon/xtensa-hifi4-gcc/xtensa-hifi4-elf/sys-include/machine/types.h \
 /home/paul/sandbox/carbon/xtensa-hifi4-gcc/xtensa-hifi4-elf/sys-include/sys/stdio.h
cmd_arch/sun8iw20/mmu.o := /home/paul/sandbox/carbon/xtensa-hifi4-gcc/bin/xtensa-hifi4-elf-gcc -Wp,-MD,arch/sun8iw20/.mmu.o.d,-MT,arch/sun8iw20/mmu.o -"O2" -Os -g -Wall -ffunction-sections -fdata-sections -mlongcalls -DXT_BOARD -DSTANDALONE=1 -DXTUTIL_NO_OVERRIDE -Iprojects/"r528"/"dsp0"/src -Iprojects/"r528"/"dsp0"/include -Iinclude -Iinclude/hal -Iinclude/hal/sound -Iinclude/osal -Idrivers/hal/source -Icomponents/thirdparty/freertos -Ikernel/"FreeRTOS_xtensa_v1.7"/include -Ikernel/"FreeRTOS_xtensa_v1.7"/include/private -Ikernel/"FreeRTOS_xtensa_v1.7"/FreeRTOS/portable/XCC/Xtensa -Iarch/include -Iarch/"sun8iw20"/include/ -I/home/paul/sandbox/carbon/eLinuxCore/rtos-dsp/freertos/include/generated -include autoconf.h -include map_func.h -D"BUILD_STR(s)=$(pound)s"   -c -o arch/sun8iw20/mmu.o arch/sun8iw20/mmu.c 
