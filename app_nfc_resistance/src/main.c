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

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "board.h"
#include "ndeft2t/ndeft2t.h"

#define LOCAL "en"
#define MIME1 "nhs31xx/mime1"
#define MIME2 "nhs31xx/mime2"

#define MAX_TEXT_PAYLOAD (254 - (NDEFT2T_MSG_OVERHEAD(true, \
        NDEFT2T_TEXT_RECORD_OVERHEAD(true, sizeof(LOCAL) - 1) + \
        NDEFT2T_MIME_RECORD_OVERHEAD(true, sizeof(MIME1) - 1)) / 2))
static uint8_t sText[MAX_TEXT_PAYLOAD] = "default text";

#define MAX_MIME_PAYLOAD (254 - (NDEFT2T_MSG_OVERHEAD(true, NDEFT2T_MIME_RECORD_OVERHEAD(true, sizeof(MIME1) - 1))))

static uint8_t sBytes1[MAX_MIME_PAYLOAD];
static uint8_t sBytes2[MAX_MIME_PAYLOAD];
static uint8_t sBytes3[MAX_MIME_PAYLOAD];
static uint8_t sBytes4[MAX_MIME_PAYLOAD];

#define SP0_ADDR   0x4001c010
#define READ_SP0()     (*(volatile uint32_t *)SP0_ADDR)
#define WRITE_SP0(val) ((*(volatile uint32_t *)SP0_ADDR) = (val))

static volatile bool sMsgAvailable = false;
static volatile bool sFieldPresent = false;

static volatile bool flag = false;

//static volatile float AN4 = 0.0;
static volatile int current_native;
static volatile int current_pA;
static volatile double resistance = -1;

int volatile N = 200;

int pre [] = {0x880,0x900,0x97F,0x9FD,0xA78,0xAF1,0xB67,0xBDA,0xC49,0xCB3,
		       0xD19,0xD79,0xDD4,0xE2A,0xE78,0xEC1,0xF02,0xF3D,0xF70,0xF9B,
			   0xFBF,0xFDB,0xFEF,0xFFB,0x1000,0xFFB,0xFEF,0xFDB,0xFBF,0xF9B,
			   0xF70,0xF3D,0xF02,0xEC1,0xE78,0xE2A,0xDD4,0xD79,0xD19,0xCB3,
			   0xC49,0xBDA,0xB67,0xAF1,0xA78,0x9FD,0x97F,0x900,0x880,0x7FF,
			   0x77F,0x6FF,0x680,0x602,0x587,0x50E,0x498,0x425,0x3B6,0x34C,
			   0x2E6,0x286,0x22B,0x1D5,0x187,0x13E,0xFD,0xC2,0x8F,0x64,
			   0x40,0x24,0x10,0x4,0x0,0x4,0x10,0x24,0x40,0x64,
			   0x8F,0xC2,0xFD,0x13E,0x187,0x1D5,0x22B,0x286,0x2E6,0x34C,
			   0x3B6,0x425,0x498,0x50E,0x587,0x602,0x680,0x6FF,0x77F,0x800,
			   0x880,0x900,0x97F,0x9FD,0xA78,0xAF1,0xB67,0xBDA,0xC49,0xCB3,
			   0xD19,0xD79,0xDD4,0xE2A,0xE78,0xEC1,0xF02,0xF3D,0xF70,0xF9B,
			   0xFBF,0xFDB,0xFEF,0xFFB,0x1000,0xFFB,0xFEF,0xFDB,0xFBF,0xF9B,
			   0xF70,0xF3D,0xF02,0xEC1,0xE78,0xE2A,0xDD4,0xD79,0xD19,0xCB3,
			   0xC49,0xBDA,0xB67,0xAF1,0xA78,0x9FD,0x97F,0x900,0x880,0x800,
			   0x77F,0x6FF,0x680,0x602,0x587,0x50E,0x498,0x425,0x3B6,0x34C,
			   0x2E6,0x286,0x22B,0x1D5,0x187,0x13E,0xFD,0xC2,0x8F,0x64,
			   0x40,0x24,0x10,0x4,0x0,0x4,0x10,0x24,0x40,0x64,
			   0x8F,0xC2,0xFD,0x13E,0x187,0x1D5,0x22B,0x286,0x2E6,0x34C,
			   0x3B6,0x425,0x498,0x50E,0x587,0x602,0x680,0x6FF,0x77F,0x800};

