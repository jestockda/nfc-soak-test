/*
 * Copyright 2015-2016,2018-2019,2021 NXP
 * NXP confidential
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#include "stdbool.h"
#include "stdint.h"
#include "string.h"
#include "board.h"
extern void ReadWrite(Chip_SSP_DATA_SETUP_T * setup);

int main(void)
{
    Board_Init();

    /* There are only two differences in using the SPI interface between master and slave:
     * - a slave needs a higher SysClock than a master to achieve the same communication speed.
     * - a master declares himself as master in the SPI API; a slave declares himself as slave.
     */

    /* SSP initialization */
    Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_PIO0_2, IOCON_FUNC_1); /* SSEL */
    Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_PIO0_6, IOCON_FUNC_1); /* SCLK */
    Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_PIO0_8, IOCON_FUNC_1); /* MISO */
    Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_PIO0_9, IOCON_FUNC_1); /* MOSI */
    Chip_SSP_Init(NSS_SSP0);
#if MASTER
    Chip_SSP_SetMaster(NSS_SSP0, true);
#else /* SLAVE */
    Chip_SSP_SetMaster(NSS_SSP0, false);
#endif
    Chip_SSP_SetFormat(NSS_SSP0, SSP_BITS_8, SSP_FRAME_FORMAT_SPI, SSP_CLOCK_MODE0);
    Chip_SSP_SetBitRate(NSS_SSP0, 19232);
    /* To achieve 19200 bps:
     * - the master's SysClock must at least be 65.5 kHz.
     * - the slave's SysClock must at least be 250 kHz.
     * 19232 and not 19200: not all bit rates can be achieved, and the bit rate that is used is the closest possible
     * value @b less @b than the given bit rate. See "SPI bitrate selection" in the firmware documentation.
     */

    LED_On(LED_ALL);
    for (;;) {
        /* Initialize buffers */
        #define BUFFER_SIZE 345
        uint8_t txBuffer[BUFFER_SIZE];
        for (int i = 0; i < BUFFER_SIZE; i++) {
            txBuffer[i] = (uint8_t)i;
        }
        uint8_t rxBuffer[BUFFER_SIZE] = {0};

        /* Start the transfer.
         * The implementation of ReadWrite is taken either from using_interrupts.c or with_polling.c
         */
        Chip_SSP_DATA_SETUP_T sspDataSetup = {.rx_cnt = 0,
                                              .tx_cnt = 0,
                                              .rx_data = rxBuffer,
                                              .tx_data = txBuffer,
                                              .length = sizeof(txBuffer)};

        ReadWrite(&sspDataSetup);

        /* Provide visual feedback on the result of the transfer. */
        if (memcmp(txBuffer, rxBuffer, sizeof(txBuffer))) { /* if error */
            for (int i = 0; i < 20; i++) {
                LED_Toggle(LED_ALL);
                Chip_Clock_System_BusyWait_ms(25);
            }
        }
        else { /* else success */
            LED_Toggle(LED_ALL);
            Chip_Clock_System_BusyWait_ms(500);
        }
#if MASTER
        Chip_Clock_System_BusyWait_ms(10);
#endif
    }

    return 0;
}
