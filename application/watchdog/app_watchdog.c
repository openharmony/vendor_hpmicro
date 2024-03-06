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
#include <watchdog_if.h>

#include "shell.h" 
#include "shcmd.h" 

#define LOG_TAG "HPM_WDG"
static void *WatchdogDriverApiTask(unsigned int arg);

typedef enum {
    FEED_WDG = 0,
    NO_FEED_EDG
} WDG_CMD_FEED_STATUS;

WDG_CMD_FEED_STATUS wdg_feed_status;

static void wdg_task_select(void)
{
    TSK_INIT_PARAM_S taskInitParam = {0};
    UINT32 taskID;

    switch (wdg_feed_status) {
    case FEED_WDG:
        taskInitParam.pcName = "wdg_feed_test";
        printf("enable wdg and feed case\n");
        break;
    case NO_FEED_EDG:
        taskInitParam.pcName = "wdg_reset_test";
        printf("enable wdg but no feed case\n");
        break;

    }
    
    taskInitParam.pfnTaskEntry = WatchdogDriverApiTask;
    taskInitParam.stackAddr = 0;
    taskInitParam.uwStackSize = 4096;
    taskInitParam.usTaskPrio = 20;
    taskInitParam.uwArg = 0x66;
    LOS_TaskCreate(&taskID, &taskInitParam);
}

int cmd_wdg_feed(void) 
{     
    wdg_feed_status = FEED_WDG;
    wdg_task_select();
    return 0; 
}

int cmd_wdg_reset(void) 
{     
    wdg_feed_status = NO_FEED_EDG;
    wdg_task_select();
    return 0; 
}

static void app_cmd_init(void) 
{        
    osCmdReg(CMD_TYPE_EX, "wdg", 0,(CMD_CBK_FUNC)cmd_wdg_feed);
    osCmdReg(CMD_TYPE_EX, "wdg_reset", 0,(CMD_CBK_FUNC)cmd_wdg_reset);
}

static void *WatchdogDriverApiTask(unsigned int arg)
{
    HILOG_INFO(HILOG_MODULE_APP, "%s", __func__);
    DevHandle handle;
    int32_t status;
    uint32_t seconds;
    int ret;
    ret = WatchdogOpen(0,  &handle);
    if (ret) {
        HILOG_ERROR(HILOG_MODULE_APP, "WatchdogOpen: %d", ret);
        return NULL;
    }

    WatchdogGetStatus(handle, &status);
    if (status != WATCHDOG_STOP) {
        HILOG_INFO(HILOG_MODULE_APP, "WatchdogGetStatus expect <WATCHDOG_STOP>, %u", __LINE__);
    }
    
    WatchdogSetTimeout(handle, 3);
    WatchdogGetTimeout(handle, &seconds);
    if (seconds != 3) {
        HILOG_ERROR(HILOG_MODULE_APP, "WatchdogGetTimeout Failed!!!");
    }

    WatchdogStart(handle);
    WatchdogGetStatus(handle, &status);
    if (status != WATCHDOG_START) {
        HILOG_ERROR(HILOG_MODULE_APP, "WatchdogGetStatus expect <WATCHDOG_START>");
    }

    if(wdg_feed_status)
    {
        printf("this case will not feed dog,system will reset after 3 seconds!\n");
    }

    uint32_t cnt = 3;
    while (cnt--) {
        HILOG_INFO(HILOG_MODULE_APP, "WatchdogFeed");
        if(!wdg_feed_status)
        {
            printf("will feed dog \n");
            WatchdogFeed(handle);
        }
        LOS_TaskDelay(2000);
    }

    WatchdogStop(handle);
    WatchdogGetStatus(handle, &status);
    if (status != WATCHDOG_STOP) {
        HILOG_ERROR(HILOG_MODULE_APP, "WatchdogGetStatus expect <WATCHDOG_STOP>, %u", __LINE__);
    }

    WatchdogClose(handle);

    return NULL;
}

static void show_menu(void)
{
    static const char menu_info[] = "\n"
                                    "****************************************************************\n"
                                    "*                                                              *\n"
                                    "*  WDG Example Menu                                            *\n"
                                    "*  cmd: 'wdg' enable wdg and feed every second                 *\n"
                                    "*  cmd: 'wdg_reset' will enable wdg but no feed                *\n"
                                    "****************************************************************\n";
    printf("%s", menu_info);
}

static void WatchdogDriverTest(void)
{
    app_cmd_init();
    show_menu();
}

APP_FEATURE_INIT(WatchdogDriverTest);