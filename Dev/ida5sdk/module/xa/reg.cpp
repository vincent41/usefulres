/*
 *      Interactive disassembler (IDA).
 *      Copyright (c) 1990-98 by Ilfak Guilfanov.
 *      ALL RIGHTS RESERVED.
 *                              E-mail: ig@estar.msk.su, ig@datarescue.com
 *                              FIDO:   2:5020/209
 *
 */

#include "xa.hpp"
#include <entry.hpp>
#include <srarea.hpp>

//--------------------------------------------------------------------------
processor_subtype_t ptype;

static char *RegNames[] =
{
  "R0L", "R0H", "R1L", "R1H", "R2L", "R2H", "R3L", "R3H",
  "R4L", "R4H", "R5L", "R5H", "R6L", "R6H", "R7L", "R7H",
  "R0",  "R1",  "R2",  "R3",  "R4",  "R5",  "R6",  "R7",
  "R8",  "R9",  "R10",  "R11",  "R12",  "R13",  "R14",  "R15",
  "A", "DPTR", "C", "PC", "USP",
  "CS","DS", "ES",
};

//----------------------------------------------------------------------

predefined_t iregs[] = {
  { prc_xaG3,  0x6A, 0, "BCR"     ,  "Bus configuration register" },
  { prc_xaG3,  0x69, 0, "BTRH"    ,  "Bus timing register high byte" },
  { prc_xaG3,  0x68, 0, "BTRL"    ,  "Bus timing register low byte" },
  { prc_xaG3,  0x43, 0, "CS"      ,  "Code segment" },
  { prc_xaG3,  0x41, 0, "DS"      ,  "Data segment" },
  { prc_xaG3,  0x42, 0, "ES"      ,  "Extra segment" },
  { prc_xaG3,  0x27, 0, "IEH"     ,  "Interrupt enable high byte" },
  { prc_xaG3,  0x26, 0, "IEL"     ,  "Interrupt enable low byte" },
  { prc_xaG3,  0xa0, 0, "IPA0"    ,  "Interrupt priority 0" },
  { prc_xaG3,  0xa1, 0, "IPA1"    ,  "Interrupt priority 1" },
  { prc_xaG3,  0xa2, 0, "IPA2"    ,  "Interrupt priority 2" },
  { prc_xaG3,  0xa3, 0, "IPA3"    ,  "Interrupt priority 3" },
  { prc_xaG3,  0xa4, 0, "IPA4"    ,  "Interrupt priority 4" },
  { prc_xaG3,  0xa5, 0, "IPA5"    ,  "Interrupt priority 5" },
  { prc_xaG3,  0x30, 0, "P0"      ,  "Port 0" },
  { prc_xaG3,  0x31, 0, "P1"      ,  "Port 1" },
  { prc_xaG3,  0x32, 0, "P2"      ,  "Port 2" },
  { prc_xaG3,  0x33, 0, "P3"      ,  "Port 3" },
  { prc_xaG3,  0x70, 0, "P0CFGA"  ,  "Port 0 configuration A" },
  { prc_xaG3,  0x71, 0, "P1CFGA"  ,  "Port 1 configuration A" },
  { prc_xaG3,  0x72, 0, "P2CFGA"  ,  "Port 2 configuration A" },
  { prc_xaG3,  0x73, 0, "P3CFGA"  ,  "Port 3 configuration A" },
  { prc_xaG3,  0xF0, 0, "P0CFGB"  ,  "Port 0 configuration B" },
  { prc_xaG3,  0xF1, 0, "P1CFGB"  ,  "Port 1 configuration B" },
  { prc_xaG3,  0xF2, 0, "P2CFGB"  ,  "Port 2 configuration B" },
  { prc_xaG3,  0xF3, 0, "P3CFGB"  ,  "Port 3 configuration B" },
  { prc_xaG3,  0x04, 0, "PCON"    ,  "Power control register" },
  { prc_xaG3,  0x01, 0, "PSWH"    ,  "Program status word high byte" },
  { prc_xaG3,  0x00, 0, "PSWL"    ,  "Program status word low byte" },
  { prc_xaG3,  0x02, 0, "PSW51"   ,  "8051 compatible PSW" },
  { prc_xaG3,  0x55, 0, "RTH0"    ,  "Timer 0 extender reload high byte" },
  { prc_xaG3,  0x57, 0, "RTH1"    ,  "Timer 1 extender reload high byte" },
  { prc_xaG3,  0x54, 0, "RTL0"    ,  "Timer 0 extender reload low byte" },
  { prc_xaG3,  0x56, 0, "RTL1"    ,  "Timer 1 extender reload low byte" },
  { prc_xaG3,  0x20, 0, "S0CON"   ,  "Serial port 0 control register" },
  { prc_xaG3,  0x21, 0, "S0STAT"  ,  "Serial port 0 extended status" },
  { prc_xaG3,  0x60, 0, "S0BUF"   ,  "Serial port 0 buffer register" },
  { prc_xaG3,  0x61, 0, "S0ADDR"  ,  "Serial port 0 address register" },
  { prc_xaG3,  0x62, 0, "S0ADEN"  ,  "Serial port 0 address enable" },
  { prc_xaG3,  0x24, 0, "S1CON"   ,  "Serial port 1 control register" },
  { prc_xaG3,  0x25, 0, "S1STAT"  ,  "Serial port 1 extended status" },
  { prc_xaG3,  0x64, 0, "S1BUF"   ,  "Serial port 1 buffer register" },
  { prc_xaG3,  0x65, 0, "S1ADDR"  ,  "Serial port 1 address register" },
  { prc_xaG3,  0x66, 0, "S1ADEN"  ,  "Serial port 1 address enable" },
  { prc_xaG3,  0x40, 0, "SCR"     ,  "System configuration register" },
  { prc_xaG3,  0x03, 0, "SSEL"    ,  "Segment selection register" },
  { prc_xaG3,  0x7A, 0, "SWE"     ,  "Software interrupt enable" },
  { prc_xaG3,  0x2A, 0, "SWR"     ,  "Software interrupt reguest" },
  { prc_xaG3,  0x18, 0, "T2CON"   ,  "Timer 2 control register" },
  { prc_xaG3,  0x19, 0, "T2MOD"   ,  "Timer 2 mode control" },
  { prc_xaG3,  0x59, 0, "TH2"     ,  "Timer 2 high byte" },
  { prc_xaG3,  0x58, 0, "TL2"     ,  "Timer 2 low byte" },
  { prc_xaG3,  0x5B, 0, "T2CAPH"  ,  "Timer 2 capture register high byte" },
  { prc_xaG3,  0x5A, 0, "T2CAPL"  ,  "Timer 2 capture register low byte" },
  { prc_xaG3,  0x10, 0, "TCON"    ,  "Timer 0 and 1 control register" },
  { prc_xaG3,  0x51, 0, "TH0"     ,  "Timer 0 high byte" },
  { prc_xaG3,  0x53, 0, "TH1"     ,  "Timer 1 high byte" },
  { prc_xaG3,  0x50, 0, "TL0"     ,  "Timer 0 low byte" },
  { prc_xaG3,  0x52, 0, "TL1"     ,  "Timer 1 low byte" },
  { prc_xaG3,  0x5C, 0, "TMOD"    ,  "Timer 0 and 1 mode control" },
  { prc_xaG3,  0x11, 0, "TSTAT"   ,  "Timer 0 and 1 extended status" },
  { prc_xaG3,  0x1F, 0, "WDCON"   ,  "Watchdog control register" },
  { prc_xaG3,  0x5F, 0, "WDL"     ,  "Watchdog timer reload" },
  { prc_xaG3,  0x5D, 0, "WFEED1"  ,  "Watchdog feed 1" },
  { prc_xaG3,  0x5E, 0, "WFEED2"  ,  "Watchdog feed 2" },
  { prc_xaG3,  0x00, 0, NULL      ,  NULL }
};

