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

#include "tx_api.h"
#include "fx_api.h"
#include "board_init.h"
#include "sensor.h"
#include <stdio.h>

#define DEMO_STACK_SIZE         2048
#define DEMO_BYTE_POOL_SIZE     20480  /* Increased byte pool size to accommodate FileX memory allocations */
#define SENSOR_QUEUE_MAX_MSGS   10
#define SENSOR_MSG_WORDS        3  /* 12 bytes = 3 ULONGs */
#define RAM_DISK_SIZE           32768  /* 32 KB RAM Disk */
#define RAM_DISK_SECTOR_SIZE    512

/* Thread X objects */
TX_THREAD               sensor_thread;
TX_THREAD               logger_thread;
TX_QUEUE                sensor_queue;
TX_BYTE_POOL            byte_pool_sensor;

/* FileX objects */
FX_MEDIA                ram_disk;
FX_FILE                 log_file;

/* Memory area for byte pool */
static uint8_t pool_memory[DEMO_BYTE_POOL_SIZE];

/* Statically allocated memory for RAM disk and sector cache */
static uint8_t ram_disk_memory[RAM_DISK_SIZE];
static uint8_t cache_buffer[RAM_DISK_SECTOR_SIZE];

/* External FileX RAM driver prototype */
VOID _fx_ram_driver(FX_MEDIA *media_ptr);

/* Thread entry prototypes */
static void sensor_thread_entry(ULONG thread_input);
static void logger_thread_entry(ULONG thread_input);

int main(void)
{
    /* Initialize Board Support Package (BSP) - clock, GPIO, UART, I2C, etc. */
    board_init();

    printf("\r\n==================================================\r\n");
    printf("  Network Environmental Station Demo - Phase 2   \r\n");
    printf("==================================================\r\n");

    /* Enter the ThreadX kernel */
    tx_kernel_enter();

    return 0;
}

