/* ldr.ld */

/* Entry Point */
ENTRY(Reset_Handler)


_estack = 0x20002000;
/* */
MEMORY
{
  /* sample flash, 512k */
  LDR (rx) : ORIGIN = 0x08000000, LENGTH = 0x00008000
  SHR_FLIP (rw) : ORIGIN = 0x08008000, LENGTH = 0x00008000
  /* sample RAM, 128k is enough for loader */
  RAM_USER   (rx) : ORIGIN = 0x20000000, LENGTH = 0x00020000
  /* Backup SRAM, safe against RDP2->RDP1 downgrade  */
  BKP_SRAM   (rw) : ORIGIN = 0x40024000, LENGTH = 0x00001000
  /* keybag storage, not upgradable through DFU */
  NOUPGRADE_SIG  (r) : ORIGIN = 0x08102000, LENGTH = 4*1023
  NOUPGRADE_DFU  (r) : ORIGIN = 0x08101000, LENGTH = 4*1023
  NOUPGRADE_AUTH (r) : ORIGIN = 0x08100000, LENGTH = 4*1023
}



/* Define output sections */
SECTIONS
{
  /* The startup code goes first into FLASH */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
  } >LDR

  /* The program code and other data goes into FLASH */
  .text :
  {
    _stext = .;	            /* create a global symbol at data start */
    *startup*(.text.Reset_Handler)
    *(.text*)
    *(.rodata*)         /* .rodata sections (constants, strings, etc.) */
    *(.glue_7*)         /* glue arm to thumb code */
    *(.glue_7t*)        /* glue thumb to arm code */
    *(.eh_frame*)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
    _exit = .;
  } >LDR

  /* used by the startup to initialize got */
  _sigot = .;
  .got : AT ( _sigot ) {
    . = ALIGN(4);
        _sgot = .;
        /*  *(.got.plt)
         *    We don't need plt segment
         *    since we do not need dynamic library relocation
         */
        *(.got)
    . = ALIGN(4);
    _egot = .;
    _sidata = .;
  } >LDR

  /* used by the startup to initialize data */

  /* Shared variables */
  .shared_flip :    {
    . = ALIGN(4);
    KEEP(*(.shared_flip)) ;
  }>SHR_FLIP

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data : AT ( _sidata )
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */

    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } >RAM_USER


  /* Uninitialized data section */
  . = ALIGN(4);
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss secion */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *debug.o(.bss)
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  } >RAM_USER AT >RAM_USER
  
    /* User_heap_stack section, used to check that there is enough RAM left */
  ._user_heap_stack :
  {
    . = ALIGN(4);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + 4096;
    . = ALIGN(4);
  } >RAM_USER AT >RAM_USER

  /* Uninitialized data section with explicit no zero init */
  ._non_zero_bss :
  {
    *(.nonzerobss)
  } >BKP_SRAM AT >BKP_SRAM

  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libgcc.a ( * )
    *(.ARM.exidx*)
    *(.gnu.linkonce.armexidx.*)
  }

  /* Helpers to get our keybag memory addresses */
  __noupgrade_sig_flash_start = ORIGIN(NOUPGRADE_SIG);
  __noupgrade_sig_flash_len = LENGTH(NOUPGRADE_SIG);
  __noupgrade_dfu_flash_start = ORIGIN(NOUPGRADE_DFU);
  __noupgrade_dfu_flash_len = LENGTH(NOUPGRADE_DFU);
  __noupgrade_auth_flash_start = ORIGIN(NOUPGRADE_AUTH);
  __noupgrade_auth_flash_len = LENGTH(NOUPGRADE_AUTH);
}