predefined_t ibits[] =
{
  { prc_xaG3, 0x427, 0, "ERI0"    ,  "IEH.0 - " },
  { prc_xaG3, 0x427, 1, "ETI0"    ,  "IEH.1 - " },
  { prc_xaG3, 0x427, 2, "ERI1"    ,  "IEH.2 - " },
  { prc_xaG3, 0x427, 3, "ETI1"    ,  "IEH.3 - " },
  { prc_xaG3, 0x426, 0, "EX0"     ,  "IEL.0 - " },
  { prc_xaG3, 0x426, 1, "ET0"     ,  "IEL.1 - " },
  { prc_xaG3, 0x426, 2, "EX1"     ,  "IEL.2 - " },
  { prc_xaG3, 0x426, 3, "ET1"     ,  "IEL.3 - " },
  { prc_xaG3, 0x426, 4, "ET2"     ,  "IEL.4 - " },
  { prc_xaG3, 0x426, 7, "EA"      ,  "IEL.7 - " },
  { prc_xaG3, 0x426, 7, "EA"      ,  "IEL.7 - " },
  { prc_xaG3, 0x404, 0, "IDL"     ,  "PCON.0 - Idle Mode Bit" },
  { prc_xaG3, 0x404, 1, "PD"      ,  "PCON.1 - Power Down Mode Bit" },
  { prc_xaG3, 0x401, 0, "IM0"     ,  "PSWH.0 - Interrupt mask 0" },
  { prc_xaG3, 0x401, 1, "IM1"     ,  "PSWH.1 - Interrupt mask 1" },
  { prc_xaG3, 0x401, 2, "IM2"     ,  "PSWH.2 - Interrupt mask 2" },
  { prc_xaG3, 0x401, 3, "IM3"     ,  "PSWH.3 - Interrupt mask 3" },
  { prc_xaG3, 0x401, 4, "RS0"     ,  "PSWH.4 - Register select 0" },
  { prc_xaG3, 0x401, 5, "RS1"     ,  "PSWH.5 - Register select 1" },
  { prc_xaG3, 0x401, 6, "TM"      ,  "PSWH.6 - Trace mode" },
  { prc_xaG3, 0x401, 7, "SM"      ,  "PSWH.7 - System mode" },
  { prc_xaG3, 0x400, 0, "Z"       ,  "PSWL.0 - Zero flag" },
  { prc_xaG3, 0x400, 1, "N"       ,  "PSWL.1 - Negative flag" },
  { prc_xaG3, 0x400, 2, "V"       ,  "PSWL.2 - Overflow flag" },
  { prc_xaG3, 0x400, 6, "AC"      ,  "PSWL.6 - Auxiliary carry flag" },
  { prc_xaG3, 0x400, 7, "CY"      ,  "PSWL.7 - Carry flag" },
  { prc_xaG3, 0x420, 0, "RI_0"    ,  "S0CON.0 -" },
  { prc_xaG3, 0x420, 1, "TI_0"    ,  "S0CON.1 -" },
  { prc_xaG3, 0x420, 2, "RB8_0"   ,  "S0CON.2 -" },
  { prc_xaG3, 0x420, 3, "TB8_0"   ,  "S0CON.3 -" },
  { prc_xaG3, 0x420, 4, "REN_0"   ,  "S0CON.4 -" },
  { prc_xaG3, 0x420, 5, "SM2_0"   ,  "S0CON.5 -" },
  { prc_xaG3, 0x420, 6, "SM1_0"   ,  "S0CON.6 -" },
  { prc_xaG3, 0x420, 7, "SM0_0"   ,  "S0CON.7 -" },
  { prc_xaG3, 0x421, 0, "STINT0"  ,  "S0STAT.0 -" },
  { prc_xaG3, 0x421, 1, "OE0"     ,  "S0STAT.1 -" },
  { prc_xaG3, 0x421, 2, "BR0"     ,  "S0STAT.2 -" },
  { prc_xaG3, 0x421, 3, "FE0"     ,  "S0STAT.3 -" },
  { prc_xaG3, 0x424, 0, "RI_1"    ,  "S1CON.0 -" },
  { prc_xaG3, 0x424, 1, "TI_1"    ,  "S1CON.1 -" },
  { prc_xaG3, 0x424, 2, "RB8_1"   ,  "S1CON.2 -" },
  { prc_xaG3, 0x424, 3, "TB8_1"   ,  "S1CON.3 -" },
  { prc_xaG3, 0x424, 4, "REN_1"   ,  "S1CON.4 -" },
  { prc_xaG3, 0x424, 5, "SM2_1"   ,  "S1CON.5 -" },
  { prc_xaG3, 0x424, 6, "SM1_1"   ,  "S1CON.6 -" },
  { prc_xaG3, 0x424, 7, "SM0_1"   ,  "S1CON.7 -" },
  { prc_xaG3, 0x425, 0, "STINT1"  ,  "S1STAT.0 -" },
  { prc_xaG3, 0x425, 1, "OE1"     ,  "S1STAT.1 -" },
  { prc_xaG3, 0x425, 2, "BR1"     ,  "S1STAT.2 -" },
  { prc_xaG3, 0x425, 3, "FE1"     ,  "S1STAT.3 -" },
  { prc_xaG3, 0x403, 0, "R0SEG"   ,  "SSEL.0 -" },
  { prc_xaG3, 0x403, 1, "R1SEG"   ,  "SSEL.1 -" },
  { prc_xaG3, 0x403, 2, "R2SEG"   ,  "SSEL.2 -" },
  { prc_xaG3, 0x403, 3, "R3SEG"   ,  "SSEL.3 -" },
  { prc_xaG3, 0x403, 4, "R4SEG"   ,  "SSEL.4 -" },
  { prc_xaG3, 0x403, 5, "R5SEG"   ,  "SSEL.5 -" },
  { prc_xaG3, 0x403, 6, "R6SEG"   ,  "SSEL.6 -" },
  { prc_xaG3, 0x403, 7, "ESWEN"   ,  "SSEL.7 -" },
  { prc_xaG3, 0x42A, 0, "SWR1"    ,  "SWR.0 -" },
  { prc_xaG3, 0x42A, 1, "SWR2"    ,  "SWR.1 -" },
  { prc_xaG3, 0x42A, 2, "SWR3"    ,  "SWR.2 -" },
  { prc_xaG3, 0x42A, 3, "SWR4"    ,  "SWR.3 -" },
  { prc_xaG3, 0x42A, 4, "SWR5"    ,  "SWR.4 -" },
  { prc_xaG3, 0x42A, 5, "SWR6"    ,  "SWR.5 -" },
  { prc_xaG3, 0x42A, 6, "SWR7"    ,  "SWR.6 -" },
  { prc_xaG3, 0x418, 0, "CPRL2"   ,  "T2CON.0 -" },
  { prc_xaG3, 0x418, 1, "CT2"     ,  "T2CON.1 -" },
  { prc_xaG3, 0x418, 2, "TR2"     ,  "T2CON.2 -" },
  { prc_xaG3, 0x418, 3, "EXEN2"   ,  "T2CON.3 -" },
  { prc_xaG3, 0x418, 4, "TCLK0"   ,  "T2CON.4 -" },
  { prc_xaG3, 0x418, 5, "RCLK0"   ,  "T2CON.5 -" },
  { prc_xaG3, 0x418, 6, "EXF2"    ,  "T2CON.6 -" },
  { prc_xaG3, 0x418, 7, "TF2"     ,  "T2CON.7 -" },
  { prc_xaG3, 0x419, 0, "DCEN"    ,  "T2MOD.0 -" },
  { prc_xaG3, 0x419, 1, "T2OE"    ,  "T2MOD.1 -" },
  { prc_xaG3, 0x419, 2, "T2RD"    ,  "T2MOD.2 -" },
  { prc_xaG3, 0x419, 4, "TCLK1"   ,  "T2MOD.4 -" },
  { prc_xaG3, 0x419, 5, "RCLK1"   ,  "T2MOD.5 -" },
  { prc_xaG3, 0x410, 0, "IT0"     ,  "TCON.0 -" },
  { prc_xaG3, 0x410, 1, "IE0"     ,  "TCON.1 -" },
  { prc_xaG3, 0x410, 2, "IT1"     ,  "TCON.2 -" },
  { prc_xaG3, 0x410, 3, "IE1"     ,  "TCON.3 -" },
  { prc_xaG3, 0x410, 4, "TR0"     ,  "TCON.4 -" },
  { prc_xaG3, 0x410, 6, "TF0"     ,  "TCON.5 -" },
  { prc_xaG3, 0x410, 6, "TR1"     ,  "TCON.6 -" },
  { prc_xaG3, 0x410, 7, "TF1"     ,  "TCON.7 -" },
  { prc_xaG3, 0x411, 0, "T0OE"    ,  "TSTAT.0 -" },
  { prc_xaG3, 0x411, 1, "T0RD"    ,  "TSTAT.1 -" },
  { prc_xaG3, 0x411, 2, "T1OE"    ,  "TSTAT.2 -" },
  { prc_xaG3, 0x411, 3, "T1RD"    ,  "TSTAT.3 -" },
  { prc_xaG3, 0x41f, 1, "WDTOF"   ,  "WDCON.1 -" },
  { prc_xaG3, 0x41f, 2, "WDRUN"   ,  "WDCON.2 -" },
  { prc_xaG3, 0x41f, 5, "PRE0"    ,  "WDCON.5 -" },
  { prc_xaG3, 0x41f, 6, "PRE1"    ,  "WDCON.6 -" },
  { prc_xaG3, 0x41f, 7, "PRE2"    ,  "WDCON.7 -" },
  { prc_xaG3, 0x000, 0, NULL      ,  NULL }
};