void tx_application_define(void *first_unused_memory)
{
    CHAR *pointer;

    /* Create a byte memory pool from which to allocate the thread stacks and queues */
    if (tx_byte_pool_create(&byte_pool_sensor, "sensor_pool", pool_memory, DEMO_BYTE_POOL_SIZE) != TX_SUCCESS)
    {
        printf("[System] Error: Failed to create sensor byte pool\r\n");
        Error_Handler();
    }

    /* Allocate the message queue memory */
    if (tx_byte_allocate(&byte_pool_sensor, (VOID **) &pointer, 
                         SENSOR_QUEUE_MAX_MSGS * SENSOR_MSG_WORDS * sizeof(ULONG), TX_NO_WAIT) != TX_SUCCESS)
    {
        printf("[System] Error: Failed to allocate queue memory\r\n");
        Error_Handler();
    }

    /* Create the sensor message queue */
    if (tx_queue_create(&sensor_queue, "sensor_queue", SENSOR_MSG_WORDS, pointer, 
                        SENSOR_QUEUE_MAX_MSGS * SENSOR_MSG_WORDS * sizeof(ULONG)) != TX_SUCCESS)
    {
        printf("[System] Error: Failed to create sensor message queue\r\n");
        Error_Handler();
    }

    /* Allocate the stack for the sensor thread */
    if (tx_byte_allocate(&byte_pool_sensor, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
    {
        printf("[System] Error: Failed to allocate sensor thread stack\r\n");
        Error_Handler();
    }

    /* Create the sensor thread */
    if (tx_thread_create(&sensor_thread, "sensor_thread", sensor_thread_entry, 0,  
                         pointer, DEMO_STACK_SIZE, 
                         3, 3, TX_NO_TIME_SLICE, TX_AUTO_START) != TX_SUCCESS)
    {
        printf("[System] Error: Failed to create sensor thread\r\n");
        Error_Handler();
    }

    /* Allocate the stack for the logger thread */
    if (tx_byte_allocate(&byte_pool_sensor, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
    {
        printf("[System] Error: Failed to allocate logger thread stack\r\n");
        Error_Handler();
    }

    /* Create the logger thread (Priority 5, lower priority than sensor_thread) */
    if (tx_thread_create(&logger_thread, "logger_thread", logger_thread_entry, 0,  
                         pointer, DEMO_STACK_SIZE, 
                         5, 5, TX_NO_TIME_SLICE, TX_AUTO_START) != TX_SUCCESS)
    {
        printf("[System] Error: Failed to create logger thread\r\n");
        Error_Handler();
    }

    printf("[System] ThreadX and FileX objects created successfully.\r\n");
}

static void sensor_thread_entry(ULONG thread_input)
{
    (void)thread_input;
    float temp = 0.0f;
    float hum = 0.0f;
    sensor_msg_t msg;

    /* Initialize sensor hardware (or fallback to mock) */
    if (sensor_init() != SENSOR_OK)
    {
        printf("[Sensor Thread] Critical Error: Sensor initialization failed!\r\n");
        Error_Handler();
    }

    printf("[Sensor Thread] Started polling loop (every 2 seconds).\r\n");

    while (1)
    {
        /* Read sensor data */
        if (sensor_read(&temp, &hum) == SENSOR_OK)
        {
            /* Pack the message */
            msg.temperature = temp;
            msg.humidity = hum;
            msg.timestamp = HAL_GetTick();

            /* Send the message to the queue */
            if (tx_queue_send(&sensor_queue, &msg, TX_NO_WAIT) == TX_SUCCESS)
            {
                float temp_abs = (temp < 0.0f) ? -temp : temp;
                int temp_dec = (int)temp_abs;
                int temp_frac = (int)((temp_abs - (float)temp_dec) * 100.0f);
                
                int hum_dec = (int)hum;
                int hum_frac = (int)((hum - (float)hum_dec) * 100.0f);

                printf("[Sensor Thread] Temp: %s%d.%02d C, Hum: %d.%02d %%, Time: %lu ms (Sent to Queue)\r\n", 
                       (temp < 0.0f) ? "-" : "", temp_dec, temp_frac, hum_dec, hum_frac, (unsigned long)msg.timestamp);
            }
            else
            {
                printf("[Sensor Thread] WARNING: Queue is full, message dropped.\r\n");
            }
        }
        else
        {
            printf("[Sensor Thread] Error: Failed to read sensor data.\r\n");
        }

        /* Sleep for 2 seconds (assuming 100 ticks per second) */
        tx_thread_sleep(2 * TX_TIMER_TICKS_PER_SECOND);
    }
}

static void logger_thread_entry(ULONG thread_input)
{
    (void)thread_input;
    UINT status;
    sensor_msg_t msg;
    char write_buffer[64];

    printf("[Logger Thread] Starting FileX RAM Disk Logger...\r\n");

    /* Initialize FileX system */
    fx_system_initialize();

    /* Format the RAM disk. This is required before opening it. */
    status = fx_media_format(&ram_disk, 
                             _fx_ram_driver, 
                             ram_disk_memory, 
                             cache_buffer, 
                             sizeof(cache_buffer), 
                             "RAM DISK", 
                             1, 
                             32, 
                             0, 
                             RAM_DISK_SIZE / RAM_DISK_SECTOR_SIZE, 
                             RAM_DISK_SECTOR_SIZE, 
                             1, 
                             1, 
                             1);
    if (status != FX_SUCCESS)
    {
        printf("[Logger Thread] Error: RAM Disk format failed! (0x%02X)\r\n", status);
        Error_Handler();
    }

    /* Open the RAM Disk */
    status = fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, 
                           cache_buffer, sizeof(cache_buffer));
    if (status != FX_SUCCESS)
    {
        printf("[Logger Thread] Error: RAM Disk open failed! (0x%02X)\r\n", status);
        Error_Handler();
    }

    /* Create the log file */
    status = fx_file_create(&ram_disk, "sensor_log.txt");
    if (status != FX_SUCCESS && status != FX_ALREADY_CREATED)
    {
        printf("[Logger Thread] Error: Failed to create log file! (0x%02X)\r\n", status);
        Error_Handler();
    }

    /* Open the log file for writing */
    status = fx_file_open(&ram_disk, &log_file, "sensor_log.txt", FX_OPEN_FOR_WRITE);
    if (status != FX_SUCCESS)
    {
        printf("[Logger Thread] Error: Failed to open log file! (0x%02X)\r\n", status);
        Error_Handler();
    }

    /* Move the write pointer to the end of the file to append new data */
    fx_file_seek(&log_file, log_file.fx_file_current_file_size);

    printf("[Logger Thread] RAM Disk initialized. Logging 'sensor_log.txt'...\r\n");

    while (1)
    {
        /* Retrieve the next sensor reading from the queue (blocks until a message is available) */
        if (tx_queue_receive(&sensor_queue, &msg, TX_WAIT_FOREVER) == TX_SUCCESS)
        {
            /* Format the sensor reading as a CSV line: timestamp,temp,hum */
            float temp_abs = (msg.temperature < 0.0f) ? -msg.temperature : msg.temperature;
            int temp_dec = (int)temp_abs;
            int temp_frac = (int)((temp_abs - (float)temp_dec) * 100.0f);
            
            int hum_dec = (int)msg.humidity;
            int hum_frac = (int)((msg.humidity - (float)hum_dec) * 100.0f);

            int len = snprintf(write_buffer, sizeof(write_buffer), "%lu,%s%d.%02d,%d.%02d\n",
                               (unsigned long)msg.timestamp,
                               (msg.temperature < 0.0f) ? "-" : "", temp_dec, temp_frac,
                               hum_dec, hum_frac);

            if (len > 0)
            {
                /* Write the CSV line to the file */
                status = fx_file_write(&log_file, write_buffer, (ULONG)len);
                if (status == FX_SUCCESS)
                {
                    /* Flush the media to commit changes to the RAM Disk immediately */
                    fx_media_flush(&ram_disk);
                    
                    printf("[Logger Thread] Logged to RAM Disk: %lu,%s%d.%02d,%d.%02d\r\n",
                           (unsigned long)msg.timestamp,
                           (msg.temperature < 0.0f) ? "-" : "", temp_dec, temp_frac,
                           hum_dec, hum_frac);
                }
                else
                {
                    printf("[Logger Thread] Error: Failed to write to log file! (0x%02X)\r\n", status);
                }
            }
        }
    }
}
