/*
 * 	National Semiconductor Corporation CR16 processor module for IDA Pro.
 *	Copyright (c) 2002-2006 Konstantin Norvatoff, <konnor@bk.ru>
 *	Freeware.
 */

#include "cr16.hpp"

static bool flow;        // 䫠��� �⮯�
//----------------------------------------------------------------------
// ���⠢�� �ᯮ�짮�����/��������� ���࠭���
static void near TouchArg(op_t &x,int isAlt,int isload)
{
ea_t ea = toEA(codeSeg(x.addr,x.n), x.addr);
  switch ( x.type ) {
  // �� ���� �� �ᯮ������ !
  case o_void:  break;
  // ��� ⮦� ��祣� ������
  case o_reg:           break;

  // ��� ⮦� ��祣� ������
  case o_displ:           break;

  // �����।�⢥��� ���࠭�
  case o_imm:   // �����।�⢥��� �� ����� ��������
                if ( ! isload ) goto badTouch;
                // ���⠢�� 䫠��� �����।�⢥����� ���࠭��
                doImmd(cmd.ea);
                // �᫨ �� ���஢�� � ����祭 ᬥ饭���
                if ( !isAlt && isOff(uFlag,x.n) )
                        // �� ᬥ饭�� !
						ua_add_dref(x.offb,ea,dr_O);
                break;

  // ���室 ��� �맮�
  case o_near:	// �� �맮� ? (��� ���室)
				if(InstrIsSet(cmd.itype,CF_CALL)){
					// ���⠢�� ��뫪� �� ���
					ua_add_cref(x.offb,ea,fl_CN);
					// �� �㭪�� ��� ������ ?
#if IDP_INTERFACE_VERSION > 37
					flow = func_does_return(ea);
#else
                    // ����稬 ����⥫� �㭪樨
					func_t *pfn = get_func(ea);
					// �᫨ �㭪�� ���ᠭ� � �� ����� ������ - ��⠭����
					if(pfn != NULL && (pfn->flags & FUNC_NORET) ) flow = false;
#endif
                }
				else ua_add_cref(x.offb,ea,fl_JN);
                break;

  // ��뫪� �� �祩�� �����
  case o_mem:	// ᤥ���� ����� �� 㪠������� �����
				ua_dodata(ea, x.dtyp);
				// �᫨ ��������� - ���⠢�� ��६�����
				if ( ! isload ) doVar(ea);
				// ������� ��뫪� �� ������
				ua_add_dref(x.offb,ea,isload ? dr_R : dr_W);
                break;

  // ��祥 - ᮮ�騬 �訡��
  default:
badTouch:
#if IDP_INTERFACE_VERSION > 37
		warning("%a %s,%d: bad optype %d",
				cmd.ea, cmd.get_canon_mnem(),
#else
		warning("%08lX %s,%d: bad optype (%x)",
				cmd.ea,(char far *)Instructions[cmd.itype].name,
#endif
				x.n, x.type);
    break;
  }
}

//----------------------------------------------------------------------
// ������
int idaapi CR16_emu(void)
{
#if IDP_INTERFACE_VERSION > 37
ulong Feature = cmd.get_canon_feature();
#else
ulong Feature = Instructions[cmd.itype].feature;
uFlag = getFlags(cmd.ea);
#endif
  // ����稬 ⨯� ���࠭���
  int flag1 = is_forced_operand(cmd.ea, 0);
  int flag2 = is_forced_operand(cmd.ea, 1);

  flow = ((Feature & CF_STOP) == 0);

  // ����⨬ ��뫪� ���� ���࠭���
  if(Feature & CF_USE1) TouchArg(cmd.Op1, flag1, 1);
  if(Feature & CF_USE2) TouchArg(cmd.Op2, flag2, 1);
  // ���⠢�� ���室 � ��।�
  if(Feature & CF_JUMP) QueueMark(Q_jumps,cmd.ea);

  // ���⠢�� ���������
  if(Feature & CF_CHG1) TouchArg(cmd.Op1, flag1, 0);
  if(Feature & CF_CHG2) TouchArg(cmd.Op2, flag2, 0);
  // �᫨ �� �⮯ - �த����� �� ᫥�. ������樨
  if(flow) ua_add_cref(0,cmd.ea+cmd.size,fl_F);

  return(1);
}