//----------------------------------------------------------------------
int IsPredefined(const char *name)
{
  predefined_t *ptr;

  for ( ptr = ibits; ptr->name != NULL; ptr++ )
    if ( strcmp(ptr->name,name) == 0 ) return 1;

  for ( ptr = iregs; ptr->name != NULL; ptr++ )
    if ( strcmp(ptr->name,name) == 0 ) return 1;

  return 0;
}

//----------------------------------------------------------------------
predefined_t *GetPredefined(predefined_t *ptr,int addr,int bit)
{
  for ( ; ptr->name != NULL; ptr++ )
  {
    if ( ptr->proc != ptype ) continue;
    if ( addr == ptr->addr && bit == ptr->bit )
      return ptr;
  }
  return NULL;
}

typedef struct
{
  char proc;
  ulong off;
  char *name;
  char *cmt;
} entry_t;

static entry_t entries[] =
{
  { prc_xaG3,  0x00, "Reset", "Reset (h/w, watchdog, s/w)" },
  { prc_xaG3,  0x04, "Breakpoint", "Breakpoint instruction (h/w trap 1)" },
  { prc_xaG3,  0x08, "Trace", "Trace (h/w trap 2)" },
  { prc_xaG3,  0x0C, "StackOverflow", "Stack Overflow (h/w trap 3)" },
  { prc_xaG3,  0x10, "DivBy0",  "Divide by 0 (h/w trap 4)" },
  { prc_xaG3,  0x14, "UserRETI", "User RETI (h/w trap 5)" },
  { prc_xaG3,  0x40, "Trap0",  "Software TRAP 0" },
  { prc_xaG3,  0x44, "Trap1",  "Software TRAP 1" },
  { prc_xaG3,  0x48, "Trap2",  "Software TRAP 2" },
  { prc_xaG3,  0x4C, "Trap3",  "Software TRAP 3" },
  { prc_xaG3,  0x50, "Trap4",  "Software TRAP 4" },
  { prc_xaG3,  0x54, "Trap5",  "Software TRAP 5" },
  { prc_xaG3,  0x58, "Trap6",  "Software TRAP 6" },
  { prc_xaG3,  0x5C, "Trap7",  "Software TRAP 7" },
  { prc_xaG3,  0x60, "Trap8",  "Software TRAP 8" },
  { prc_xaG3,  0x64, "Trap9",  "Software TRAP 9" },
  { prc_xaG3,  0x68, "Trap10",  "Software TRAP 10" },
  { prc_xaG3,  0x6C, "Trap11",  "Software TRAP 11" },
  { prc_xaG3,  0x70, "Trap12",  "Software TRAP 12" },
  { prc_xaG3,  0x74, "Trap13",  "Software TRAP 13" },
  { prc_xaG3,  0x78, "Trap14",  "Software TRAP 14" },
  { prc_xaG3,  0x7C, "Trap15",  "Software TRAP 15" },
  { prc_xaG3,  0x80, "ExtInt0",  "External Interrupt 0" },
  { prc_xaG3,  0x84, "TimerInt0",  "Timer 0 Interrupt" },
  { prc_xaG3,  0x88, "ExtInt1",  "External Interrupt 1" },
  { prc_xaG3,  0x8C, "TimerInt1",  "Timer 1 Interrupt" },
  { prc_xaG3,  0x90, "TimerInt2",  "Timer 2 Interrupt" },
  { prc_xaG3,  0xA0, "SerIntRx0",  "Serial port 0 Rx" },
  { prc_xaG3,  0xA4, "SerIntTx0",  "Serial port 0 Tx" },
  { prc_xaG3,  0xA8, "SerIntRx1",  "Serial port 1 Rx" },
  { prc_xaG3,  0xAC, "SerIntTx1",  "Serial port 1 Tx" },
  { prc_xaG3, 0x100, "SWI1",  "Software Interrupt 1" },
  { prc_xaG3, 0x104, "SWI2",  "Software Interrupt 2" },
  { prc_xaG3, 0x108, "SWI3",  "Software Interrupt 3" },
  { prc_xaG3, 0x10C, "SWI4",  "Software Interrupt 4" },
  { prc_xaG3, 0x110, "SWI5",  "Software Interrupt 5" },
  { prc_xaG3, 0x114, "SWI6",  "Software Interrupt 6" },
  { prc_xaG3, 0x118, "SWI7",  "Software Interrupt 7" },
};

