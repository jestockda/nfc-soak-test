/*
 * Measure two terminal resistance and report out over NFC
 * Last revised on 7 December 2022
 * by J. Evan Smith
 *
 * TODO:
 *
 * (1) Reduce NDEF to TEXT rather than TEXT+MIME
 *
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "board.h"
#include "ndeft2t/ndeft2t.h"

#define LOCAL "en"
#define MIME "nhs31xx/example.ndef"

#define MAX_TEXT_PAYLOAD (254 - (NDEFT2T_MSG_OVERHEAD(true, \
        NDEFT2T_TEXT_RECORD_OVERHEAD(true, sizeof(LOCAL) - 1) + \
        NDEFT2T_MIME_RECORD_OVERHEAD(true, sizeof(MIME) - 1)) / 2))
static uint8_t sText[MAX_TEXT_PAYLOAD] = "default text";

#define MAX_MIME_PAYLOAD MAX_TEXT_PAYLOAD
static uint8_t sBytes[MAX_MIME_PAYLOAD] = {0, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE};

static volatile bool sMsgAvailable = false;
static volatile bool sFieldPresent = false;

static volatile float AN4 = 0.0;
static volatile int current_native;
static volatile int current_pA;
static volatile double resistance = -1;

static void set_dac(void);
static void get_i2d(void);
static void get_adc(void);

static void GenerateNdef_Text(void);

void App_FieldStatusCb(bool status)
{
    if (status) {
        LED_On(LED_RED);
    }
    else {
        LED_Off(LED_RED);
    }
    sFieldPresent = status; /* Handled in main loop */
}

void App_MsgAvailableCb(void)
{
    sMsgAvailable = true; /* Handled in main loop */
}


static void GenerateNdef_Text(void)
{
    uint8_t instance[NDEFT2T_INSTANCE_SIZE];
    uint8_t buffer[NFC_SHARED_MEM_BYTE_SIZE ];
    NDEFT2T_CREATE_RECORD_INFO_T textRecordInfo = {.pString = (uint8_t *)"en" /* language code */,
                                                   .shortRecord = true,
                                                   .uriCode = 0 /* don't care */};
    NDEFT2T_CREATE_RECORD_INFO_T mimeRecordInfo = {.pString = (uint8_t *)MIME /* mime type */,
                                                   .shortRecord = true,
                                                   .uriCode = 0 /* don't care */};
    NDEFT2T_CreateMessage(instance, buffer, NFC_SHARED_MEM_BYTE_SIZE, true);
    if (NDEFT2T_CreateTextRecord(instance, &textRecordInfo)) {
        if (NDEFT2T_WriteRecordPayload(instance, sText, sizeof(sText) - 1 /* exclude NUL char */)) {
            NDEFT2T_CommitRecord(instance);
        }
    }
    if (NDEFT2T_CreateMimeRecord(instance, &mimeRecordInfo)) {
        if (NDEFT2T_WriteRecordPayload(instance, sBytes, sizeof(sBytes))) {
            NDEFT2T_CommitRecord(instance);
        }
    }
    NDEFT2T_CommitMessage(instance); /* Copies the generated message to NFC shared memory. */
}

void set_dac(void){
	Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_ANA0_1, IOCON_FUNC_1);
	Chip_ADCDAC_Init(NSS_ADCDAC0);
	Chip_ADCDAC_SetMuxDAC(NSS_ADCDAC0, ADCDAC_IO_ANA0_1);
	Chip_ADCDAC_SetModeDAC(NSS_ADCDAC0, ADCDAC_CONTINUOUS); // hold
	Chip_ADCDAC_WriteOutputDAC(NSS_ADCDAC0, 4095); // max
}

void get_i2d(void){
	int i2dValue;
	int i2dNativeValue;
	Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_ANA0_4, IOCON_FUNC_1);
	Chip_I2D_Init(NSS_I2D);
	Chip_I2D_Setup(NSS_I2D, I2D_SINGLE_SHOT, I2D_SCALER_GAIN_10_1, I2D_CONVERTER_GAIN_LOW, 100);
	Chip_I2D_SetMuxInput(NSS_I2D, I2D_INPUT_ANA0_4);
	Chip_I2D_Start(NSS_I2D);
	while (!(Chip_I2D_ReadStatus(NSS_I2D) & I2D_STATUS_CONVERSION_DONE)) {;} // wait
	i2dNativeValue = Chip_I2D_GetValue(NSS_I2D);
	i2dValue = Chip_I2D_NativeToPicoAmpere(i2dNativeValue, I2D_SCALER_GAIN_10_1, I2D_CONVERTER_GAIN_LOW, 100);
	Chip_I2D_DeInit(NSS_I2D);

	current_native = i2dNativeValue;
	current_pA = i2dValue;
}


void get_adc(void){
	Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_ANA0_4, IOCON_FUNC_1);
	Chip_ADCDAC_SetMuxADC(NSS_ADCDAC0, ADCDAC_IO_ANA0_4);
	Chip_ADCDAC_SetInputRangeADC(NSS_ADCDAC0, ADCDAC_INPUTRANGE_WIDE);
	Chip_ADCDAC_SetModeADC(NSS_ADCDAC0, ADCDAC_SINGLE_SHOT);
	Chip_ADCDAC_StartADC(NSS_ADCDAC0);
	while (!(Chip_ADCDAC_ReadStatus(NSS_ADCDAC0) & ADCDAC_STATUS_ADC_DONE)) {;} // wait
	AN4 = Chip_ADCDAC_GetValueADC(NSS_ADCDAC0);
	AN4 = (AN4/4095.0)*1.6;
}

int main(void)
{
    Board_Init();
    NDEFT2T_Init();

    for (;;) {
        if (sFieldPresent) {
        	set_dac();
        	get_i2d();
        	get_adc();
            resistance = (1.6-AN4)/(current_pA*1e-12);
            sprintf((char *)sText, "AN4: %6.2f current_pA: %8.d resistance: %e current_native: %6.d", AN4,current_pA, resistance, current_native);
            GenerateNdef_Text();
        }
    }
    return 0;
}

