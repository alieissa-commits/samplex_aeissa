/*
 * Copyright (c) 2026 Eclipse ThreadX contributors
 *
 *  This program and the accompanying materials are made available
 *  under the terms of the MIT license which is available at
 *  https://opensource.org/license/mit.
 *
 *  SPDX-License-Identifier: MIT
 *
 *  Contributors:
 *     Ali Eissa - 2026 version.
 */

#include "board_init.h"
#include <stdint.h>
#include "tx_api.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "soc/soc.h"
#include "soc/assist_debug_reg.h"

#define TAG "BoardInit"

/* Extern declarations for ThreadX kernel variables and tick processor */
extern ULONG _tx_thread_system_state;
extern void _tx_timer_interrupt(void);

/* System tick timer callback */
static void system_tick_cb(void* arg)
{
    (void)arg;
    
    /* Increment nesting level to signal ISR execution to ThreadX */
    _tx_thread_system_state++;
    
    /* Call ThreadX core tick processor */
    _tx_timer_interrupt();
    
    /* Decrement nesting level */
    _tx_thread_system_state--;
}

/* Initialize low-level board hardware register configurations */
void board_init(void)
{
    /* Disable hardware stack pointer monitor bounds to allow stack switching */
    REG_WRITE(ASSIST_DEBUG_CORE_0_SP_MIN_REG, 0);
    REG_WRITE(ASSIST_DEBUG_CORE_0_SP_MAX_REG, 0xFFFFFFFF);
    ESP_LOGI(TAG, "Hardware stack bounds monitor disabled");
}

/* Initialize and start the periodic system tick timer callback */
void board_timer_setup(void)
{
    /* Configure and start high-resolution system tick timer (1ms interval = 1000Hz) */
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &system_tick_cb,
        .name = "threadx_tick"
    };
    esp_timer_handle_t periodic_timer;
    esp_timer_create(&periodic_timer_args, &periodic_timer);
    esp_timer_start_periodic(periodic_timer, 1000); // 1000 microseconds = 1 ms
    
    ESP_LOGI(TAG, "System tick timer initialized at 1ms resolution (1000Hz)");
}
