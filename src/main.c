#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// #include "FreeRTOS.h"
// #include "task.h"

#include <console.h>

#include <aw-alsa-lib/common.h>
#include <platform.h>
#include <components/aw/linux_debug/debug_common.h>

#include "include/bus/share_space.h"
#include "include/printer/scheder.h"

// source ./build/envsetup.sh
// ./build.sh
int main(void)   //--G-G-2022-08-03---
{
#ifdef CONFIG_PM_CLIENT_DSP_WAITI
	// pm_standby_service_init();
#endif

	sched_main();
	printf( "sched_main FAILED!\n" );

	while(1);
	return 1;
}