//----------------------------------------------------------------------
// The kernel event notifications
// Here you may take desired actions upon some kernel events

static int notify(processor_t::idp_notify msgid, ...)
{
  va_list va;
  va_start(va, msgid);

// A well behaving processor module should call invoke_callbacks()
// in his notify() function. If this function returns 0, then
// the processor module should process the notification itself
// Otherwise the code should be returned to the caller:

  int code = invoke_callbacks(HT_IDP, msgid, va);
  if ( code ) return code;

  switch(msgid)
  {
    case processor_t::init:
      inf.mf = 0;       // Set a little endian mode of the IDA kernel
      break;

    case processor_t::newfile:
      {
        segment_t *sptr = getnseg(0);
        if ( sptr != NULL )
        {
          if ( sptr->startEA-get_segm_base(sptr) == 0 )
          {
            inf.beginEA = sptr->startEA;
            inf.startIP = BADADDR;
            if (sptr->size() > 0x10000)
            {
               ea_t endEA = sptr->endEA;
               ea_t startEA = sptr->startEA;

               if (endEA & 0xFFFF)
               {
                  ea_t start = endEA & 0xFFFF0000;
                  add_segm(start >> 4, start, endEA, "ROMxx", "CODE");
                  endEA = start;
               }
               for (ea_t start = endEA - 0x10000; start>startEA; endEA -= 0x10000, start -= 0x10000)
               {
                 add_segm(start>>4, start, start+0x10000, NULL, "CODE");
                 sptr = getseg(start);
                 set_segm_name(sptr, "ROM%02x", start>>16);
               }
               sptr = getseg(startEA);
               set_segm_name(sptr, "ROM00");
               set_segm_class(sptr, "CODE");
            }
            for ( int i=0; i < qnumber(entries); i++ )
            {
              if ( entries[i].proc > ptype ) continue;
              ea_t ea = inf.beginEA+entries[i].off;
              if ( isEnabled(ea) && get_byte(ea) == 0 )
              {
                add_entry(ea, ea, entries[i].name, 0);
                doWord(ea, 2);
                doWord(ea+2, 2);
                op_offset(ea+2, 0, REF_OFF16, get_word(ea+2));
                ua_add_cref(ea+2, get_word(ea+2), fl_CN);
                set_cmt(ea, entries[i].cmt, 1);
              }
            }
          }
        }

        add_segm(INTMEMBASE>>4, INTMEMBASE, INTMEMBASE+0x400, "INTMEM", "DATA");
        add_segm(INTMEMBASE>>4, SFRBASE, SFRBASE+0x400, "SFR", "DATA");

        // the default data segment will be INTMEM
        set_default_dataseg(getseg(INTMEMBASE)->sel);

        predefined_t *ptr;
        for ( ptr=iregs; ptr->name != NULL; ptr++ )
        {
          ea_t ea = SFRBASE + ptr->addr;
          ea_t oldea = get_name_ea(BADADDR, ptr->name);
          if ( oldea != ea )
          {
            if ( oldea != BADADDR ) del_global_name(oldea);
            doByte(ea, 1);
            set_name(ea, ptr->name, 0);
          }
          if ( ptr->cmt != NULL ) set_cmt(ea, ptr->cmt, 0);
        }

        // Perform the final pass of analysis even for the binary files
        if ( inf.filetype == f_BIN )
          inf.af |= AF_FINAL;
      }
      break;

    case processor_t::oldfile:
      break;

    case processor_t::newseg:
        // make the default DS point to INTMEM
      {
        segment_t *newseg = va_arg(va, segment_t *);
        segment_t *intseg = getseg(INTMEMBASE);
        if ( intseg != NULL )
          newseg->defsr[rDS-ph.regFirstSreg] = intseg->sel;
      }
      break;

    case processor_t::newprc:
      {
        processor_subtype_t prcnum = processor_subtype_t(va_arg(va, int));
        ptype = prcnum;
      }
      break;

    default:
      break;
  }
  va_end(va);

  return(1);
}

