// Generic reset command handler for ARM Cortex-M boards
//
// Copyright (C) 2019  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

// #include "stm32/internal.h" // NVIC_SystemReset
#include "../../include/printer/command.h" // DECL_COMMAND_FLAGS

void command_reset(uint32_t *args)
{
	// struct reset_control *reset = hal_reset_control_get(HAL_SUNXI_RESET, RST_BUS_AUDIO_CODEC);
	// hal_reset_control_reset(reset);
	// hal_reset_control_put(reset);            
    // NVIC_SystemReset();          //--G-G-2022-08-03---
    // share_space_clear();
    // shutdown("reset");
_ResetHandler();     //--G-G-2022-08-09---
}
DECL_COMMAND_FLAGS(command_reset, HF_IN_SHUTDOWN, "reset");
