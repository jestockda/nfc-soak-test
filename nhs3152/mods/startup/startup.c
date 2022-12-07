/*
 * Copyright 2011-2012 Code Red Technologies
 * Copyright 2015-2019,2021 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#include "cmsis.h"
#include "stdbool.h"
#include "assert.h"
#include "startup/startup.h"

#define WEAK __attribute__ ((weak))
#define ALIAS(f) __attribute__ ((weak, alias (#f)))

#ifdef __REDLIB__
    #if defined (__cplusplus)
        #error Redlib does not support C++
    #endif
    extern void __main(void); /**< __main() is the entry point for Redlib based applications; it calls main() */
#endif

#if defined (__cplusplus)
extern void __libc_init_array(void); /**< The entry point for the C++ library startup */
#endif

extern void _vStackTop(void); /**< External declaration for the pointer to the stack top from the Linker Script */

/* ------------------------------------------------------------------------- */

/** Initializes RW data sections. */
__attribute__ ((section(".after_vectors")))
static void data_init(unsigned int romstart, unsigned int start, unsigned int len)
{
    unsigned int *pulDest = (unsigned int*) start;
    unsigned int *pulSrc = (unsigned int*) romstart;
    unsigned int loop;
    for (loop = 0; loop < len; loop = loop + 4) {
        *pulDest++ = *pulSrc++;
    }
}

/** Initializes BSS data sections. */
__attribute__ ((section(".after_vectors")))
static void bss_init(unsigned int start, unsigned int len)
{
    unsigned int *pulDest = (unsigned int*) start;
    unsigned int loop;
    for (loop = 0; loop < len; loop = loop + 4) {
        *pulDest++ = 0;
    }
}

/** Default interrupt handler. Should never be entered. */
static void defaultIntHandler(void)
{
    ASSERT(false);
    NVIC_SystemReset();
}

/* Startup default Reset handler (if ResetISR is not defined by user)*/
__attribute__ ((section(".after_vectors")))
static void defaultResetISR(void)
{
    Startup_VarInit(); /* Initialize all global variables (to zero, or to their initializer) */
#if defined (__cplusplus) /* Initialize the runtime libraries, if c++ */
    __libc_init_array();
#endif
#if defined (__REDLIB__)
    __main() ;
#else
    main();
#endif

    /* main() shouldn't return. But if it does, do something sensible.*/
    ASSERT(false);
    NVIC_SystemReset();
}

/* ------------------------------------------------------------------------- */

/* The following symbols are constructs generated by the linker, indicating
 * the location of various points in the "Global Section Table". This table is
 * created by the linker via the Code Red managed linker script mechanism. It
 * contains the load address, execution address and length of the RW data
 * section and the execution and length of the BSS (zero initialized) section.
 */
extern unsigned int __data_section_table;
extern unsigned int __data_section_table_end;
//extern unsigned int __bss_section_table;
extern unsigned int __bss_section_table_end;

__attribute__ ((section(".after_vectors")))
void Startup_VarInit(void)
{
    unsigned int LoadAddr;
    unsigned int ExeAddr;
    unsigned int SectionLen;
    unsigned int *SectionTableAddr;

    /* Load base address of Global Section Table */
    SectionTableAddr = &__data_section_table;

    /* Copy the data sections from flash to SRAM. */
    while (SectionTableAddr < &__data_section_table_end) {
        LoadAddr = *SectionTableAddr++;
        ExeAddr = *SectionTableAddr++;
        SectionLen = *SectionTableAddr++;
        data_init(LoadAddr, ExeAddr, SectionLen);
    }

    /* At this point, SectionTableAddr = &__bss_section_table;
     * Zero fill the bss segment
     */
    while (SectionTableAddr < &__bss_section_table_end) {
        ExeAddr = *SectionTableAddr++;
        SectionLen = *SectionTableAddr++;
        bss_init(ExeAddr, SectionLen);
    }
}

/* Forward declaration of the specific IRQ handlers. These are aliased to defaultIntHandler. */

void ResetISR(void)          ALIAS(defaultResetISR);
void NMI_Handler(void)       ALIAS(defaultIntHandler);
void HardFault_Handler(void) ALIAS(defaultIntHandler);
void SVC_Handler(void)       ALIAS(defaultIntHandler);
void PendSV_Handler(void)    ALIAS(defaultIntHandler);
void SysTick_Handler(void)   ALIAS(defaultIntHandler);

