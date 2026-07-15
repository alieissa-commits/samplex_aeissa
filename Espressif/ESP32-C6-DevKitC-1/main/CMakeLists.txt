#  Copyright (c) 2026 Eclipse ThreadX contributors
#
#  This program and the accompanying materials are made available
#  under the terms of the MIT license which is available at
#  https://opensource.org/license/mit.
#
#  SPDX-License-Identifier: MIT
#
#  Contributors:
#     Ali Eissa - 2026 version.

# Select the active demo to build (default: threadx_basic)
set(ACTIVE_DEMO "threadx_basic" CACHE STRING "Active demo name to build: threadx_basic or telemetry_edge_node")

# Common Board Abstraction sources
set(srcs
    "board_init.c"
    "tx_initialize_low_level.S"
)

# Append demo-specific sources dynamically
if(ACTIVE_DEMO STREQUAL "threadx_basic")
    list(APPEND srcs "demos/threadx_basic/main.c")
elseif(ACTIVE_DEMO STREQUAL "telemetry_edge_node")
    list(APPEND srcs 
        "demos/telemetry_edge_node/main.c"
        "demos/telemetry_edge_node/mock_sensor.c"
    )
endif()

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS 
        "."
        "demos/${ACTIVE_DEMO}"
    REQUIRES 
        threadx
        esp_timer
)