static volatile uint16_t AN2 [200];
static volatile uint16_t AN4 [200];

static void set_dac(void);
static void get_i2d(void);
static void get_adc(void);

static void GenerateNdef_TextMime(void);

void App_FieldStatusCb(bool status)
{
    /*
	if (status) {
        LED_On(LED_RED);
    }
    else {
        LED_Off(LED_RED);
    }
    */
    sFieldPresent = status; /* Handled in main loop */

}

void App_MsgAvailableCb(void)
{
    sMsgAvailable = true; /* Handled in main loop */
}

static void GenerateNdef_Mime_1(void)
{
    uint8_t instance[NDEFT2T_INSTANCE_SIZE];
    uint8_t buffer[NFC_SHARED_MEM_BYTE_SIZE];
    NDEFT2T_CREATE_RECORD_INFO_T mimeRecordInfo = {.pString = (uint8_t *)MIME1,
                                                   .shortRecord = true, // <<<<< THIS
                                                   .uriCode = 0 /* don't care */};

    NDEFT2T_CreateMessage(instance, buffer, NFC_SHARED_MEM_BYTE_SIZE, true);
    if (NDEFT2T_CreateMimeRecord(instance, &mimeRecordInfo)) {
        if (NDEFT2T_WriteRecordPayload(instance, sBytes1, sizeof(sBytes1))) {
            NDEFT2T_CommitRecord(instance);
        }
    }
    if (NDEFT2T_CreateMimeRecord(instance, &mimeRecordInfo)) {
        if (NDEFT2T_WriteRecordPayload(instance, sBytes2, sizeof(sBytes2))) {
            NDEFT2T_CommitRecord(instance);
        }
    }
    NDEFT2T_CommitMessage(instance); /* Copies the generated message to NFC shared memory. */
}

static void GenerateNdef_Mime_2(void)
{
    uint8_t instance[NDEFT2T_INSTANCE_SIZE];
    uint8_t buffer[NFC_SHARED_MEM_BYTE_SIZE];
    NDEFT2T_CREATE_RECORD_INFO_T mimeRecordInfo = {.pString = (uint8_t *)MIME2,
                                                   .shortRecord = true, // <<<<< THIS
                                                   .uriCode = 0 /* don't care */};

    NDEFT2T_CreateMessage(instance, buffer, NFC_SHARED_MEM_BYTE_SIZE, true);
    if (NDEFT2T_CreateMimeRecord(instance, &mimeRecordInfo)) {
        if (NDEFT2T_WriteRecordPayload(instance, sBytes3, sizeof(sBytes3))) {
            NDEFT2T_CommitRecord(instance);
        }
    }
    if (NDEFT2T_CreateMimeRecord(instance, &mimeRecordInfo)) {
        if (NDEFT2T_WriteRecordPayload(instance, sBytes4, sizeof(sBytes4))) {
            NDEFT2T_CommitRecord(instance);
        }
    }
    NDEFT2T_CommitMessage(instance); /* Copies the generated message to NFC shared memory. */
}

