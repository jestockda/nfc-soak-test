/*
 * Copyright 2015-2016,2018,2021 NXP
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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wunused-variable"

void nfc_nss_example_1(void)
{
//! [nfc_nss_example_1]

    /* See the section 'Tag reader' in communication.html for an in-depth example how an ndef message is built up. */
    __attribute__((aligned (4)))
    uint8_t message[24] = {0x03, /* Type: NDEF TLV */
                           0x12, /* Number of bytes that follow, not including the next TLV. */
                           0xD1, /* Text Record Header [MB=1, ME=1, CF=0, SR=1, IL=0, TNF=1] */
                           0x01, /* Type length */
                           0x0E, /* Payload length */
                           (uint8_t)'T', /* TEXT type message */
                           0x02, /* Text Record sub-Header, indicates the length of the language code. */
                           (uint8_t)'e',
                           (uint8_t)'n',
                           (uint8_t)'H',
                           (uint8_t)'e',
                           (uint8_t)'l',
                           (uint8_t)'l',
                           (uint8_t)'o',
                           (uint8_t)' ',
                           (uint8_t)'W',
                           (uint8_t)'o',
                           (uint8_t)'r',
                           (uint8_t)'l',
                           (uint8_t)'d',
                           0xFE, /* Terminator TLV used to indicate the end of the used NFC memory. */
                           0x00, /* NULL TLV for word alignment */
                           0x00, /* NULL TLV for word alignment */
                           0x00 /* NULL TLV for word alignment */
    };
    ASSERT(NFC_SHARED_MEM_BYTE_SIZE >= sizeof(message));

    Chip_NFC_Init(NSS_NFC);
    if (Chip_NFC_WordWrite(NSS_NFC, (uint32_t*)NFC_SHARED_MEM_START, (uint32_t*)message, sizeof(message) / 4)) {
        /* Success. The message can now be read by a PCD (Proximity Coupled Device) like a USB NFC read or a phone. */
    }
//! [nfc_nss_example_1]
}

#pragma GCC diagnostic pop
