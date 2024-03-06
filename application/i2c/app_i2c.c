/*
 * Copyright (c) 2024 HPMicro
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
#include <i2c_if.h>
#include <i2c_core.h>
#include "hpm_soc.h"
#include "board.h"
#include "hpm_i2c_drv.h"

#include "shell.h" 
#include "shcmd.h" 

struct HPMI2cDevice {
    uint32_t id;
    uint32_t base;
    uint32_t clkFreq;
};

typedef enum {
    MASTER = 0,
    SLAVE
} I2C_TYPE;

#define TEST_I2C_SLAVE_ADDRESS  (0x16U)
#define TEST_TRANSFER_DATA_IN_BYTE  (128U)
uint8_t trx_buff[TEST_TRANSFER_DATA_IN_BYTE];
uint8_t tx_buff[TEST_TRANSFER_DATA_IN_BYTE];
uint8_t rrx_buff[TEST_TRANSFER_DATA_IN_BYTE];

static void *I2CDriverApiMasterTask(unsigned int arg);
static void *I2CDriverApiSlaveTask(unsigned int arg);
int32_t hpmReceive(struct I2cCntlr *cntlr, struct I2cMsg *msgs, int16_t count);

I2C_TYPE i2c_type;
static DevHandle i2c_handle;
static I2C_Type *i2c_base; 

static void I2CModeSelect(void)
{
    TSK_INIT_PARAM_S taskInitParam = {0};
    UINT32 taskID;

    switch (i2c_type) {
    case MASTER:
        taskInitParam.pcName = "i2c_master_test";
        printf("I2C switch to master mode\n");
        taskInitParam.pfnTaskEntry = I2CDriverApiMasterTask;
        break;
    case SLAVE:
        taskInitParam.pcName = "i2c_slave_test";
        printf("I2C switch to slave mode\n");
        taskInitParam.pfnTaskEntry = I2CDriverApiSlaveTask;
        break;

    }
    
    taskInitParam.stackAddr = 0;
    taskInitParam.uwStackSize = 4096;
    taskInitParam.usTaskPrio = 20;
    taskInitParam.uwArg = 0x66;
    LOS_TaskCreate(&taskID, &taskInitParam);
}

int cmd_i2c_slave(void) 
{     
    i2c_type = SLAVE;
    I2CModeSelect();
    return 0; 
}

int cmd_i2c_master(void) 
{     
    i2c_type = MASTER;
    I2CModeSelect();
    return 0; 
}

static void app_cmd_init(void) 
{        
    osCmdReg(CMD_TYPE_EX, "i2cm", 0,(CMD_CBK_FUNC)cmd_i2c_master);
    osCmdReg(CMD_TYPE_EX, "i2cs", 0,(CMD_CBK_FUNC)cmd_i2c_slave);
}

static void prepare_tx_data(void)
{
    for (uint8_t i = 0; i < TEST_TRANSFER_DATA_IN_BYTE; i++) {
        tx_buff[i] = i % 0xFF;
    }
}

static void check_transfer_data(void)
{
    uint32_t i = 0U, error_count = 0U;

    printf("The sent data are:");
    for (i = 0; i < TEST_TRANSFER_DATA_IN_BYTE; i++) {
        if ((i & 0x0FU) == 0U) {
            printf("\n");
        }
        printf("0x%02X ", tx_buff[i]);
    }
    printf("\n");
    printf("The received data are:");
    for (i = 0; i < TEST_TRANSFER_DATA_IN_BYTE; i++) {
        if ((i & 0x0FU) == 0U) {
            printf("\n");
        }
        printf("0x%02X ", trx_buff[i]);
        if (tx_buff[i] != trx_buff[i]) {
            error_count++;
        }
    }
    printf("\n");
    if (error_count == 0) {
        printf("I2C transfer all data matched!\n");
    } else {
        printf("Error occurred in I2C transfer!\n");
    }
}

static void print_received_data(void)
{
    printf("The received data are:");
    for (uint32_t i = 0; i < TEST_TRANSFER_DATA_IN_BYTE; i++) {
        if ((i & 0x0FU) == 0U) {
            printf("\n");
        }
        printf("0x%02X ", rrx_buff[i]);
    }
    printf("\n");
}

static void *I2CDriverApiMasterTask(unsigned int arg)
{
    HILOG_INFO(HILOG_MODULE_APP, "%s", __func__);
    struct I2cMsg msgs[2] = {0};

    printf("I2C Master Test Case ~~~ !\n");

    memset(tx_buff, 0, TEST_TRANSFER_DATA_IN_BYTE);
    memset(trx_buff, 0, TEST_TRANSFER_DATA_IN_BYTE);

    prepare_tx_data();

    // msgs[0] write regaddr to adapter
    msgs[0].addr = TEST_I2C_SLAVE_ADDRESS;
    msgs[0].flags = 0; // I2C_FLAG_WRITE
    msgs[0].len = TEST_TRANSFER_DATA_IN_BYTE;
    msgs[0].buf = &tx_buff[0];

    // msgs[1] read dataLen byte regvalue to *regdata
    msgs[1].addr = TEST_I2C_SLAVE_ADDRESS;
    msgs[1].flags = I2C_FLAG_READ;
    msgs[1].len = TEST_TRANSFER_DATA_IN_BYTE;
    msgs[1].buf = &trx_buff[0];

    I2cTransfer(i2c_handle, msgs, 2);

    LOS_TaskDelay(10);

    check_transfer_data();

    return NULL;
}

static void *I2CDriverApiSlaveTask(unsigned int arg)
{
    HILOG_INFO(HILOG_MODULE_APP, "%s", __func__);
    struct I2cMsg msgs[1] = {0};
    hpm_stat_t stat;
    i2c_config_t config;
    uint32_t freq;

    config.i2c_mode = i2c_mode_normal;
    config.is_10bit_addressing = false;
    freq = clock_get_frequency(TEST_APP_I2C_CLK_NAME);
    stat = i2c_init_slave(i2c_base, freq, &config, TEST_I2C_SLAVE_ADDRESS);
    if (stat != status_success) {
        printf("I2C slave init failed~~~ !\n");
    }

    memset(rrx_buff, 0, TEST_TRANSFER_DATA_IN_BYTE);

    printf("I2C Slave Test Case, will sent back the received data ~~~ !\n");

    // msgs[0] read dataLen byte regvalue to *regdata
    msgs[0].flags = I2C_FLAG_READ;
    msgs[0].len = TEST_TRANSFER_DATA_IN_BYTE;
    msgs[0].buf = &rrx_buff[0];

    hpmReceive((struct I2cCntlr *)i2c_handle, msgs, 1);

    print_received_data();

    return NULL;
}

static void show_menu(void)
{
    static const char menu_info[] = "\n"
                                    "****************************************************************\n"
                                    "*                                                              *\n"
                                    "*  I2C Example Menu                                            *\n"
                                    "*  cmd: 'i2cm' enable I2C Master Mode                          *\n"
                                    "*  cmd: 'i2cs' enable I2C Slave Mode                           *\n"
                                    "****************************************************************\n";
    printf("%s", menu_info);
}

static void I2CDriverTest(void)
{
    show_menu();

    app_cmd_init();

    i2c_handle = I2cOpen((int16_t)I2C_BUS_NUM);
    if (i2c_handle == NULL) {
        printf("handle is NULL, failed \n");
    }

    struct HPMI2cDevice *hpmI2cDev = (struct HPMI2cDevice *)((struct I2cCntlr *)i2c_handle)->priv;
    i2c_base = (I2C_Type *)(hpmI2cDev->base);

    init_i2c_pins(i2c_base);
}

APP_FEATURE_INIT(I2CDriverTest);