static void GenerateNdef_TextMime(void)
{
    uint8_t instance[NDEFT2T_INSTANCE_SIZE];
    uint8_t buffer[NFC_SHARED_MEM_BYTE_SIZE];
    NDEFT2T_CREATE_RECORD_INFO_T textRecordInfo = {.pString = (uint8_t *)"en" /* language code */,
                                                   .shortRecord = true,
                                                   .uriCode = 0 /* don't care */};
    NDEFT2T_CREATE_RECORD_INFO_T mimeRecordInfo = {.pString = (uint8_t *)MIME1 /* mime type */,
                                                   .shortRecord = true,
                                                   .uriCode = 0 /* don't care */};
    NDEFT2T_CreateMessage(instance, buffer, NFC_SHARED_MEM_BYTE_SIZE, true);
    if (NDEFT2T_CreateTextRecord(instance, &textRecordInfo)) {
        if (NDEFT2T_WriteRecordPayload(instance, sText, sizeof(sText) - 1 /* exclude NUL char */)) {
            NDEFT2T_CommitRecord(instance);
        }
    }
    if (NDEFT2T_CreateMimeRecord(instance, &mimeRecordInfo)) {
        if (NDEFT2T_WriteRecordPayload(instance, sBytes1, sizeof(sBytes1))) {
            NDEFT2T_CommitRecord(instance);
        }
    }
    NDEFT2T_CommitMessage(instance); /* Copies the generated message to NFC shared memory. */
}

/* ============================================== */

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
	Chip_I2D_Setup(NSS_I2D, I2D_SINGLE_SHOT, I2D_SCALER_GAIN_10_1, I2D_CONVERTER_GAIN_LOW, 200);
	Chip_I2D_SetMuxInput(NSS_I2D, I2D_INPUT_ANA0_4);
	Chip_I2D_Start(NSS_I2D);
	while (!(Chip_I2D_ReadStatus(NSS_I2D) & I2D_STATUS_CONVERSION_DONE)) {;} // wait
	i2dNativeValue = Chip_I2D_GetValue(NSS_I2D);
	i2dValue = Chip_I2D_NativeToPicoAmpere(i2dNativeValue, I2D_SCALER_GAIN_10_1, I2D_CONVERTER_GAIN_LOW, 200);

	if (i2dValue == 25e6) {
		Chip_I2D_Setup(NSS_I2D, I2D_SINGLE_SHOT, I2D_SCALER_GAIN_100_1, I2D_CONVERTER_GAIN_LOW, 200);
		Chip_I2D_SetMuxInput(NSS_I2D, I2D_INPUT_ANA0_4);
		Chip_I2D_Start(NSS_I2D);
		while (!(Chip_I2D_ReadStatus(NSS_I2D) & I2D_STATUS_CONVERSION_DONE)) {;} // wait
		i2dNativeValue = Chip_I2D_GetValue(NSS_I2D);
		i2dValue = Chip_I2D_NativeToPicoAmpere(i2dNativeValue, I2D_SCALER_GAIN_100_1, I2D_CONVERTER_GAIN_LOW, 200);
	}

	current_native = i2dNativeValue;
	current_pA = i2dValue;
	Chip_I2D_DeInit(NSS_I2D);
}


void get_adc(void){
	Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_ANA0_4, IOCON_FUNC_1);
	Chip_ADCDAC_SetMuxADC(NSS_ADCDAC0, ADCDAC_IO_ANA0_4);
	Chip_ADCDAC_SetInputRangeADC(NSS_ADCDAC0, ADCDAC_INPUTRANGE_WIDE);
	Chip_ADCDAC_SetModeADC(NSS_ADCDAC0, ADCDAC_SINGLE_SHOT);
	Chip_ADCDAC_StartADC(NSS_ADCDAC0);
	while (!(Chip_ADCDAC_ReadStatus(NSS_ADCDAC0) & ADCDAC_STATUS_ADC_DONE)) {;} // wait
	//AN4 = Chip_ADCDAC_GetValueADC(NSS_ADCDAC0);
    //AN4 = (AN4/4095.0)*1.6;
}

/* ============================================== */