//-----------------------------------------------------------------------
//      Checkarg data. Common for all assemblers. Not good.
//
//      What is checkarg?
//        It is a possibilty to compare the value of a manually entered
//        operand against its original value.
//        Checkarg is currently implemented for IBM PC, 8051, and PDP-11
//        processors. Other processor are unlikely to be supported.
//      You may just get rid of checkarg and replace the pointers to it
//      in the 'LPH' structure by NULLs.
//
//-----------------------------------------------------------------------
static const char *operdim[15] = {  // ������ � ������ 15
     "(", ")", "!", "-", "+", "%",
     "\\", "/", "*", "&", "|", "^", "<<", ">>", NULL};

inline int pere(int c) { return(c&0xFF); }

static int preline(char *argstr, s_preline *S)
{
    char    *pc;
    int     *ind;
    char    *prefix;
    char    *seg;
    char    *reg;
    char    *offset;

    if(!argstr) return(-1); // request for default selector

    ind     = S->ind;
    prefix  = S->prefix;
    seg     = S->seg;
    reg     = S->reg;
    offset  = S->offset;
    *ind = 0;
    *prefix = '\0';
    *reg = '\0';
    *seg = '\0';
    *offset = '\0';

    pc = argstr;
    if (*pc == '#') ++*ind, pc++;
    qstrncpy(offset, pc, PRELINE_SIZE);

    return(0);

} /* preline */

