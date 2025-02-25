/*
 *      Interactive disassembler (IDA).
 *      Copyright (c) 1990-99 by Ilfak Guilfanov.
 *      ALL RIGHTS RESERVED.
 *                              E-mail: ig@datarescue.com
 *
 *
 */

#include "st7.hpp"
//#include <frame.hpp>

static int flow;
//------------------------------------------------------------------------
static void process_immediate_number(int n)
{
  doImmd(cmd.ea);
  if ( isDefArg(uFlag,n) ) return;
  switch ( cmd.itype )
  {
    case ST7_bres:
    case ST7_bset:
    case ST7_btjf:
    case ST7_btjt:
      op_dec(cmd.ea, n);
      break;
  }
}

//----------------------------------------------------------------------
ea_t calc_mem(ea_t ea)
{
  return toEA(cmd.cs, ea);
}

//----------------------------------------------------------------------
static void process_operand(op_t &x, bool isload)
{
  ea_t ea;
  switch ( x.type )
  {
    case o_reg:
    case o_phrase:
      break;

    case o_imm:
      if ( !isload ) interr("emu");
      process_immediate_number(x.n);
      if ( isOff(uFlag, x.n) ) ua_add_off_drefs(x, dr_O);
      break;

    case o_mem:
      if ( !is_forced_operand(cmd.ea, x.n) )
      {
        ea = calc_mem(x.addr);
        ua_dodata(ea, x.dtyp);
        dref_t dref = isload || (cmd.auxpref & aux_indir) ? dr_R : dr_W;
        ua_add_dref(x.offb, ea, dref);
      }
      break;

    case o_near:
      {
        cref_t ftype = fl_JN;
        ea = calc_mem(x.addr);
        if ( InstrIsSet(cmd.itype, CF_CALL) )
        {
          if ( !func_does_return(ea) )
            flow = false;
          ftype = fl_CN;
        }
        ua_add_cref(x.offb, ea, ftype);
      }
      break;

    case o_displ:
      process_immediate_number(x.n);
      if ( isOff(uFlag, x.n) && !is_forced_operand(cmd.ea, x.n) )
        ua_add_off_drefs(x, isload ? dr_R : dr_W);
      break;

    default:
      interr("emu");
  }
}


//----------------------------------------------------------------------
int emu(void)
{
  ulong Feature = cmd.get_canon_feature();

  flow = ((Feature & CF_STOP) == 0);

  if ( Feature & CF_USE1 ) process_operand(cmd.Op1, true);
  if ( Feature & CF_USE2 ) process_operand(cmd.Op2, true);
  if ( Feature & CF_USE3 ) process_operand(cmd.Op3, true);
  if ( Feature & CF_CHG1 ) process_operand(cmd.Op1, false);
  if ( Feature & CF_CHG2 ) process_operand(cmd.Op2, false);
  if ( Feature & CF_CHG3 ) process_operand(cmd.Op3, false);

//
//      Determine if the next instruction should be executed
//
  if ( segtype(cmd.ea) == SEG_XTRN ) flow = 0;
  if ( flow ) ua_add_cref(0,cmd.ea+cmd.size,fl_F);

  return 1;
}

//----------------------------------------------------------------------
int is_jump_func(const func_t * /*pfn*/, ea_t *jump_target)
{
  *jump_target = BADADDR;
  return 1; // means "no"
}

//----------------------------------------------------------------------
int may_be_func(void)           // can a function start here?
                                // arg: none, the instruction is in 'cmd'
                                // returns: probability 0..100
                                // 'cmd' structure is filled upon the entrace
                                // the idp module is allowed to modify 'cmd'
{
//  if ( cmd.itype == H8_push && isbp(cmd.Op1.reg) ) return 100;  // push.l er6
  return 0;
}

//----------------------------------------------------------------------
int is_sane_insn(int /*nocrefs*/)
{
  if ( cmd.itype == ST7_nop )
  {
    for ( int i=0; i < 8; i++ )
      if ( get_word(cmd.ea-i*2) != 0 ) return 1;
    return 0; // too many nops in a row
  }
  return 1;
}

//----------------------------------------------------------------------
int is_align_insn(ea_t ea)
{
  if ( !ua_ana0(ea) ) return 0;
  switch ( cmd.itype )
  {
    case ST7_nop:
      break;
    default:
      return 0;
  }
  return cmd.size;
}

