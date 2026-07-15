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

cmake_minimum_required(VERSION 3.22)

# Include the ESP-IDF CMake project build system
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Enable minimal build to keep compilation fast
idf_build_set_property(MINIMAL_BUILD ON)

project(esp32c6_threadx LANGUAGES C ASM)
