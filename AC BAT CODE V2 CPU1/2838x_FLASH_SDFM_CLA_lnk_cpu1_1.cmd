/*
 * CPU1.cmd
 *
 *  Created on: 27/7/2025
 *      Author: LUU LINH K65
 */

/* CLA scratchpad size */
CLA_SCRATCHPAD_SIZE = 0x100;
--undef_sym=__cla_scratchpad_end
--undef_sym=__cla_scratchpad_start

MEMORY
{
   /* BEGIN is used for the "boot to Flash" bootloader mode   */
   BEGIN            : origin = 0x080000, length = 0x000002
   BOOT_RSVD        : origin = 0x000002, length = 0x0001AF     /* Part of M0, BOOT rom will use this for stack */
   RAMM0            : origin = 0x0001B1, length = 0x00024F
   RAMM1            : origin = 0x000400, length = 0x0003F8     /* on-chip RAM block M1 */
   RAMD0            : origin = 0x00C000, length = 0x000800
   RAMD1            : origin = 0x00C800, length = 0x000800
   RAMLS0           : origin = 0x008000, length = 0x000800
   //RAMLS1           : origin = 0x008800, length = 0x000800
   //RAMLS2           : origin = 0x009000, length = 0x000800
   //RAMLS3           : origin = 0x009800, length = 0x000800
   //RAMLS4           : origin = 0x00A000, length = 0x000800
   //RAMLS5           : origin = 0x00A800, length = 0x000800
   //RAMLS6           : origin = 0x00B000, length = 0x000800
   //RAMLS7           : origin = 0x00B800, length = 0x000800
   RAMLS1234567     : origin = 0x008800, length = 0x003800
   RAMGS0           : origin = 0x00D000, length = 0x001000
   RAMGS1           : origin = 0x00E000, length = 0x001000
   RAMGS2           : origin = 0x00F000, length = 0x001000
   RAMGS3           : origin = 0x010000, length = 0x001000
   RAMGS4           : origin = 0x011000, length = 0x001000
   RAMGS5           : origin = 0x012000, length = 0x001000
   RAMGS6           : origin = 0x013000, length = 0x001000
   RAMGS7           : origin = 0x014000, length = 0x001000
   RAMGS8           : origin = 0x015000, length = 0x001000
   RAMGS9           : origin = 0x016000, length = 0x001000
   RAMGS10          : origin = 0x017000, length = 0x001000
   RAMGS11          : origin = 0x018000, length = 0x001000
   RAMGS12          : origin = 0x019000, length = 0x001000
   RAMGS13          : origin = 0x01A000, length = 0x001000
   RAMGS14          : origin = 0x01B000, length = 0x001000
   RAMGS15          : origin = 0x01C000, length = 0x000FF8

   /* Flash sectors */
   FLASH0           : origin = 0x080002, length = 0x001FFE  /* on-chip Flash (first two bytes reserved for BEGIN) */
   FLASH1           : origin = 0x082000, length = 0x002000  /* on-chip Flash */
   FLASH2           : origin = 0x084000, length = 0x002000  /* on-chip Flash */
   FLASH3           : origin = 0x086000, length = 0x002000  /* on-chip Flash */
   FLASH4           : origin = 0x088000, length = 0x008000  /* on-chip Flash */
   FLASH5           : origin = 0x090000, length = 0x008000  /* on-chip Flash */
   FLASH6           : origin = 0x098000, length = 0x008000  /* on-chip Flash */
   FLASH7           : origin = 0x0A0000, length = 0x008000  /* on-chip Flash */
   FLASH8           : origin = 0x0A8000, length = 0x008000  /* on-chip Flash */
   FLASH9           : origin = 0x0B0000, length = 0x008000  /* on-chip Flash */
   FLASH10          : origin = 0x0B8000, length = 0x002000  /* on-chip Flash */
   FLASH11          : origin = 0x0BA000, length = 0x002000  /* on-chip Flash */
   FLASH12          : origin = 0x0BC000, length = 0x002000  /* on-chip Flash */
   FLASH13          : origin = 0x0BE000, length = 0x001FF0  /* on-chip Flash (end reserved) */

   CPU1TOCPU2RAM   : origin = 0x03A000, length = 0x000800
   CPU2TOCPU1RAM   : origin = 0x03B000, length = 0x000800
   CPUTOCMRAM      : origin = 0x039000, length = 0x000800
   CMTOCPURAM      : origin = 0x038000, length = 0x000800

   CANA_MSG_RAM     : origin = 0x049000, length = 0x000800
   CANB_MSG_RAM     : origin = 0x04B000, length = 0x000800

   RESET            : origin = 0x3FFFC0, length = 0x000002

   CLA1_MSGRAMLOW   : origin = 0x001480,   length = 0x000080
   CLA1_MSGRAMHIGH  : origin = 0x001500,   length = 0x000080
}

