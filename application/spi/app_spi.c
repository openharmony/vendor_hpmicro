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
#include "board.h"
#include "hpm_spi_drv.h"
#include <spi_if.h>
#include <spi_core.h>

#define BITS_PER_WORD_8BITS      8
#define SPI_MAX_SPEED             10000000
#define BUFF_NUM             10

spi_control_config_t control_config = {0};

uint8_t wbuff[BUFF_NUM] = {0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9};
uint8_t rbuff[BUFF_NUM] = {0};

struct SpiClient {
    struct SpiCntlr *cntlr;
    uint32_t csNum;
};

struct HPMSpiDevice {
    uint32_t id;
    uint32_t base;
    uint32_t clkFreq;
    struct SpiCfg cfg;
};

static DevHandle GetSpiHandle(void)
{
    int32_t ret;
    DevHandle spiHandle = NULL;
    struct SpiDevInfo spiDevinfo;
    struct SpiCfg cfg;

    spiDevinfo.busNum = 0;
    spiDevinfo.csNum = 0;
    spiHandle = SpiOpen(&spiDevinfo);
    if (spiHandle == NULL) {
        printf("SpiOpen failed\n");
        return NULL;
    }
    cfg.transferMode = SPI_POLLING_TRANSFER;
    cfg.bitsPerWord = BITS_PER_WORD_8BITS;
    cfg.mode = SPI_CLK_PHASE | SPI_MODE_LOOP;
    cfg.maxSpeedHz = SPI_MAX_SPEED;
    ret = SpiSetCfg(spiHandle, &cfg);
    if (ret != HDF_SUCCESS) {
        printf("SpiSetCfg failed, ret:%d\n", ret);
        SpiClose(spiHandle);
        return NULL;
    }
    printf("Spi init success!\n");
    return spiHandle;
}

static void show_menu(void)
{
    static const char menu_info[] = "\n"
                                    "****************************************************************\n"
                                    "*                                                              *\n"
                                    "*  SPI Example                                                 *\n"
                                    "****************************************************************\n";
    printf("%s", menu_info);
}

static void SPIDriverTest(void)
{
    DevHandle spiHdl = NULL;
    struct SpiClient *client = NULL;
    struct SpiCntlr *cntlr = NULL;
    struct SpiMsg msg = {0};
    int32_t ret;
    int i=0;

    show_menu();

    spiHdl = GetSpiHandle();
    if (spiHdl == NULL) {
        printf("GetSpiHandle failed\n");
    }

    client = (struct SpiClient *)spiHdl;
    cntlr = (struct SpiCntlr *)client->cntlr;
    struct HPMSpiDevice *hpmSpiDev = (struct HPMSpiDevice *)cntlr->priv;
    SPI_Type *base = (SPI_Type *)hpmSpiDev->base;
    init_spi_pins(base);
    control_config.master_config.cmd_enable = false;  /* cmd phase control for master */
    control_config.master_config.addr_enable = false; /* address phase control for master */
    control_config.slave_config.slave_data_only = false;
    control_config.common_config.trans_mode = spi_trans_write_read_together;

    msg.rbuf = &rbuff[0];
    msg.wbuf = &wbuff[0];
    msg.len = BUFF_NUM;
    msg.keepCs = 0; // switch off the CS after transfer
    msg.delayUs = 0;
    msg.speed = 0;    // use default speed

    ret = SpiTransfer(spiHdl, &msg, 1);
    if (ret != HDF_SUCCESS) {
        printf("SpiTransferTest: spi write err, ret: %d\n", ret);
    }

    printf("SPI-Master, write_data: ");
    for(i=0; i<BUFF_NUM; )
    {
        printf("0x%02x ", wbuff[i++]);
    }

    printf("\nSPI-Master, read_data: ");

    for(i=0; i<BUFF_NUM; )
    {
        printf("0x%02x ", rbuff[i++]);
    }

    printf("\n");

    if(!memcmp(msg.wbuf,msg.rbuf,msg.len))
    {
        printf("Spi-test success\n");
    }
    else
    {
        printf("Spi-test failed\n");
    }
}

APP_FEATURE_INIT(SPIDriverTest);