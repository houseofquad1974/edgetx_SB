/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _BOARD_COMMON_H_
#define _BOARD_COMMON_H_

#include <inttypes.h>
#include "cpu_id.h"

#if !defined(SIMU) && !defined(BACKUP)
#if defined(STM32F4)
  #include "stm32f4xx.h"
#else
  #include "stm32f2xx.h"
  #include "dwt.h"    // the old ST library that we use does not define DWT register for STM32F2xx
#endif
#endif


#if defined(SIMU)
#include "../simu/simpgmspace.h"
#endif

#if !defined(SIMU) && !defined(BACKUP)
  #define ticksNow() ((uint32_t)(DWT->CYCCNT))
#else
  #define ticksNow() ((uint32_t)(0UL))
#endif

#include "delays_driver.h"

#endif
