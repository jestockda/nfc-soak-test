/*
 * Copyright 2014-2016,2021 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#include "board.h"

int main(void)
{
    Board_Init();

    /* This wait call is added to protect you from bricking the IC.
     *
     * When the IC resets, the bootloader will prepare the IC for use. During this time, SWD access is not possible.
     * When the bootloader is finished, it will open up SWD access and transfer control to the main application.
     * If the code of your application turns off SWD access almost immediately, the time window during which a debugger
     * can halt the core may become too small. The chance of timing the debug commands correctly becomes too small to
     * have a viable chance of success. As a result, the developer is locked out of the IC, rendering it useless.
     *
     * SWD access is turned off by reconfiguring one of the SWD pins (PIO 10 and PIO 11), or by going to the
     * Deep Power Down state or Power-off state.
     *
     * Another way to prevent your IC from becoming useless is to check a pin state at startup and to wait while it is
     * in an unexpected state - see for example the ResetISR function in app_demo_dp_tlogger.
     *
     * IF YOU ARE STARTING TO CHECK OUT THE DIFFERENT FEATURES OF THIS IC, START AFTER THIS WAIT CALL.
     */
    Chip_Clock_System_BusyWait_ms(1000);

    /* Optional feature: send the ARM clock to PIO0_1 */
    Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_PIO0_1, IOCON_FUNC_1);
    Chip_Clock_Clkout_SetClockSource(CLOCK_CLKOUTSOURCE_SYSTEM);

    /* Blink with a period of 250ms+250ms, or 2Hz */
    while (1) {
        LED_Toggle(LED_RED);
        Chip_Clock_System_BusyWait_ms(250);
    }

    return 0;
}
