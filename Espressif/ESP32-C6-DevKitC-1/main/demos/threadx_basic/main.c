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

#include <stdint.h>
#include <stdlib.h>
#include "tx_api.h"
#include "esp_log.h"
#include "board_init.h"

#define TAG "System"

/* ThreadX resources */
#define BYTE_POOL_SIZE 16384
#define THREAD_STACK_SIZE 2048

static TX_THREAD thread_1;
static TX_THREAD thread_2;

static uint8_t byte_pool_memory[BYTE_POOL_SIZE];
TX_BYTE_POOL byte_pool;

/* Thread entry prototypes */
static void thread_1_entry(ULONG thread_input);
static void thread_2_entry(ULONG thread_input);

/* ESP-IDF entry point */
void app_main(void)
{
    /* Initialize Board Abstraction Layer (Assist Debug bypass, hardware config) */
    board_init();

    ESP_LOGI(TAG, "ESP32-C6 Booted! Bootstrapping ThreadX...");

    /* Start the ThreadX Kernel */
    tx_kernel_enter();
}

/* Define ThreadX kernel resources */
void tx_application_define(void *first_unused_memory)
{
    (void)first_unused_memory;
    CHAR *stack_1;
    CHAR *stack_2;

    ESP_LOGI(TAG, "Entering tx_application_define");

    /* Initialize and start the periodic system tick timer callback */
    board_timer_setup();

    /* Create ThreadX Memory Byte Pool */
    if (tx_byte_pool_create(&byte_pool, "Memory Pool", byte_pool_memory, BYTE_POOL_SIZE) != TX_SUCCESS)
    {
        ESP_LOGE(TAG, "Failed to create dynamic byte pool");
        return;
    }

    /* Allocate stacks from the byte pool */
    if (tx_byte_allocate(&byte_pool, (VOID**)&stack_1, THREAD_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
    {
        ESP_LOGE(TAG, "Failed to allocate stack for Thread 1");
        return;
    }

    if (tx_byte_allocate(&byte_pool, (VOID**)&stack_2, THREAD_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
    {
        ESP_LOGE(TAG, "Failed to allocate stack for Thread 2");
        return;
    }

    /* Create Thread 1 (higher priority) */
    tx_thread_create(&thread_1, "Thread 1", thread_1_entry, 0x11,
                     stack_1, THREAD_STACK_SIZE, 10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);

    /* Create Thread 2 (lower priority) */
    tx_thread_create(&thread_2, "Thread 2", thread_2_entry, 0x22,
                     stack_2, THREAD_STACK_SIZE, 15, 15, TX_NO_TIME_SLICE, TX_AUTO_START);

    ESP_LOGI(TAG, "ThreadX resources and scheduler successfully initialized");
}

/* Thread 1 Entry Point */
static void thread_1_entry(ULONG thread_input)
{
    ESP_LOGI("Thread 1", "Started (input = 0x%02lx)", thread_input);
    uint32_t count = 0;

    while (1)
    {
        count++;
        ESP_LOGI("Thread 1", "Ping %lu - Yielding context...", count);
        tx_thread_sleep(1000); /* Sleep 1000 ticks = 1 second */
    }
}

/* Thread 2 Entry Point */
static void thread_2_entry(ULONG thread_input)
{
    ESP_LOGI("Thread 2", "Started (input = 0x%02lx)", thread_input);
    uint32_t count = 0;

    while (1)
    {
        count++;
        ESP_LOGI("Thread 2", "Pong %lu - Yielding context...", count);
        tx_thread_sleep(1000); /* Sleep 1000 ticks = 1 second */
    }
}
