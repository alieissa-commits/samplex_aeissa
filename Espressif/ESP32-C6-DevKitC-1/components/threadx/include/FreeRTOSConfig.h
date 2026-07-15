/***************************************************************************
 * Copyright (c) 2024 Microsoft Corporation
 * Copyright (c) 2026-present Eclipse ThreadX contributors
 *
 * This program and the accompanying materials are made available under the
 * terms of the MIT License which is available at
 * https://opensource.org/licenses/MIT.
 *
 * SPDX-License-Identifier: MIT
 **************************************************************************/

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "tx_api.h"

#define configTICK_RATE_HZ                         (100U)
#define configMAX_PRIORITIES                       (32U)
#define configMINIMAL_STACK_SIZE                   (512U)
#define configTOTAL_HEAP_SIZE                      (1024U * 64U)
#define configUSE_16_BIT_TICKS                     0
#define configSTACK_DEPTH_TYPE                     uint32_t
#define INCLUDE_vTaskDelete                        1

/* Map FreeRTOS critical sections directly to ThreadX interrupt controls for RISC-V */
#define portDISABLE_INTERRUPTS()                   _tx_thread_interrupt_control(TX_INT_DISABLE)
#define portENABLE_INTERRUPTS()                    _tx_thread_interrupt_control(TX_INT_ENABLE)
#define configASSERT(x)
#define TX_FREERTOS_ASSERT_FAIL()
#define TX_FREERTOS_AUTO_INIT                      0

#endif /* FREERTOS_CONFIG_H */
