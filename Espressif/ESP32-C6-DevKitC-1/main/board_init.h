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

#ifndef BOARD_INIT_H
#define BOARD_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize the board hardware register configurations (Assist Debug bypass, etc.) */
void board_init(void);

/* Initialize and start the periodic system tick timer callback */
void board_timer_setup(void);

#ifdef __cplusplus
}
#endif

#endif /* BOARD_INIT_H */
