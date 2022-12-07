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

#if POLLING

void ReadWrite(Chip_SSP_DATA_SETUP_T * setup)
{
    Chip_SSP_Enable(NSS_SSP0);
    uint32_t length = Chip_SSP_RWFrames_Blocking(NSS_SSP0, setup);
    if (length != setup->length) {
        /* Error. Will become apparent when comparing rx and tx buffers in parent */
    }
}

#endif