SECTIONS
{
   codestart           : > BEGIN, ALIGN(8)

   /* Place executable code in Flash segments (LOAD in flash) */
   .text               : >> FLASH1 | FLASH2 | FLASH3 | FLASH4, ALIGN(8)
   .cinit              : > FLASH1 | FLASH2 , ALIGN(8)
   .switch             : > FLASH1, ALIGN(8)
   .reset              : > RESET, TYPE = DSECT /* not used, */
   .stack              : > RAMM1

#if defined(__TI_EABI__)
   .init_array      : > FLASH1, ALIGN(8)
   .bss             : > RAMLS0
   .bss:output      : > RAMLS0
   .data            : > RAMLS0
   .sysmem          : > RAMLS0
   /* Initialized const/data sections in Flash */
   .const           : > FLASH5, ALIGN(8)
#else
   .pinit           : > FLASH1, ALIGN(8)
   .ebss            : > RAMLS0
   .esysmem         : > RAMLS0
   /* Initialized const/data sections in Flash */
   .econst          : >> FLASH4 | FLASH5, ALIGN(8)
#endif

   /* Message RAM regions (NOINIT) */
   MSGRAM_CPU1_TO_CPU2 : > CPU1TOCPU2RAM, type=NOINIT
   MSGRAM_CPU2_TO_CPU1 : > CPU2TOCPU1RAM, type=NOINIT
   MSGRAM_CPU_TO_CM    : > CPUTOCMRAM, type=NOINIT
   MSGRAM_CM_TO_CPU    : > CMTOCPURAM, type=NOINIT

   /* CLA specific sections */
#if defined(__TI_EABI__)
   Cla1Prog         :   LOAD = FLASH4 | FLASH5,
                        RUN  = RAMLS1234567,
                        LOAD_START(Cla1funcsLoadStart),
                        LOAD_END(Cla1funcsLoadEnd),
                        RUN_START(Cla1funcsRunStart),
                        LOAD_SIZE(Cla1funcsLoadSize),
                        ALIGN(8)
#else
   Cla1Prog         :   LOAD = FLASH4,
                        RUN  = RAMLS5,
                        LOAD_START(_Cla1funcsLoadStart),
                        LOAD_END(_Cla1funcsLoadEnd),
                        RUN_START(_Cla1funcsRunStart),
                        LOAD_SIZE(_Cla1funcsLoadSize),
                        ALIGN(8)
#endif

   CLADataLS0       : > RAMLS0

   Cla1ToCpuMsgRAM  : > CLA1_MSGRAMLOW, type=NOINIT
   CpuToCla1MsgRAM  : > CLA1_MSGRAMHIGH, type=NOINIT

   DLOG : > RAMGS0 | RAMGS1 | RAMGS2

   /* CLA C compiler sections */
   CLAscratch       :
                     { *.obj(CLAscratch)
                     . += CLA_SCRATCHPAD_SIZE;
                     *.obj(CLAscratch_end) } >  RAMLS0

   .scratchpad      : > RAMLS0
   .bss_cla         : > RAMLS0

#if defined(__TI_EABI__)
   .const_cla       :   LOAD = FLASH2,
                        RUN  = RAMLS0,
                        RUN_START(Cla1ConstRunStart),
                        LOAD_START(Cla1ConstLoadStart),
                        LOAD_SIZE(Cla1ConstLoadSize)
#else
   .const_cla       :   LOAD = FLASH2,
                        RUN  = RAMLS0,
                        RUN_START(_Cla1ConstRunStart),
                        LOAD_START(_Cla1ConstLoadStart),
                        LOAD_SIZE(_Cla1ConstLoadSize)
#endif

   #if defined(__TI_EABI__)
       .TI.ramfunc : {} LOAD = FLASH3,
                        RUN  = RAMD0,
                        LOAD_START(RamfuncsLoadStart),
                        LOAD_SIZE(RamfuncsLoadSize),
                        LOAD_END(RamfuncsLoadEnd),
                        RUN_START(RamfuncsRunStart),
                        RUN_SIZE(RamfuncsRunSize),
                        RUN_END(RamfuncsRunEnd),
                        ALIGN(8)
   #else
       .TI.ramfunc : {} LOAD = FLASH3,
                        RUN  = RAMD0,
                        LOAD_START(_RamfuncsLoadStart),
                        LOAD_SIZE(_RamfuncsLoadSize),
                        LOAD_END(_RamfuncsLoadEnd),
                        RUN_START(_RamfuncsRunStart),
                        RUN_SIZE(_RamfuncsRunSize),
                        RUN_END(_RamfuncsRunEnd),
                        ALIGN(8)
   #endif

}