//
//              Definitions of the target assemblers
//              8051 has unusually many of them.
//

//-----------------------------------------------------------------------
//                   XA Assembler by Macraigor Systems
//-----------------------------------------------------------------------
static asm_t xaasm = {
  AS_COLON | ASH_HEXF0 ,
  UAS_PBIT | UAS_SECT,
  "XA Macro Assembler dummy entry",
  0,
  NULL,         // no headers
  NULL,         // no bad instructions
  "org",
  "end",

  ";",          // comment string
  '"',          // string delimiter
  '\'',         // char delimiter
  "\\\"'",      // special symbols in char and string constants

  "db",         // ascii string directive
  "db",         // byte directive
  "dw",         // word directive
  "long",       // dword  (4 bytes)
  NULL,         // qword  (8 bytes)
  NULL,         // oword  (16 bytes)
  NULL,         // float  (4 bytes)
  NULL,         // double (8 bytes)
  NULL,         // tbyte  (10/12 bytes)
  NULL,         // packed decimal real
  "#d dup(#v)",         // arrays (#h,#d,#v,#s(...)
  "ds %s",      // uninited arrays
  "reg",        // equ
  NULL,         // seg prefix
  preline, NULL, operdim,
  NULL,
  "$",
  NULL,		// func_header
  NULL,		// func_footer
  NULL,		// public
  NULL,		// weak
  NULL,		// extrn
  NULL,		// comm
  NULL,		// get_type_name
  "align",		// align
  '(', ')',	// lbrace, rbrace
  NULL,    // mod
  "&",    // and
  "|",    // or
  "^",    // xor
  "not",    // not
  "shl",    // shl
  "shr",    // shr
  NULL,    // sizeof
};

