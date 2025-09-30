// Watchdog handler on STM32
//
// Copyright (C) 2019  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "../../include/printer/scheder.h" // DECL_TASK
#include "sunxi_hal_watchdog.h"

void
watchdog_reset(void)
{
    // hal_watchdog_reset(0);
}
DECL_TASK(watchdog_reset);

void
watchdog_init(void)
{
    // hal_watchdog_init();
}
DECL_INIT(watchdog_init);
