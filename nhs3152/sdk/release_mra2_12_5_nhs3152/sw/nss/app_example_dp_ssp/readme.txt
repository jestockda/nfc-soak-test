/**
 * @defgroup APP_NSS_SSP ssp: SSP Driver Example
 * @ingroup APPS_NSS
 * Example application that demonstrates SSP master and slave functionality to send and receive data in both 
 * polling and interrupt modes using the @ref SSP_NSS "SSP Driver".
 *
 * @par Introduction
 *  This is an example application to show the similiarities and differences when making use of the
 *  @ref SSP_NSS "SSP Driver" when operating as slave or master, and when operating with polling or when using
 *  interrupts.
 *  4 build configurations demonstrate all the flavors. Each build configuration sets two defines that catch all the
 *  differences.
 *   -# Debug_Master_Interrupt @n
 *      A debug build for Master rx/tx in interrupt mode. @n
 *      The define MASTER is set to 1, the define POLLING is set to 0.
 *
 *   -# Debug_Slave_Interrupt @n
 *      Debug build for Slave rx/tx in interrupt mode. @n
 *      The define MASTER is set to 0, the define POLLING is set to 0.
 *
 *   -# Debug_Master_Polling @n
 *      Debug build for Master rx/tx in polling mode. @n
 *      The define MASTER is set to 1, the define POLLING is set to 1.
 *
 *   -# Debug_Slave_Polling @n
 *      Debug build for Slave rx/tx in polling mode. @n
 *      The define MASTER is set to 0, the define POLLING is set to 1.
 *  See the project properties > C/C++ Build > Settings > Tool Settings > MCU C Compiler > Symbols.
 *  The defines are used at an application level only.
 *
 * @par Setup
 *  - Connect the 4 SSP pins of two NHS31xx demo PCBs back to back: @c PIO0_2, @c PIO0_6, @c PIO0_8 and @c PIO0_9.
 *  - Connect the grounds of the two demo PCBs.
 *  - Flash one board with one of the master build configurations,
 *  - and the other board with one of slave build configurations.
 *
 * @par Use Case Description
 *  - First power cycle the slave, then power cycle the master.
 *  - The ICs will:
 *      - send 345 bytes to each other using frames of 8 bits.
 *      - compare the received data with what was expected.
 *      - blink the LED accordingly. @n
 *          After each transfer of 345 bytes, visual feedback is given on the correctness of the transmission and
 *          reception:
 *          - a steadily blinking led at a little less than 1 Hz indicates communication went as expected;
 *          - a high frequent flashing led (~20Hz) for half a second indicates at least one received byte was not as
 *              expected.
 *      - Repeat this cycle indefinitely until the power supply (LPC-Link2 debugger or external battery) is removed.
 *  When the demo runs, disconnecting a wire or resetting one IC should result in at least one IC complaining. After
 *  reconnecting the cable or letting the IC out of reset, the transfers will continue and be successful again.
 */
