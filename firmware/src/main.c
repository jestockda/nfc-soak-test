/*
 * Measure resistance or impedance and report out over NFC
 * Last revised on 14 December 2022
 * by J. Evan Smith
 */

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "board.h"
#include "ndeft2t/ndeft2t.h"

#define SP0_ADDR   0x4001c010
#define READ_SP0()     (*(volatile uint32_t *)SP0_ADDR)
#define WRITE_SP0(val) ((*(volatile uint32_t *)SP0_ADDR) = (val))

#define LOCAL "en"
#define MIME1 "nhs31xx/mime1"
#define MIME2 "nhs31xx/mime2"

#define MAX_TEXT_PAYLOAD (254-(NDEFT2T_MSG_OVERHEAD(true, NDEFT2T_TEXT_RECORD_OVERHEAD(true, sizeof(LOCAL)-1))))
#define MAX_MIME_PAYLOAD (254-(NDEFT2T_MSG_OVERHEAD(true, NDEFT2T_MIME_RECORD_OVERHEAD(true, sizeof(MIME1)-1))))

static uint8_t sText[MAX_TEXT_PAYLOAD] = "default text";

static uint8_t sBytes1[MAX_MIME_PAYLOAD];
static uint8_t sBytes2[MAX_MIME_PAYLOAD];
static uint8_t sBytes3[MAX_MIME_PAYLOAD];
static uint8_t sBytes4[MAX_MIME_PAYLOAD];

static volatile bool sMsgAvailable = false;
static volatile bool sFieldPresent = false;
static volatile bool flag = false;

static volatile int AN1;
static volatile int I2D;
static volatile int I2D_pA;

static volatile double AN1_V;
static volatile double resistance = -1;

static volatile uint16_t AN3 [200];
static volatile uint16_t AN4 [200];

int sine [] = {0x880,0x900,0x97F,0x9FD,0xA78,0xAF1,0xB67,0xBDA,0xC49,0xCB3,
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

void App_FieldStatusCb(bool status){

	if (status) {
        LED_On(LED_RED);
    }
    else {
        LED_Off(LED_RED);
    }
    sFieldPresent = status; // handled in main loop
}

void App_MsgAvailableCb(void){

    sMsgAvailable = true; // handled in main loop
}

static void GenerateNdef_Text(void){

    uint8_t instance[NDEFT2T_INSTANCE_SIZE];
    uint8_t buffer[NFC_SHARED_MEM_BYTE_SIZE];
    NDEFT2T_CREATE_RECORD_INFO_T textRecordInfo = {.pString = (uint8_t *)"en" /* language code */,
                                                   .shortRecord = true,
                                                   .uriCode = 0 /* don't care */};

    NDEFT2T_CreateMessage(instance, buffer, NFC_SHARED_MEM_BYTE_SIZE, true);
    if (NDEFT2T_CreateTextRecord(instance, &textRecordInfo)) {
        if (NDEFT2T_WriteRecordPayload(instance, sText, sizeof(sText) - 1 /* exclude NUL char */)) {
            NDEFT2T_CommitRecord(instance);
        }
    }
    NDEFT2T_CommitMessage(instance); // copies the generated message to NFC shared memory
}

static void GenerateNdef_Mime(uint8_t data1[], uint8_t data2[], char mime[]){

    uint8_t instance[NDEFT2T_INSTANCE_SIZE];
    uint8_t buffer[NFC_SHARED_MEM_BYTE_SIZE];
    NDEFT2T_CREATE_RECORD_INFO_T mimeRecordInfo = {.pString = (uint8_t *)mime,
                                                   .shortRecord = true,
                                                   .uriCode = 0 /* don't care */};

    NDEFT2T_CreateMessage(instance, buffer, NFC_SHARED_MEM_BYTE_SIZE, true);
    if (NDEFT2T_CreateMimeRecord(instance, &mimeRecordInfo)) {
        if (NDEFT2T_WriteRecordPayload(instance, data1, MAX_MIME_PAYLOAD)) {
            NDEFT2T_CommitRecord(instance);
        }
    }
    if (NDEFT2T_CreateMimeRecord(instance, &mimeRecordInfo)) {
        if (NDEFT2T_WriteRecordPayload(instance, data2, MAX_MIME_PAYLOAD)) {
            NDEFT2T_CommitRecord(instance);
        }
    }
    NDEFT2T_CommitMessage(instance); // copies the generated message to NFC shared memory
}

static void parseNdef(void){

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
                    break;
            }
        }
    }
}

