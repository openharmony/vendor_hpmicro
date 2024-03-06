/*
 * Copyright (c) 2022 HPMicro
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <ohos_init.h>
#include <stdio.h>
#include <los_task.h>
#include <stdint.h>
#include <log.h>
#include <gpio_if.h>
#include <hpm_soc.h>
#include <hpm_clock_drv.h>
#include <board.h>

#define LOG_TAG "HPM_GPIO_OUT"

static int32_t GpioIrq(uint16_t gpio, void *data)
{
    printf("gpio: %u\n", gpio);
    return 0;
}

static void GpioDriverInTestTask(void)
{
    HILOG_INFO(HILOG_MODULE_APP, "GpioDriverIrqTest");

    init_gpio_pins();

    GpioSetDir(BOARD_GPIO_IN_IRQ_TASK_GPIO1, GPIO_DIR_IN);
    GpioSetDir(BOARD_GPIO_IN_IRQ_TASK_GPIO2, GPIO_DIR_IN);

    GpioSetIrq(BOARD_GPIO_IN_IRQ_TASK_GPIO1, GPIO_IRQ_TRIGGER_RISING, GpioIrq, NULL);
    GpioSetIrq(BOARD_GPIO_IN_IRQ_TASK_GPIO2, GPIO_IRQ_TRIGGER_FALLING, GpioIrq, NULL);

    GpioEnableIrq(BOARD_GPIO_IN_IRQ_TASK_GPIO1);
    GpioEnableIrq(BOARD_GPIO_IN_IRQ_TASK_GPIO2);
}

APP_FEATURE_INIT(GpioDriverInTestTask);