void PIO0_0_IRQHandler(void)   ALIAS(defaultIntHandler);
void PIO0_1_IRQHandler(void)   ALIAS(defaultIntHandler);
void PIO0_2_IRQHandler(void)   ALIAS(defaultIntHandler);
void PIO0_3_IRQHandler(void)   ALIAS(defaultIntHandler);
void PIO0_4_IRQHandler(void)   ALIAS(defaultIntHandler);
void PIO0_5_IRQHandler(void)   ALIAS(defaultIntHandler);
void PIO0_6_IRQHandler(void)   ALIAS(defaultIntHandler);
void PIO0_7_IRQHandler(void)   ALIAS(defaultIntHandler);
void PIO0_8_IRQHandler(void)   ALIAS(defaultIntHandler);
void PIO0_9_IRQHandler(void)   ALIAS(defaultIntHandler);
void PIO0_10_IRQHandler(void)  ALIAS(defaultIntHandler);
void RFFIELD_IRQHandler(void)  ALIAS(defaultIntHandler);
void RTCPWREQ_IRQHandler(void) ALIAS(defaultIntHandler);
void NFC_IRQHandler(void)      ALIAS(defaultIntHandler);
void RTC_IRQHandler(void)      ALIAS(defaultIntHandler);
void I2C0_IRQHandler(void)     ALIAS(defaultIntHandler);
void CT16B0_IRQHandler(void)   ALIAS(defaultIntHandler);
void PMUFLD_IRQHandler(void)   ALIAS(defaultIntHandler);
void CT32B0_IRQHandler(void)   ALIAS(defaultIntHandler);
void PMUBOD_IRQHandler(void)   ALIAS(defaultIntHandler);
void SSP0_IRQHandler(void)     ALIAS(defaultIntHandler);
void TSEN_IRQHandler(void)     ALIAS(defaultIntHandler);
void C2D_IRQHandler(void)      ALIAS(defaultIntHandler);
void I2D_IRQHandler(void)      ALIAS(defaultIntHandler);
void ADC_IRQHandler(void)      ALIAS(defaultIntHandler);
void WDT_IRQHandler(void)      ALIAS(defaultIntHandler);
void FLASH_IRQHandler(void)    ALIAS(defaultIntHandler);
void EEPROM_IRQHandler(void)   ALIAS(defaultIntHandler);
void PIO0_IRQHandler(void)     ALIAS(defaultIntHandler);

