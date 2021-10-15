#ifndef LITHIUM_WATCHDOG_H
#define LITHIUM_WATCHDOG_H

/*
 * Part of liblithium, under the Apache License v2.0.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef LITH_ENABLE_WATCHDOG
#define LITH_ENABLE_WATCHDOG 0
#endif

void lith_watchdog_pet(void);

#endif /* LITHIUM_WATCHDOG_H */