void init_io (int case_){

	switch(case_){
		case 0: // resistance
			Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_ANA0_0, IOCON_FUNC_1);
			Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_ANA0_1, IOCON_FUNC_1);
			Chip_ADCDAC_Init(NSS_ADCDAC0);
			Chip_ADCDAC_SetMuxDAC(NSS_ADCDAC0, ADCDAC_IO_ANA0_0);
			Chip_ADCDAC_SetModeDAC(NSS_ADCDAC0, ADCDAC_CONTINUOUS); // hold
			Chip_ADCDAC_SetModeADC(NSS_ADCDAC0, ADCDAC_SINGLE_SHOT);
			Chip_ADCDAC_SetInputRangeADC(NSS_ADCDAC0, ADCDAC_INPUTRANGE_WIDE);
			Chip_I2D_Init(NSS_I2D);
			Chip_I2D_Setup(NSS_I2D, I2D_SINGLE_SHOT, I2D_SCALER_GAIN_10_1, I2D_CONVERTER_GAIN_LOW, 200);
			break;

		case 1: // impedance
		    Chip_Flash_SetHighPowerMode(true);
		    bool check_pm = Chip_Flash_GetHighPowerMode();
		    //Chip_Flash_SetNumWaitStates(1);
		    //int check_ws = Chip_Flash_GetNumWaitStates();
		    if (check_pm){
		    	Chip_Clock_System_SetClockDiv(1); // WARNING: need HPM or WS
		    }
		    else{
		    	Chip_Clock_System_SetClockDiv(2);
			}
			Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_ANA0_2, IOCON_FUNC_1);
			Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_ANA0_3, IOCON_FUNC_1);
			Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_ANA0_4, IOCON_FUNC_1);
			Chip_ADCDAC_Init(NSS_ADCDAC0);
			Chip_ADCDAC_SetMuxDAC(NSS_ADCDAC0, ADCDAC_IO_ANA0_2);
			Chip_ADCDAC_SetModeDAC(NSS_ADCDAC0, ADCDAC_CONTINUOUS); // hold
			Chip_ADCDAC_SetMuxADC(NSS_ADCDAC0, ADCDAC_IO_ANA0_3);
			Chip_ADCDAC_SetModeADC(NSS_ADCDAC0, ADCDAC_CONTINUOUS);
			Chip_ADCDAC_SetInputRangeADC(NSS_ADCDAC0, ADCDAC_INPUTRANGE_WIDE);
			Chip_ADCDAC_StartADC(NSS_ADCDAC0);
			WRITE_SP0(0x2);
			break;

	}
}

void deinit_io (int case_){

	switch(case_){
		case 0: // resistance
		    Chip_ADCDAC_StopDAC(NSS_ADCDAC0);
		    Chip_ADCDAC_DeInit(NSS_ADCDAC0);
		    Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_ANA0_0, IOCON_FUNC_0);
		    Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_ANA0_1, IOCON_FUNC_0);
		    break;

		case 1: // impedance
			Chip_ADCDAC_StopDAC(NSS_ADCDAC0);
			Chip_ADCDAC_StopADC(NSS_ADCDAC0);
			Chip_ADCDAC_DeInit(NSS_ADCDAC0);
			Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_ANA0_2, IOCON_FUNC_0);
			Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_ANA0_3, IOCON_FUNC_0);
			Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_ANA0_4, IOCON_FUNC_0);
			Chip_Clock_System_SetClockDiv(16);
	        Chip_Clock_System_BusyWait_ms(200);
			Chip_Flash_SetHighPowerMode(false);
			WRITE_SP0(0x22); // SLOWCLK + ADWIDERANGE
			break;

	}
}

int get_adc(void){

	Chip_ADCDAC_SetMuxADC(NSS_ADCDAC0, ADCDAC_IO_ANA0_1);
	Chip_ADCDAC_StartADC(NSS_ADCDAC0);
	while (!(Chip_ADCDAC_ReadStatus(NSS_ADCDAC0) & ADCDAC_STATUS_ADC_DONE)) {;} // wait
	int AN1_ = Chip_ADCDAC_GetValueADC(NSS_ADCDAC0);
    return AN1_;

}

