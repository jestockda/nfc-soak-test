/*
 * Copyright 2021 NXP
 * NXP confidential
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */
#include "board.h"

#if !POLLING

static Chip_SSP_DATA_SETUP_T * sSetup;

/**
 * SSP interrupt handler sub-routine
 */
void SSP0_IRQHandler(void)
{
    if (!Chip_SSP_Int_RWFrames8Bits(NSS_SSP0, sSetup)) {
        Chip_SSP_Int_Disable(NSS_SSP0); /* Some error occurred. */
    }
}

void ReadWrite(Chip_SSP_DATA_SETUP_T * setup)
{
    sSetup = setup;

    NVIC_DisableIRQ(SSP0_IRQn);
    Chip_SSP_Enable(NSS_SSP0);
    Chip_SSP_Int_FlushData(NSS_SSP0);
    Chip_SSP_Int_Enable(NSS_SSP0);
    NVIC_EnableIRQ(SSP0_IRQn);

    while (!Chip_SSP_GetStatus(NSS_SSP0, SSP_STAT_TFE)) {
        /* In this example, we wait forever until FIFO is empty.
         * An application could now start any other action in parallel.
         */
        ;
    }

    Chip_SSP_Int_Disable(NSS_SSP0);
    NVIC_DisableIRQ(SSP0_IRQn);
}

#endif