static asm_t *asms[] = { &xaasm, NULL };
//-----------------------------------------------------------------------
// The short and long names of the supported processors
// The short names must match
// the names in the module DESCRIPTION in the makefile (the
// description is copied in the offset 0x80 in the result DLL)

static char *shnames[] =
{
  "51XA-G3",
  NULL
};

static char *lnames[] =
{
  "Philips 51XA G3",
  NULL
};

//--------------------------------------------------------------------------
// Opcodes of "return" instructions. This information will be used in 2 ways:
//      - if an instruction has the "return" opcode, its autogenerated label
//        will be "locret" rather than "loc".
//      - IDA will use the first "return" opcode to create empty subroutines.

static uchar retcode_1[] = { 0xd6, 0x80 };
static uchar retcode_2[] = { 0xd6, 0x90 };

static bytes_t retcodes[] = {
 { sizeof(retcode_1), retcode_1 },
 { sizeof(retcode_2), retcode_2 },
 { 0, NULL }                            // NULL terminated array
};

//-----------------------------------------------------------------------
//      Processor Definition
//-----------------------------------------------------------------------
processor_t LPH =
{
  IDP_INTERFACE_VERSION,// version
  0x8051,               // id
  PR_SEGS|PR_RNAMESOK|  // can use register names for byte names
  PR_BINMEM,		// The module creates RAM/ROM segments for binary files
  8,                    // 8 bits in a byte for code segments
  8,                    // 8 bits in a byte for other segments

  shnames,              // array of short processor names
                        // the short names are used to specify the processor
                        // with the -p command line switch)
  lnames,               // array of long processor names
                        // the long names are used to build the processor type
                        // selection menu

  asms,                 // array of target assemblers

  notify,               // the kernel event notification callback

  header,               // generate the disassembly header
  footer,               // generate the disassembly footer

  segstart,             // generate a segment declaration (start of segment)
  std_gen_segm_footer,  // generate a segment footer (end of segment)

  NULL,                 // generate 'assume' directives

  ana,                  // analyze an instruction and fill the 'cmd' structure
  emu,                  // emulate an instruction

  out,                  // generate a text representation of an instruction
  outop,                // generate a text representation of an operand
  xa_data,             // generate a text representation of a data item
  NULL,                 // compare operands
  NULL,                 // can an operand have a type?

  qnumber(RegNames),    // Number of registers
  RegNames,             // Regsiter names
  NULL,                 // get abstract register

  0,                    // Number of register files
  NULL,                 // Register file names
  NULL,                 // Register descriptions
  NULL,                 // Pointer to CPU registers

  rCS,rES,
  1,                    // size of a segment register
  rCS,rDS,

  NULL,                 // No known code start sequences
  retcodes,

  0,XA_last,
  Instructions,
  NULL,			// is_far_jump
  NULL,			// translate
  0,			// tbyte_size
  NULL,			// realcvt
  {0,0,0,0},		// real_width
  xa_is_switch,		// is_switch
  NULL,			// gen_map_file
  NULL,			// extract_address
  NULL,			// is_sp_based
  xa_create_func,	// create_func_frame
  xa_frame_retsize,	// get_frame_retsize
  xa_stkvar_def,	// gen_stkvar_def
  NULL,			// u_outspec
  XA_ret,		// icode_return
  NULL,			// set_idp_options
  xa_align_insn,			// is_align_insn

};