void init_adcdac(void){
	Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_ANA0_0, IOCON_FUNC_1);
	Chip_ADCDAC_Init(NSS_ADCDAC0);
	Chip_ADCDAC_SetMuxDAC(NSS_ADCDAC0, ADCDAC_IO_ANA0_0);
	Chip_ADCDAC_SetInputRangeADC(NSS_ADCDAC0, ADCDAC_INPUTRANGE_WIDE);
	Chip_ADCDAC_SetModeDAC(NSS_ADCDAC0, ADCDAC_CONTINUOUS); // hold

	Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_ANA0_2, IOCON_FUNC_1);
	Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_ANA0_4, IOCON_FUNC_1);
	Chip_ADCDAC_SetMuxADC(NSS_ADCDAC0, ADCDAC_IO_ANA0_2);
	Chip_ADCDAC_SetInputRangeADC(NSS_ADCDAC0, ADCDAC_INPUTRANGE_WIDE);
	Chip_ADCDAC_SetModeADC(NSS_ADCDAC0, ADCDAC_CONTINUOUS);
	Chip_ADCDAC_StartADC(NSS_ADCDAC0);

    Chip_Flash_SetHighPowerMode(true);
    bool check_pm = Chip_Flash_GetHighPowerMode();

    //Chip_Flash_SetNumWaitStates(1);
    //int check_ws = Chip_Flash_GetNumWaitStates();

    if (check_pm){
    	Chip_Clock_System_SetClockDiv(1);
    }
    else{
    	Chip_Clock_System_SetClockDiv(2);
	}

    WRITE_SP0(0x2);
}

void sine_dac_fast(void){


	//Chip_ADCDAC_StopADC(NSS_ADCDAC0);

	//Chip_ADCDAC_StartADC(NSS_ADCDAC0);

	//while (!(Chip_ADCDAC_ReadStatus(NSS_ADCDAC0) & ADCDAC_STATUS_ADC_DONE)) {;} // wait

	for (int i = 0; i < N; i++){
		Chip_ADCDAC_WriteOutputDAC(NSS_ADCDAC0, pre[i]);
		AN2[i] = Chip_ADCDAC_GetValueADC(NSS_ADCDAC0);
	}

	//Chip_ADCDAC_StopADC(NSS_ADCDAC0);
	Chip_ADCDAC_SetMuxADC(NSS_ADCDAC0, ADCDAC_IO_ANA0_4);
	//Chip_ADCDAC_StartADC(NSS_ADCDAC0);

	//while (!(Chip_ADCDAC_ReadStatus(NSS_ADCDAC0) & ADCDAC_STATUS_ADC_DONE)) {;} // wait

	for (int i = 0; i < N; i++){
		Chip_ADCDAC_WriteOutputDAC(NSS_ADCDAC0, pre[i]);
		AN4[i] = Chip_ADCDAC_GetValueADC(NSS_ADCDAC0);
	}

	Chip_ADCDAC_SetMuxADC(NSS_ADCDAC0, ADCDAC_IO_ANA0_2);
}

void pwm_dev(void)
{

	Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_PIO0_7, IOCON_FUNC_1);
	Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_PIO0_3, IOCON_FUNC_1);

    Chip_TIMER16_0_Init();
    Chip_TIMER_PrescaleSet(NSS_TIMER16_0, ((uint32_t)Chip_Clock_System_GetClockFreq() / 250000)-1); //250

    /* MR0 -> low to high at 15, no interrupt, no stop, no reset */
    Chip_TIMER_SetMatch(NSS_TIMER16_0, 0, 63); /* 0-based MR. 15/125 -> 88% duty-cycle */
    Chip_TIMER_MatchDisableInt(NSS_TIMER16_0, 0);
    Chip_TIMER_StopOnMatchDisable(NSS_TIMER16_0, 0);
    Chip_TIMER_ResetOnMatchDisable(NSS_TIMER16_0, 0);

    /* MR1 -> low to high at 75, no interrupt, no stop, no reset */
    Chip_TIMER_SetMatch(NSS_TIMER16_0, 1, 63); /* 0-based MR. 75/125 -> 40% duty-cycle */
    Chip_TIMER_MatchDisableInt(NSS_TIMER16_0, 1);
    Chip_TIMER_StopOnMatchDisable(NSS_TIMER16_0, 1);
    Chip_TIMER_ResetOnMatchDisable(NSS_TIMER16_0, 1);

    //MR2 -> PWM cycle time, no interrupt, no stop, reset on match

    Chip_TIMER_SetMatch(NSS_TIMER16_0, 2, 125 - 1);
    Chip_TIMER_MatchDisableInt(NSS_TIMER16_0, 2);
    Chip_TIMER_StopOnMatchDisable(NSS_TIMER16_0, 2);
    Chip_TIMER_ResetOnMatchEnable(NSS_TIMER16_0, 2);

    //MR3

    Chip_TIMER_MatchDisableInt(NSS_TIMER16_0, 3);
    Chip_TIMER_StopOnMatchDisable(NSS_TIMER16_0, 3);
    Chip_TIMER_ResetOnMatchDisable(NSS_TIMER16_0, 3);

    /* Enable PWM */
    Chip_TIMER_SetMatchOutputMode(NSS_TIMER16_0, 0, TIMER_MATCH_OUTPUT_PWM);
    Chip_TIMER_SetMatchOutputMode(NSS_TIMER16_0, 1, TIMER_MATCH_OUTPUT_PWM);

    /* Reset TC */
    Chip_TIMER_Reset(NSS_TIMER16_0);
    Chip_TIMER_Enable(NSS_TIMER16_0);
}