int get_i2d(void){

	int i2dValue;
	int i2dNativeValue;
	Chip_I2D_SetMuxInput(NSS_I2D, I2D_INPUT_ANA0_1);
	Chip_I2D_Start(NSS_I2D);
	while (!(Chip_I2D_ReadStatus(NSS_I2D) & I2D_STATUS_CONVERSION_DONE)) {;} // wait
	i2dNativeValue = Chip_I2D_GetValue(NSS_I2D);
	i2dValue = Chip_I2D_NativeToPicoAmpere(i2dNativeValue, I2D_SCALER_GAIN_10_1, I2D_CONVERTER_GAIN_LOW, 200);
	if (i2dValue == 25e6) {
		Chip_I2D_Setup(NSS_I2D, I2D_SINGLE_SHOT, I2D_SCALER_GAIN_100_1, I2D_CONVERTER_GAIN_LOW, 200);
		Chip_I2D_Start(NSS_I2D);
		while (!(Chip_I2D_ReadStatus(NSS_I2D) & I2D_STATUS_CONVERSION_DONE)) {;} // wait
		i2dNativeValue = Chip_I2D_GetValue(NSS_I2D);
		i2dValue = Chip_I2D_NativeToPicoAmpere(i2dNativeValue, I2D_SCALER_GAIN_100_1, I2D_CONVERTER_GAIN_LOW, 200);
	}
	return i2dValue;
	Chip_I2D_DeInit(NSS_I2D);

}

void set_dac_sine(void){

	for (int i = 0; i < 200; i++){
		Chip_ADCDAC_WriteOutputDAC(NSS_ADCDAC0, sine[i]);
		AN3[i] = Chip_ADCDAC_GetValueADC(NSS_ADCDAC0);
	}

	Chip_ADCDAC_SetMuxADC(NSS_ADCDAC0, ADCDAC_IO_ANA0_4);

	for (int i = 0; i < 200; i++){
		Chip_ADCDAC_WriteOutputDAC(NSS_ADCDAC0, sine[i]);
		AN4[i] = Chip_ADCDAC_GetValueADC(NSS_ADCDAC0);
	}

	Chip_ADCDAC_SetMuxADC(NSS_ADCDAC0, ADCDAC_IO_ANA0_3);

}

void send_resistance(void){

	Chip_ADCDAC_WriteOutputDAC(NSS_ADCDAC0, 4095); // max
	I2D_pA = get_i2d();
	AN1 = get_adc();
    AN1_V = (AN1/4095.0)*1.6;

    resistance = (1.6-AN1_V)/(I2D_pA*1e-12);
    sprintf((char *)sText, "%6.4f,%8.d,%e", AN1_V, I2D_pA, resistance);
    GenerateNdef_Text();

}

void send_impedance(void){

	set_dac_sine();

	int j = 0;
	for (int i = 0; i<100; i++){
		sBytes1[j]   = (uint8_t)(AN3[i] >>  0);
	    sBytes1[j+1] = (uint8_t)(AN3[i] >>  8);
	    sBytes2[j]   = (uint8_t)(AN3[i+100] >>  0);
		sBytes2[j+1] = (uint8_t)(AN3[i+100] >>  8);
		sBytes3[j]   = (uint8_t)(AN4[i] >>  0);
		sBytes3[j+1] = (uint8_t)(AN4[i] >>  8);
		sBytes4[j]   = (uint8_t)(AN4[i+100] >>  0);
		sBytes4[j+1] = (uint8_t)(AN4[i+100] >>  8);
		j+=2;
	}

	if (!flag){ // !flag == first pass
		GenerateNdef_Mime(sBytes1,sBytes2,MIME1);
	    flag = true;
	}
	while (!sMsgAvailable) {;}
	if (sMsgAvailable){
		parseNdef();
		sMsgAvailable = false;
	}
	if (sText[0]=='a'){
		GenerateNdef_Mime(sBytes3,sBytes4,MIME2);
		sText[0]='x';
	}
	while (!sMsgAvailable) {;}
	if (sMsgAvailable){
		parseNdef();
		sMsgAvailable = false;
	}
	if (sText[0]=='b'){
		sText[0]='x';
		flag = false;
	}
}

int main(void)
{
    Board_Init();
    NDEFT2T_Init();

    for (;;) {
    	while (!sMsgAvailable) {;}
    	if (sMsgAvailable){
    		parseNdef();
    		sMsgAvailable = false;
    	}
    	if (sText[0]=='0'){
    		init_io(0);
    		send_resistance();
    		deinit_io(0);
    		sText[0]='x';
    	}
    	else if (sText[0]=='1'){
    		init_io(1);
    		Chip_Clock_System_BusyWait_ms(200);
    		send_impedance();
    		deinit_io(1);
    	}
    }
    return 0;
}

