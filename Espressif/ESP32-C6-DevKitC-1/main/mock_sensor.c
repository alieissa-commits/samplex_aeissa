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

#include "sensor.h"
#include "esp_timer.h"
#include <math.h>

/* Initialize mock sensor */
sensor_status_t sensor_init(void)
{
    /* No hardware initialization needed for simulated sensor */
    return SENSOR_OK;
}

/* Read simulated sensor data using sine/cosine waves over system time */
sensor_status_t sensor_read(float *temp, float *hum)
{
    int64_t uptime_us = esp_timer_get_time();
    float uptime_sec = (float)uptime_us / 1000000.0f;

    /* Base temperature: 22.0°C, variation: ±3.0°C */
    float sim_temp = 22.0f + 3.0f * sinf(uptime_sec / 10.0f);
    
    /* Base humidity: 55.0%, variation: ±10.0% */
    float sim_hum = 55.0f + 10.0f * cosf(uptime_sec / 15.0f);

    /* Add tiny pseudo-random noise based on microseconds */
    float noise = (float)(uptime_us % 100) / 500.0f - 0.1f; /* Range: -0.1 to +0.1 */
    sim_temp += noise;
    sim_hum += noise * 5.0f;

    /* Clamp humidity to valid physical boundaries */
    if (sim_hum < 0.0f) sim_hum = 0.0f;
    if (sim_hum > 100.0f) sim_hum = 100.0f;

    *temp = sim_temp;
    *hum = sim_hum;

    return SENSOR_OK;
}