static void ParseNdef(void)
{
    uint8_t instance[NDEFT2T_INSTANCE_SIZE];
    uint8_t buffer[NFC_SHARED_MEM_BYTE_SIZE];
    NDEFT2T_PARSE_RECORD_INFO_T recordInfo;
    int len = 0;
    uint8_t *pData = NULL;

    if (NDEFT2T_GetMessage(instance, buffer, NFC_SHARED_MEM_BYTE_SIZE)) {
        while (NDEFT2T_GetNextRecord(instance, &recordInfo) != false) {
            pData = (uint8_t *)NDEFT2T_GetRecordPayload(instance, &len);
            switch (recordInfo.type) {
                case NDEFT2T_RECORD_TYPE_TEXT:
                    if ((size_t)len <= MAX_TEXT_PAYLOAD) {
                        memcpy(sText, pData, (size_t)len);
                        memset(sText + len, 0, MAX_TEXT_PAYLOAD - (size_t)len);
                    }
                    break;
                default:
                    /* ignore */
                    break;
            }
        }
    }
}

int main(void)
{
    Board_Init();
    NDEFT2T_Init();
    init_adcdac();

    for (;;) {

    	sine_dac_fast();

    	int q = 0;
    	for (int i = 0; i<N/2; i++){
    		sBytes1[q]   = (uint8_t)(AN2[i] >>  0);
    		sBytes1[q+1] = (uint8_t)(AN2[i] >>  8);

		    sBytes2[q]   = (uint8_t)(AN2[i+100] >>  0);
		    sBytes2[q+1] = (uint8_t)(AN2[i+100] >>  8);

		    sBytes3[q]   = (uint8_t)(AN4[i] >>  0);
		    sBytes3[q+1] = (uint8_t)(AN4[i] >>  8);

		    sBytes4[q]   = (uint8_t)(AN4[i+100] >>  0);
		    sBytes4[q+1] = (uint8_t)(AN4[i+100] >>  8);

		    q+=2;
    	}

    	if (!flag){
    		GenerateNdef_Mime_1();
    		flag = true;
    	}

    	while (!sMsgAvailable) {;}
		if (sMsgAvailable){
         	sMsgAvailable = false;
    		ParseNdef();
		}
		if (sText[0]=='a'){
			sText[0]='x';
			GenerateNdef_Mime_2();
		}
    	while (!sMsgAvailable) {;}
		if (sMsgAvailable){
         	sMsgAvailable = false;
    		ParseNdef();
		}
		if (sText[0]=='b'){
			sText[0]='x';
			flag = false;
		}

        /*
    	set_dac();
        get_i2d();
        get_adc();
        resistance = (1.6-AN4)/(current_pA*1e-12);
        sprintf((char *)sText, "%6.4f,%8.d,%e", AN4, current_pA, resistance);
        GenerateNdef_TextMime();
		*/
    }
    return 0;
}

