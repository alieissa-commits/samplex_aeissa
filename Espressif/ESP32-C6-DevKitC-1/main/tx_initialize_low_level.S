/***************************************************************************
 * Copyright (c) 2025 10xEngineers
 *
 * This program and the accompanying materials are made available under the
 * terms of the MIT License which is available at
 * https://opensource.org/licenses/MIT.
 *
 * SPDX-License-Identifier: MIT
 **************************************************************************/

    .section .data
    .global  _tx_initialize_unused_memory
_tx_initialize_unused_memory:
    .word    0

    .section .text
    .global  _tx_initialize_low_level
    .type    _tx_initialize_low_level, @function
_tx_initialize_low_level:

    /* Save the system stack pointer. */
    la      t0, _tx_thread_system_stack_ptr
    sw      sp, 0(t0)

    /* Return to caller */
    ret
