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

# CMakeLists.txt for ThreadX ESP-IDF Component
set(THREADX_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/threadx")
set(THREADX_PORT_DIR "${THREADX_ROOT}/ports/risc-v32/gnu")

# Glob all common C files (including txe_*.c and tx_*.c)
file(GLOB common_srcs "${THREADX_ROOT}/common/src/*.c")

# Glob all RISC-V GNU Assembly files
file(GLOB port_asm_srcs "${THREADX_PORT_DIR}/src/*.S")

set(srcs
    ${common_srcs}
    ${port_asm_srcs}
    # FreeRTOS Compatibility Layer C file
    "${THREADX_ROOT}/utility/rtos_compatibility_layers/FreeRTOS/tx_freertos.c"
)

# Register the component in the ESP-IDF build system
idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS 
        "include"
        "${THREADX_ROOT}/common/inc" 
        "${THREADX_PORT_DIR}/inc"
        "${THREADX_ROOT}/utility/rtos_compatibility_layers/FreeRTOS"
    REQUIRES 
        freertos
)

# Set compile definitions required for RISC-V ThreadX build
target_compile_definitions(${COMPONENT_LIB} PUBLIC
    TX_INCLUDE_USER_DEFINE_FILE
    TX_FREE_RTOS_CONVERSION
)
