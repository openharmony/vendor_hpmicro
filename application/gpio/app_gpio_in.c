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

static void *GpioDriverInTestTask(unsigned int arg)
{
    HILOG_INFO(HILOG_MODULE_APP, "GpioDriverInTestTask");

    init_gpio_pins();

    GpioSetDir(BOARD_GPIO_IN_IRQ_TASK_GPIO1, GPIO_DIR_IN);
    GpioSetDir(BOARD_GPIO_IN_IRQ_TASK_GPIO2, GPIO_DIR_IN);

    uint32_t gpio1_flag = GPIO_VAL_HIGH;
    uint32_t gpio2_flag = GPIO_VAL_HIGH;

    while (1) {
        uint16_t val;
        GpioRead(BOARD_GPIO_IN_IRQ_TASK_GPIO1, &val);
        if (val == GPIO_VAL_LOW && gpio1_flag == GPIO_VAL_HIGH) {
            gpio1_flag = GPIO_VAL_LOW;
            printf("GPIO1 DOWN \n");
        } else if (val == GPIO_VAL_HIGH && gpio1_flag == GPIO_VAL_LOW) {
            gpio1_flag = GPIO_VAL_HIGH;
            printf("GPIO2 UP \n");
        }

        GpioRead(BOARD_GPIO_IN_IRQ_TASK_GPIO2, &val);
        if (val == GPIO_VAL_LOW && gpio2_flag == GPIO_VAL_HIGH) {
            gpio2_flag = GPIO_VAL_LOW;
            printf("GPIO1 DOWN \n");
        } else if (val == GPIO_VAL_HIGH && gpio2_flag == GPIO_VAL_LOW) {
            gpio2_flag = GPIO_VAL_HIGH;
            printf("GPIO2 UP \n");
        }

        LOS_TaskDelay(50);
    }
    return NULL;
}

static void GpioDriverTest(void)
{
    TSK_INIT_PARAM_S taskInitParam = {0};
    
    taskInitParam.pcName = "gpio_in_test";
    taskInitParam.pfnTaskEntry = GpioDriverInTestTask;
    taskInitParam.stackAddr = 0;
    taskInitParam.uwStackSize = 4096;
    taskInitParam.usTaskPrio = 20;
    taskInitParam.uwArg = 0x66;
    UINT32 taskID;
    LOS_TaskCreate(&taskID, &taskInitParam);
}

APP_FEATURE_INIT(GpioDriverTest);