/** The vector table. This @b must be linked to physical address 0x0000.0000. */
__attribute__ ((section(".isr_vector")))
extern void (* const g_pfnVectors[])(void);
void (* const g_pfnVectors[])(void) = {
    &_vStackTop,         /* Handler for EXCEPTION0  @0x00000000 - The initial stack pointer */
    ResetISR,            /* Handler for EXCEPTION1  @0x00000004 - The reset handler */
    NMI_Handler,         /* Handler for EXCEPTION2  @0x00000008 - The NMI handler */
    HardFault_Handler,   /* Handler for EXCEPTION3  @0x0000000C - The hard fault handler */
    defaultIntHandler,   /* Handler for EXCEPTION4  @0x00000010 - Reserved */
    defaultIntHandler,   /* Handler for EXCEPTION5  @0x00000014 - Reserved */
    defaultIntHandler,   /* Handler for EXCEPTION6  @0x00000018 - Reserved */
    defaultIntHandler,   /* Handler for EXCEPTION7  @0x0000001C - Reserved */
    defaultIntHandler,   /* Handler for EXCEPTION8  @0x00000020 - Reserved */
    defaultIntHandler,   /* Handler for EXCEPTION9  @0x00000024 - Reserved */
    defaultIntHandler,   /* Handler for EXCEPTION10 @0x00000028 - Reserved */
    SVC_Handler,         /* Handler for EXCEPTION11 @0x0000002C - SVCall handler */
    defaultIntHandler,   /* Handler for EXCEPTION12 @0x00000030 - Reserved */
    defaultIntHandler,   /* Handler for EXCEPTION13 @0x00000034 - Reserved */
    PendSV_Handler,      /* Handler for EXCEPTION14 @0x00000038 - The PendSV handler */
    SysTick_Handler,     /* Handler for EXCEPTION15 @0x0000003C - The SysTick handler */
    PIO0_0_IRQHandler,   /* Handler for EXCEPTION16 - INTERRUPT0  @0x00000040 */
    PIO0_1_IRQHandler,   /* Handler for EXCEPTION17 - INTERRUPT1  @0x00000044 */
    PIO0_2_IRQHandler,   /* Handler for EXCEPTION18 - INTERRUPT2  @0x00000048 */
    PIO0_3_IRQHandler,   /* Handler for EXCEPTION19 - INTERRUPT3  @0x0000004C */
    PIO0_4_IRQHandler,   /* Handler for EXCEPTION20 - INTERRUPT4  @0x00000050 */
    PIO0_5_IRQHandler,   /* Handler for EXCEPTION21 - INTERRUPT5  @0x00000054 */
    PIO0_6_IRQHandler,   /* Handler for EXCEPTION22 - INTERRUPT6  @0x00000058 */
    PIO0_7_IRQHandler,   /* Handler for EXCEPTION23 - INTERRUPT7  @0x0000005C */
    PIO0_8_IRQHandler,   /* Handler for EXCEPTION24 - INTERRUPT8  @0x00000060 */
    PIO0_9_IRQHandler,   /* Handler for EXCEPTION25 - INTERRUPT9  @0x00000064 */
    PIO0_10_IRQHandler,  /* Handler for EXCEPTION26 - INTERRUPT10 @0x00000068 */
    RFFIELD_IRQHandler,  /* Handler for EXCEPTION27 - INTERRUPT11 @0x0000006C */
    RTCPWREQ_IRQHandler, /* Handler for EXCEPTION28 - INTERRUPT12 @0x00000070 */
    NFC_IRQHandler,      /* Handler for EXCEPTION29 - INTERRUPT13 @0x00000074 */
    RTC_IRQHandler,      /* Handler for EXCEPTION30 - INTERRUPT14 @0x00000078 */
    I2C0_IRQHandler,     /* Handler for EXCEPTION31 - INTERRUPT15 @0x0000007C */
    CT16B0_IRQHandler,   /* Handler for EXCEPTION32 - INTERRUPT16 @0x00000080 */
    PMUFLD_IRQHandler,   /* Handler for EXCEPTION33 - INTERRUPT17 @0x00000084 */
    CT32B0_IRQHandler,   /* Handler for EXCEPTION34 - INTERRUPT18 @0x00000088 */
    PMUBOD_IRQHandler,   /* Handler for EXCEPTION35 - INTERRUPT19 @0x0000008C */
    SSP0_IRQHandler,     /* Handler for EXCEPTION36 - INTERRUPT20 @0x00000090 */
    TSEN_IRQHandler,     /* Handler for EXCEPTION37 - INTERRUPT21 @0x00000094 */
    C2D_IRQHandler,      /* Handler for EXCEPTION38 - INTERRUPT22 @0x00000098 */
    defaultIntHandler,   /* Handler for EXCEPTION39 - INTERRUPT23 @0x0000009C */
    I2D_IRQHandler,      /* Handler for EXCEPTION40 - INTERRUPT24 @0x000000A0 */
    ADC_IRQHandler,      /* Handler for EXCEPTION41 - INTERRUPT25 @0x000000A4 */
    WDT_IRQHandler,      /* Handler for EXCEPTION42 - INTERRUPT26 @0x000000A8 */
    FLASH_IRQHandler,    /* Handler for EXCEPTION43 - INTERRUPT27 @0x000000AC */
    EEPROM_IRQHandler,   /* Handler for EXCEPTION44 - INTERRUPT28 @0x000000B0 */
    defaultIntHandler,   /* Handler for EXCEPTION45 - INTERRUPT29 @0x000000B4 */
    defaultIntHandler,   /* Handler for EXCEPTION46 - INTERRUPT30 @0x000000B8 */
    PIO0_IRQHandler,     /* Handler for EXCEPTION47 - INTERRUPT31 @0x000000BC */
};
