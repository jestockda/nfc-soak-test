/*
 * GENERATED FILE - DO NOT EDIT
 * (c) Code Red Technologies Ltd, 2008-2013
 * (c) NXP Semiconductors 2013-2022
 * Generated linker script file for Generic-M0plus
 * Created from memory.ldt by FMCreateLinkMemory
 * Using Freemarker v2.3.23
 * MCUXpresso IDE v10.2.1 [Build 795] [2018-07-25]  on Dec 7, 2022 4:38:25 PM
 */

MEMORY
{
  /* Define each memory region */
  Flash_00 (rx) : ORIGIN = 0x0, LENGTH = 0x7800 /* 30K bytes (alias Flash) */  
  RamLoc (rwx) : ORIGIN = 0x10000000, LENGTH = 0x1ff0 /* 8176 bytes (alias RAM) */  
}

  /* Define a symbol for the top of each memory region */
  __base_Flash_00 = 0x0  ; /* Flash_00 */  
  __base_Flash = 0x0 ; /* Flash */  
  __top_Flash_00 = 0x0 + 0x7800 ; /* 30K bytes */  
  __top_Flash = 0x0 + 0x7800 ; /* 30K bytes */  
  __base_RamLoc = 0x10000000  ; /* RamLoc */  
  __base_RAM = 0x10000000 ; /* RAM */  
  __top_RamLoc = 0x10000000 + 0x1ff0 ; /* 8176 bytes */  
  __top_RAM = 0x10000000 + 0x1ff0 ; /* 8176 bytes */  
