/*
 *      Interactive disassembler (IDA).
 *      Copyright (c) 1990-2001 by Ilfak Guilfanov.
 *      ALL RIGHTS RESERVED.
 *                              E-mail: ig@datarescue.com
 *
 *
 */

#include "78k_0s.hpp"

//----------------------------------------------------------------------
inline ulong sfr(uchar sfr_offset)
{
return 0xFF00+sfr_offset;
}
//----------------------------------------------------------------------
inline ulong saddr(uchar Saddr_offset)
{
if(Saddr_offset<0x20)
  {
  return 0xFF00+Saddr_offset;
  }
else  return 0xFE00+Saddr_offset;
}
//----------------------------------------------------------------------
inline void addr16(op_t &x)
{
//x.offb = cmd.size;
ulong low  = ua_next_byte();
ulong high = ua_next_byte();

//x.type = o_near;
x.addr = low | (high<<8);
x.addr16=1;
}
//----------------------------------------------------------------------
inline void jdisp(op_t &x, uchar addr, int previons_len)
{
x.type = o_near;
x.addr = cmd.ip + (signed char)addr + previons_len;
//x.offb = cmd.size - 1;
}
//----------------------------------------------------------------------
inline void imm16(op_t &x)
{
x.type  = o_imm;
x.dtyp  = dt_word;
x.value = ua_next_byte() | (ua_next_byte()<<8);
}
//----------------------------------------------------------------------
int ana(void) 
{
//������� ���� ����
uchar code = ua_next_byte();

switch(code)
  {
  //Mnemonic Operand Instruction Code
  //                        B1            B2             B3        B4
  case 0x00:
    {
    //ROR A, 1                0000 0000
    cmd.itype = NEC_78K_0S_ror;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg = rA;
    cmd.Op2.type = o_imm;
    cmd.Op2.value = 1;
    cmd.Op2.regmode = 1;
    }  break;

  case 0x02:
    {
    //RORC A, 1               0000 0010
    cmd.itype = NEC_78K_0S_rorc;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg = rA;
    cmd.Op2.type = o_imm;
    cmd.Op2.value = 1;
    cmd.Op2.regmode = 1;
    }  break;

  case 0x04:
    {
    //CLR1 CY                 0000 0100
    cmd.itype = NEC_78K_0S_clr1;
    cmd.Op1.type = o_phrase;
    cmd.Op1.reg  = bCY;
    } break;

  case 0x05:
    {
    //XCH A, saddr            0000 0101      Saddr-offset
    cmd.itype = NEC_78K_0S_xch;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_mem;
    cmd.Op2.dtyp = dt_byte;
    cmd.Op2.addr=saddr(ua_next_byte());
    } break;

  case 0x06:
    {
    //NOT1 CY                 0000 0110
    cmd.itype = NEC_78K_0S_not1;
    cmd.Op1.type = o_phrase;
    cmd.Op1.reg = bCY;
    } break;

  case 0x07:
    {
    //XCH A, sfr              0000 0111      Sfr-offset
    cmd.itype = NEC_78K_0S_xch;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_mem;
    cmd.Op2.dtyp = dt_byte;
    cmd.Op2.addr = sfr(ua_next_byte());
    } break;

  case 0x08:
    {
    //NOP                     0000 1000     
    cmd.itype = NEC_78K_0S_nop;
    } break;

  case 0x0A:  // All 0x0A commands
    {
    uchar code2 = ua_next_byte();
    switch(code2)
      {
      case 0x88:
      case 0x98:
      case 0xA8:
      case 0xB8:
      case 0xC8:    
      case 0xD8:    
      case 0xE8:    
      case 0xF8:
        {    
        //BT saddr.bit, $addr16   0000 1010      1 B2 B1 B0 1000  Saddr-offset   jdisp
        cmd.itype = NEC_78K_0S_bt;
        cmd.Op1.type  = o_bit;
        cmd.Op1.dtyp  = dt_byte;
        cmd.Op1.addr  = saddr(ua_next_byte());
        cmd.Op1.value = (code2>>4) & 0x07;
        jdisp(cmd.Op2, ua_next_byte(), 4);
        } break;

      case 0x84:
      case 0x94:
      case 0xA4:
      case 0xB4:
      case 0xC4:    
      case 0xD4:    
      case 0xE4:    
      case 0xF4:
        {    
        //BT sfr.bit,   $addr16   0000 1010      1 B2 B1 B0 0100  Sfr-offset  jdisp
        //BT PSW.bit,   $addr16   0000 1010      1 B2 B1 B0 1000  0001 1110   jdisp
        cmd.itype = NEC_78K_0S_bt;
        uchar code3 = ua_next_byte();
        if(code3==0x1E)
          {
          cmd.Op1.type  = o_bit;
          cmd.Op1.reg   = rPSW;
          cmd.Op1.value = (code2>>4)&0x07;
          jdisp(cmd.Op2, code3, 4);
          }
        else
          {
          cmd.Op1.type  = o_bit;
          cmd.Op1.dtyp  = dt_byte;
          cmd.Op1.addr  = sfr(code3);
          cmd.Op1.value = (code2>>4) & 0x07;
          jdisp(cmd.Op2, ua_next_byte(), 4);
          }
        } break;

      case 0x08:
      case 0x18:
      case 0x28:
      case 0x38:
      case 0x48:    
      case 0x58:    
      case 0x68:    
      case 0x78:
        {    
        //BF saddr.bit, $addr16   0000 1010      0 B2 B1 B0 1000  Saddr-offset   jdisp
        //BF PSW.bit,   $addr16   0000 1010      0 B2 B1 B0 1000  0001 1110       jdisp
        cmd.itype = NEC_78K_0S_bf;
        uchar code3=ua_next_byte();
        if(code3==0x1E)
          {
          cmd.Op1.type  = o_bit;
          cmd.Op1.reg   = rPSW;
          cmd.Op1.value = (code2>>4)&0x07;
          jdisp(cmd.Op2, ua_next_byte(), 4);
          }
        else
          {
          cmd.Op1.type  = o_bit;
          cmd.Op1.dtyp  = dt_byte;
          cmd.Op1.addr  = saddr(code3);
          cmd.Op1.value = (code2>>4) & 0x07;
          jdisp(cmd.Op2, ua_next_byte(), 4);
          }
        } break;

      case 0x04:
      case 0x14:
      case 0x24:
      case 0x34:
      case 0x44:    
      case 0x54:    
      case 0x64:    
      case 0x74:
        {    
        //BF sfr.bit,   $addr16   0000 1010      0 B2 B1 B0 0100  Sfr-offset jdisp
        cmd.itype = NEC_78K_0S_bf;
        cmd.Op1.type  = o_bit;
        cmd.Op1.dtyp  = dt_byte;
        cmd.Op1.addr  = sfr(ua_next_byte());
        cmd.Op1.value = (code2>>4) & 0x07;
        jdisp(cmd.Op2, ua_next_byte(), 4);
        } break;

      case 0x00:
      case 0x10:
      case 0x20:
      case 0x30:
      case 0x40:    
      case 0x50:    
      case 0x60:    
      case 0x70:
        {    
        //BF A.bit,     $addr16   0000 1010      0 B2 B1 B0 0000  jdisp
        cmd.itype = NEC_78K_0S_bf;
        cmd.Op1.type  = o_bit;
        cmd.Op1.reg   = rA;
        cmd.Op1.value = (code2>>4) & 0x07;
        jdisp(cmd.Op2, ua_next_byte(), 3);
        } break;

      case 0x80:
      case 0x90:
      case 0xA0:
      case 0xB0:
      case 0xC0:    
      case 0xD0:    
      case 0xE0:    
      case 0xF0:
        {    
        //BT A.bit,     $addr16   0000 1010      1 B2 B1 B0 0000  jdisp
        cmd.itype = NEC_78K_0S_bt;
        cmd.Op1.type  = o_bit;
        cmd.Op1.reg   = rA;
        cmd.Op1.value = (code2>>4) & 0x07;
        jdisp(cmd.Op2, ua_next_byte(), 3);
        } break;

      case 0x0A:
      case 0x1A:
      case 0x2A:
      case 0x3A:
      case 0x4A:    
      case 0x5A:    
      case 0x6A:    
      case 0x7A:
        {    
        //SET1 saddr.bit          0000 1010      0 B2 B1 B0 1010  Saddr-offset
        //SET1 PSW.bit            0000 1010      0 B2 B1 B0 1010  0001 1110
        //EI                      0000 1010      0 1  1  1  1010  0001 1110
        uchar code3=ua_next_byte();
        if(code3==0x1E)
          {
          if(code2==0x7A)  //EI
            {
            cmd.itype = NEC_78K_0S_EI;
            }
          else             //SET1 PSW.bit
            {
            cmd.itype = NEC_78K_0S_set1;
            cmd.Op1.type  = o_bit;
            cmd.Op1.reg   = rPSW;
            cmd.Op1.value = (code2>>4)&0x07;
            }
          }
        else //SET1 saddr.bit
          {
          cmd.itype = NEC_78K_0S_set1;
          cmd.Op1.type  = o_bit;
          cmd.Op1.dtyp  = dt_byte;
          cmd.Op1.addr  = saddr(code3);
          cmd.Op1.value = (code2>>4)&0x07;
          }
        } break;

      case 0x06:
      case 0x16:
      case 0x26:
      case 0x36:
      case 0x46:    
      case 0x56:    
      case 0x66:    
      case 0x76:
        {    
        //SET1 sfr.bit            0000 1010      0 B2 B1 B0 0110  Sfr-offset
        cmd.itype = NEC_78K_0S_set1;
        cmd.Op1.type  = o_bit;
        cmd.Op1.dtyp  = dt_byte;
        cmd.Op1.addr  = sfr(ua_next_byte());
        cmd.Op1.value = (code2>>4)&0x07;
        } break;

      case 0x02:
      case 0x12:
      case 0x22:
      case 0x32:
      case 0x42:    
      case 0x52:    
      case 0x62:    
      case 0x72:
        {    
        //SET1 A.bit              0000 1010      0 B2 B1 B0 0010
        cmd.itype = NEC_78K_0S_set1;
        cmd.Op1.type  = o_bit;
        cmd.Op1.reg   = rA;
        cmd.Op1.value = (code2>>4)&0x07;
        } break;

      case 0x0E:
      case 0x1E:
      case 0x2E:
      case 0x3E:
      case 0x4E:    
      case 0x5E:    
      case 0x6E:    
      case 0x7E:
        {    
        //SET1 [HL].bit           0000 1010      0 B2 B1 B0 1110
        cmd.itype = NEC_78K_0S_set1;
        cmd.Op1.type  = o_bit;
        cmd.Op1.reg   = rHL;
        cmd.Op1.prepost = 1;
        cmd.Op1.value = (code2>>4)&0x07;
        } break;

      case 0x8A:
      case 0x9A:
      case 0xAA:
      case 0xBA:
      case 0xCA:    
      case 0xDA:    
      case 0xEA:    
      case 0xFA:
        {    
        //CLR1 saddr.bit          0000 1010      1 B2 B1 B0 1010  Saddr-offset
        //CLR1 PSW.bit            0000 1010      1 B2 B1 B0 1010  0001 1110
        //DI                      0000 1010      1 1  1  1  1010  0001 1110
        uchar code3=ua_next_byte();
        if(code3==0x1E)
          {
          if(code2==0xFA)  //DI
            {
            cmd.itype = NEC_78K_0S_DI;
            }
          else             //CLR1 PSW.bit
            {
            cmd.itype = NEC_78K_0S_clr1;
            cmd.Op1.type  = o_bit;
            cmd.Op1.reg   = rPSW;
            cmd.Op1.value = (code2>>4)&0x07;
            }
          }
        else //CLR1 saddr.bit
          {
          cmd.itype = NEC_78K_0S_clr1;
          cmd.Op1.type  = o_bit;
          cmd.Op1.dtyp  = dt_byte;
          cmd.Op1.addr  = saddr(code3);
          cmd.Op1.value = (code2>>4)&0x07;
          }
        } break;

      case 0x86:
      case 0x96:
      case 0xA6:
      case 0xB6:
      case 0xC6:    
      case 0xD6:    
      case 0xE6:    
      case 0xF6:
        {    
        //CLR1 sfr.bit            0000 1010      1 B2 B1 B0 0110  Sfr-offset
        cmd.itype = NEC_78K_0S_clr1;
        cmd.Op1.type  = o_bit;
        cmd.Op1.dtyp  = dt_byte;
        cmd.Op1.addr  = sfr(ua_next_byte());
        cmd.Op1.value = (code2>>4)&0x07;
        } break;

      case 0x82:
      case 0x92:
      case 0xA2:
      case 0xB2:
      case 0xC2:    
      case 0xD2:    
      case 0xE2:    
      case 0xF2:
        {    
        //CLR1 A.bit              0000 1010      1 B2 B1 B0 0010
        cmd.itype = NEC_78K_0S_clr1;
        cmd.Op1.type  = o_bit;
        cmd.Op1.reg   = rA;
        cmd.Op1.value = (code2>>4)&0x07;
        } break;

      case 0x8E:
      case 0x9E:
      case 0xAE:
      case 0xBE:
      case 0xCE:    
      case 0xDE:    
      case 0xEE:    
      case 0xFE:
        {    
        //CLR1 [HL].bit           0000 1010      1 B2 B1 B0 1110
        cmd.itype = NEC_78K_0S_clr1;
        cmd.Op1.type  = o_bit;
        cmd.Op1.reg   = rHL;
        cmd.Op1.prepost = 1;
        cmd.Op1.value = (code2>>4)&0x07;
        } break;

      case 0xF1:
      case 0xF3:
      case 0xF5:
      case 0xF7:
      case 0xF9:    
      case 0xFB:    
      case 0xFD:    
      case 0xFF:
        {    
        //MOV r, #byte            0000 1010      1111 R2R1R0 1  Data
        cmd.itype = NEC_78K_0S_mov;
        cmd.Op1.type  = o_reg;
        cmd.Op1.reg   = ((code2>>1)&0x07)+rX;
        cmd.Op2.type  = o_imm;
        cmd.Op2.dtyp  = dt_byte;
        cmd.Op2.value = ua_next_byte();
        } break;

      case 0x21:
      case 0x25:
      case 0x27:
      case 0x29:    
      case 0x2B:    
      case 0x2D:    
      case 0x2F:
        {    
        //MOV A, r                0000 1010      0010 R2R1R0 1        Except r = A.
        cmd.itype = NEC_78K_0S_mov;
        cmd.Op1.type  = o_reg;
        cmd.Op1.reg   = rA;
        cmd.Op2.type  = o_reg;
        cmd.Op2.reg = ((code2>>1)&0x07)+rX;
        } break;

      case 0xE1:
      case 0xE5:
      case 0xE7:
      case 0xE9:    
      case 0xEB:    
      case 0xED:    
      case 0xEF:
        {    
        //MOV r, A                0000 1010      1110 R2R1R0 1        Except r = A.
        cmd.itype = NEC_78K_0S_mov;
        cmd.Op1.type  = o_reg;
        cmd.Op1.reg = ((code2>>1)&0x07)+rX;
        cmd.Op2.type  = o_reg;
        cmd.Op2.reg   = rA;
        } break;

      case 0x05:
      case 0x07:
      case 0x09:    
      case 0x0B:    
      case 0x0D:    
      case 0x0F:
        {    
        //XCH A, r                0000 1010      0000 R2R1R0 1       Except r = A, X.
        cmd.itype = NEC_78K_0S_xch;
        cmd.Op1.type  = o_reg;
        cmd.Op1.reg   = rA;
        cmd.Op2.type  = o_reg;
        cmd.Op2.reg = ((code2>>1)&0x07)+rX;
        } break;

      case 0x81:
      case 0x83:
      case 0x85:
      case 0x87:
      case 0x89:    
      case 0x8B:    
      case 0x8D:    
      case 0x8F:
        {    
        //ADD A, r                0000 1010      1000 R2R1R0 1
        cmd.itype = NEC_78K_0S_add;
        cmd.Op1.type  = o_reg;
        cmd.Op1.reg   = rA;
        cmd.Op2.type  = o_reg;
        cmd.Op2.reg = ((code2>>1)&0x07)+rX;
        } break;

      case 0xA1:
      case 0xA3:
      case 0xA5:
      case 0xA7:
      case 0xA9:    
      case 0xAB:    
      case 0xAD:    
      case 0xAF:
        {    
        //ADDC A, r               0000 1010      1010 R2R1R0 1
        cmd.itype = NEC_78K_0S_addc;
        cmd.Op1.type  = o_reg;
        cmd.Op1.reg   = rA;
        cmd.Op2.type  = o_reg;
        cmd.Op2.reg = ((code2>>1)&0x07)+rX;
        } break;

      case 0x91:
      case 0x93:
      case 0x95:
      case 0x97:
      case 0x99:    
      case 0x9B:    
      case 0x9D:    
      case 0x9F:
        {    
        //SUB A, r                0000 1010      1001 R2R1R0 1
        cmd.itype = NEC_78K_0S_sub;
        cmd.Op1.type  = o_reg;
        cmd.Op1.reg   = rA;
        cmd.Op2.type  = o_reg;
        cmd.Op2.reg = ((code2>>1)&0x07)+rX;
        } break;

      case 0xB1:
      case 0xB3:
      case 0xB5:
      case 0xB7:
      case 0xB9:    
      case 0xBB:    
      case 0xBD:    
      case 0xBF:
        {    
        //SUBC A, r               0000 1010      1011 R2R1R0 1
        cmd.itype = NEC_78K_0S_subc;
        cmd.Op1.type  = o_reg;
        cmd.Op1.reg   = rA;
        cmd.Op2.type  = o_reg;
        cmd.Op2.reg = ((code2>>1)&0x07)+rX;
        } break;

      case 0x61:
      case 0x63:
      case 0x65:
      case 0x67:
      case 0x69:    
      case 0x6B:    
      case 0x6D:    
      case 0x6F:
        {    
        //AND A, r                0000 1010      0110 R2R1R0 1
        cmd.itype = NEC_78K_0S_and;
        cmd.Op1.type  = o_reg;
        cmd.Op1.reg   = rA;
        cmd.Op2.type  = o_reg;
        cmd.Op2.reg = ((code2>>1)&0x07)+rX;
        } break;

      case 0x71:
      case 0x73:
      case 0x75:
      case 0x77:
      case 0x79:    
      case 0x7B:    
      case 0x7D:    
      case 0x7F:
        {    
        //OR A, r                 0000 1010      0111 R2R1R0 1
        cmd.itype = NEC_78K_0S_or;
        cmd.Op1.type  = o_reg;
        cmd.Op1.reg   = rA;
        cmd.Op2.type  = o_reg;
        cmd.Op2.reg = ((code2>>1)&0x07)+rX;
        } break;

      case 0x41:
      case 0x43:
      case 0x45:
      case 0x47:
      case 0x49:    
      case 0x4B:    
      case 0x4D:    
      case 0x4F:
        {    
        //XOR A, r                0000 1010      0100 R2R1R0 1
        cmd.itype = NEC_78K_0S_xor;
        cmd.Op1.type  = o_reg;
        cmd.Op1.reg   = rA;
        cmd.Op2.type  = o_reg;
        cmd.Op2.reg = ((code2>>1)&0x07)+rX;
        } break;

      case 0x11:
      case 0x13:
      case 0x15:
      case 0x17:
      case 0x19:    
      case 0x1B:    
      case 0x1D:    
      case 0x1F:
        {    
        //CMP A, r                0000 1010      0001 R2R1R0 1
        cmd.itype = NEC_78K_0S_cmp;
        cmd.Op1.type  = o_reg;
        cmd.Op1.reg   = rA;
        cmd.Op2.type  = o_reg;
        cmd.Op2.reg = ((code2>>1)&0x07)+rX;
        } break;

      case 0xC1:
      case 0xC3:
      case 0xC5:
      case 0xC7:
      case 0xC9:    
      case 0xCB:    
      case 0xCD:    
      case 0xCF:
        {    
        //INC r                   0000 1010      1100 R2R1R0 1
        cmd.itype = NEC_78K_0S_inc;
        cmd.Op1.type  = o_reg;
        cmd.Op1.reg   = ((code2>>1)&0x07)+rX;
        } break;

      case 0xD1:
      case 0xD3:
      case 0xD5:
      case 0xD7:
      case 0xD9:    
      case 0xDB:    
      case 0xDD:    
      case 0xDF:
        {    
        //DEC r                   0000 1010      1101 R2R1R0 1
        cmd.itype = NEC_78K_0S_dec;
        cmd.Op1.type  = o_reg;
        cmd.Op1.reg   = ((code2>>1)&0x07)+rX;
        } break;

      default:
        {
        // ��室 � �訡��� ERROR
        return 0;
        }
      } // END switch  0x0A commands
    } break; // END case 0x0A

  case 0x0B:
    {
    //XCH A, [DE]             0000 1011      
    cmd.itype = NEC_78K_0S_xch;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_reg;
    cmd.Op2.reg  = rDE;
    cmd.Op2.prepost =1;
    } break;

  case 0x0C:
    {
    //HALT                    0000 1100
    cmd.itype = NEC_78K_0S_HALT;
    } break;

  case 0x0D:
    {
    //XCH A, [HL+byte]        0000 1101      Data
    cmd.itype = NEC_78K_0S_xch;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_reg;
    cmd.Op2.reg  = rHL;
    cmd.Op2.addr = ua_next_byte();
    cmd.Op2.prepost =1;
    cmd.Op2.xmode   =1;
    } break;

  case 0x0E:
    {
    //STOP                    0000 1110
    cmd.itype = NEC_78K_0S_STOP;
    } break;

  case 0x0F:
    {
    //XCH A, [HL]             0000 1111
    cmd.itype = NEC_78K_0S_xch;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_reg;
    cmd.Op2.reg  = rHL;
    cmd.Op2.prepost = 1;
    } break;

  case 0x10:
    {
    //ROL A, 1                0001 0000
    cmd.itype = NEC_78K_0S_rol;
    cmd.Op1.type  = o_reg;
    cmd.Op1.reg   = rA;
    cmd.Op2.type  = o_imm;
    cmd.Op2.value = 1;
    cmd.Op2.regmode = 1;
    } break;

  case 0x11:
    {
    //CMP saddr, #byte        0001 0001      Saddr-offset   Data
    cmd.itype = NEC_78K_0S_cmp;
    cmd.Op1.type  = o_mem;
    cmd.Op1.dtyp  = dt_byte;
    cmd.Op1.addr  = saddr(ua_next_byte());
    cmd.Op2.type  = o_imm;
    cmd.Op2.value = ua_next_byte();
    } break;

  case 0x12:
    {
    //ROLC A, 1               0001 0010
    cmd.itype = NEC_78K_0S_rolc;
    cmd.Op1.type  = o_reg;
    cmd.Op1.reg   = rA;
    cmd.Op2.type  = o_imm;
    cmd.Op2.value = 1;
    cmd.Op2.regmode = 1;
    } break;

  case 0x13:
    {
    //CMP A, #byte            0001 0011      Data
    cmd.itype = NEC_78K_0S_cmp;
    cmd.Op1.type  = o_reg;
    cmd.Op1.reg   = rA;
    cmd.Op2.type  = o_imm;
    cmd.Op2.value = ua_next_byte();
    } break;

  case 0x14:
    {
    //SET1 CY                 0001 0100
    cmd.itype = NEC_78K_0S_set1;
    cmd.Op1.type = o_phrase;
    cmd.Op1.reg = bCY;
    } break;

  case 0x15:
    {
    //CMP A, saddr            0001 0101      Saddr-offset
    cmd.itype = NEC_78K_0S_cmp;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_mem;
    cmd.Op2.dtyp = dt_byte;
    cmd.Op2.addr = saddr(ua_next_byte());
    } break;

  case 0x19:
    {
    //CMP A, !addr16          0001 1001      Low addr       High addr
    cmd.itype = NEC_78K_0S_cmp;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_mem;
    cmd.Op2.dtyp = dt_byte;
    cmd.Op2.addr16 = 1;
    addr16(cmd.Op2);
    } break;

  case 0x1D:
    {
    //CMP A, [HL+byte]        0001 1101      Data
    cmd.itype = NEC_78K_0S_cmp;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_reg;
    cmd.Op2.reg  = rHL;
    cmd.Op2.addr = ua_next_byte();
    cmd.Op2.prepost = 1;
    cmd.Op2.xmode   = 1;
    } break;

  case 0x1F:
    {
    //CMP A, [HL]             0001 1111
    cmd.itype = NEC_78K_0S_cmp;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_reg;
    cmd.Op2.reg  = rHL;
    cmd.Op2.prepost = 1;
    } break;

  case 0x20:
    {
    //RET                     0010 0000
    cmd.itype = NEC_78K_0S_ret;
    } break;

  case 0x22:
    {
    //CALL !addr16            0010 0010      Low addr         High addr
    cmd.itype = NEC_78K_0S_call;
    //cmd.Op1.offb = cmd.size;
    cmd.Op1.type = o_near;
    cmd.Op1.addr16 = 1;
    addr16(cmd.Op1);
    } break;

  case 0x24:
    {
    //RETI                    0010 0100
    cmd.itype = NEC_78K_0S_reti;
    } break;

  case 0x25:
    {
    //MOV A, PSW              0010 0101      00011110
    //MOV A, saddr            0010 0101      Saddr-offset
    cmd.itype = NEC_78K_0S_mov;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg = rA;
    uchar tst = ua_next_byte();
    if(tst == 0x1E)
      {
      cmd.Op2.type = o_reg;
      cmd.Op2.reg  = rPSW;
      }
    else
      {
      cmd.Op2.type = o_mem;
      cmd.Op2.dtyp = dt_byte;
      cmd.Op2.addr = saddr(tst);
      }
    } break;

  case 0x27:
    {
    //MOV A, sfr              0010 0111      Sfr-offset
    cmd.itype = NEC_78K_0S_mov;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_mem;
    cmd.Op2.dtyp = dt_byte;
    cmd.Op2.addr = sfr(ua_next_byte());
    } break;

  case 0x29:
    {
    //MOV A, !addr16          0010 1001      Low addr       High addr
    cmd.itype = NEC_78K_0S_mov;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_mem;
    cmd.Op2.dtyp = dt_byte;
    cmd.Op2.addr16 = 1;
    addr16(cmd.Op2);
    } break;

  case 0x2B:
    {
    //MOV A, [DE]             0010 1011
    cmd.itype = NEC_78K_0S_mov;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_reg;
    cmd.Op2.reg  = rDE;
    cmd.Op2.prepost = 1;
    } break;

  case 0x2C:
    {
    //POP PSW                 0010 1100
    cmd.itype = NEC_78K_0S_pop;
    cmd.Op1.type = o_phrase;
    cmd.Op1.reg  = rPSW;
    } break;

  case 0x2D:
    {
    //MOV A, [HL+byte]        0010 1101      Data
    cmd.itype = NEC_78K_0S_mov;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_reg;
    cmd.Op2.reg  = rHL;
    cmd.Op2.addr = ua_next_byte();
    cmd.Op2.prepost = 1;
    cmd.Op2.xmode   = 1;
    } break;

  case 0x2E:
    {
    //PUSH PSW                0010 1110
    cmd.itype = NEC_78K_0S_push;
    cmd.Op1.type = o_phrase;
    cmd.Op1.reg  = rPSW;
    } break;

  case 0x2F:
    {
    //MOV A, [HL]             0010 1111
    cmd.itype = NEC_78K_0S_mov;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_reg;
    cmd.Op2.reg  = rHL;
    cmd.Op2.prepost = 1;
    } break;

  case 0x30:
    {
    //BR $addr16              0011 0000      jdisp
    cmd.itype = NEC_78K_0S_br;
    jdisp(cmd.Op1, ua_next_byte(), 2);
    } break;

  case 0x32:
    {
    //DBNZ saddr, $addr16     0011 0010      Saddr-offset     jdisp
    cmd.itype = NEC_78K_0S_dbnz;
    cmd.Op1.type = o_mem;
    cmd.Op1.dtyp = dt_byte;
    cmd.Op1.addr = saddr(ua_next_byte());
    jdisp(cmd.Op2, ua_next_byte(), 3);
    } break;

  case 0x34:
    {
    //DBNZ C, $addr16         0011 0100      jdisp
    cmd.itype = NEC_78K_0S_dbnz;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rC;
    jdisp(cmd.Op2, ua_next_byte(), 2);
    } break;

  case 0x36:
    {
    //DBNZ B, $addr16         0011 0110      jdisp
    cmd.itype = NEC_78K_0S_dbnz;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rB;
    jdisp(cmd.Op2, ua_next_byte(), 2);
    } break;

  case 0x38:
    {
    //BC $addr16              0011 1000      jdisp
    cmd.itype = NEC_78K_0S_bc;
    jdisp( cmd.Op1, ua_next_byte(), 2);
    } break;

  case 0x3A:
    {
    //BNC $addr16             0011 1010      jdisp
    cmd.itype = NEC_78K_0S_bnc;
    jdisp( cmd.Op1, ua_next_byte(), 2);
    } break;

  case 0x3C:
    {
    //BZ $addr16              0011 1100      jdisp
    cmd.itype = NEC_78K_0S_bz;
    jdisp( cmd.Op1, ua_next_byte(), 2);
    } break;

  case 0x3E:
    {
    //BNZ $addr16             0011 1110      jdisp
    cmd.itype = NEC_78K_0S_bnz;
    jdisp( cmd.Op1, ua_next_byte(), 2);
    } break;

  case 0x41:
    {
    //XOR saddr, #byte        0100 0001      Saddr-offset   Data
    cmd.itype = NEC_78K_0S_xor;
    cmd.Op1.type  = o_mem;
    cmd.Op1.dtyp  = dt_byte;
    cmd.Op1.addr  = saddr(ua_next_byte());
    cmd.Op2.type  = o_imm;
    cmd.Op2.value = ua_next_byte();
    } break;

  case 0x43:
    {
    //XOR A, #byte            0100 0011      Data
    cmd.itype = NEC_78K_0S_xor;
    cmd.Op1.type  = o_reg;
    cmd.Op1.reg   = rA;
    cmd.Op2.type  = o_imm;
    cmd.Op2.value = ua_next_byte();
    } break;

  case 0x45:
    {
    //XOR A, saddr            0100 0101      Saddr-offset
    cmd.itype = NEC_78K_0S_xor;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_mem;
    cmd.Op2.dtyp = dt_byte;
    cmd.Op2.addr = saddr(ua_next_byte());
    } break;

  case 0x49:
    {
    //XOR A, !addr16          0100 1001      Low addr       High addr
    cmd.itype = NEC_78K_0S_xor;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_mem;
    cmd.Op2.dtyp = dt_byte;
    cmd.Op2.addr16 = 1;
    addr16(cmd.Op2);
    } break;

  case 0x4D:
    {
    //XOR A, [HL+byte]        0100 1101      Data
    cmd.itype = NEC_78K_0S_xor;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_reg;
    cmd.Op2.reg  = rHL;
    cmd.Op2.addr = ua_next_byte();
    cmd.Op2.prepost = 1;
    cmd.Op2.xmode   = 1;
    } break;

  case 0x4F:
    {
    //XOR A, [HL]             0100 1111
    cmd.itype = NEC_78K_0S_xor;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_reg;
    cmd.Op2.reg  = rHL;
    cmd.Op2.prepost = 1;
    } break;

  case 0x61:
    {
    //AND saddr, #byte        0110 0001      Saddr-offset   Data
    cmd.itype = NEC_78K_0S_and;
    cmd.Op1.type  = o_mem;
    cmd.Op1.dtyp  = dt_byte;
    cmd.Op1.addr  = saddr(ua_next_byte());
    cmd.Op2.type  = o_imm;
    cmd.Op2.value = ua_next_byte();
    } break;

  case 0x63:
    {
    //AND A, #byte            0110 0011      Data
    cmd.itype = NEC_78K_0S_and;
    cmd.Op1.type  = o_reg;
    cmd.Op1.reg   = rA;
    cmd.Op2.type  = o_imm;
    cmd.Op2.value = ua_next_byte();
    } break;

  case 0x65:
    {
    //AND A, saddr            0110 0101      Saddr-offset
    cmd.itype = NEC_78K_0S_and;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_mem;
    cmd.Op2.dtyp = dt_byte;
    cmd.Op2.addr = saddr(ua_next_byte());
    } break;

  case 0x69:
    {
    //AND A, !addr16          0110 1001      Low addr       High addr
    cmd.itype = NEC_78K_0S_and;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_mem;
    cmd.Op2.dtyp = dt_byte;
    cmd.Op2.addr16 = 1;
    addr16(cmd.Op2);
    } break;

  case 0x6D:
    {
    //AND A, [HL+byte]        0110 1101      Data
    cmd.itype = NEC_78K_0S_and;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_reg;
    cmd.Op2.reg  = rHL;
    cmd.Op2.addr = ua_next_byte();
    cmd.Op2.prepost = 1;
    cmd.Op2.xmode   = 1;
    } break;

  case 0x6F:
    {
    //AND A, [HL]             0110 1111
    cmd.itype = NEC_78K_0S_and;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_reg;
    cmd.Op2.reg  = rHL;
    cmd.Op2.prepost = 1;
    } break;

  case 0x71:
    {
    //OR saddr, #byte         0111 0001      Saddr-offset   Data
    cmd.itype = NEC_78K_0S_or;
    cmd.Op1.type  = o_mem;
    cmd.Op1.dtyp  = o_mem;
    cmd.Op1.addr  = saddr(ua_next_byte());
    cmd.Op2.type  = o_imm;
    cmd.Op2.value = ua_next_byte();
    } break;

  case 0x73:
    {
    //OR A, #byte             0111 0011      Data
    cmd.itype = NEC_78K_0S_or;
    cmd.Op1.type  = o_reg;
    cmd.Op1.reg   = rA;
    cmd.Op2.type  = o_imm;
    cmd.Op2.value = ua_next_byte();
    } break;

  case 0x75:
    {
    //OR A, saddr             0111 0101      Saddr-offset
    cmd.itype = NEC_78K_0S_or;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_mem;
    cmd.Op2.dtyp = dt_byte;
    cmd.Op2.addr = saddr(ua_next_byte());
    } break;

  case 0x79:
    {
    //OR A, !addr16           0111 1001      Low addr       High addr
    cmd.itype = NEC_78K_0S_or;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_mem;
    cmd.Op2.dtyp = dt_byte;
    addr16(cmd.Op2);
    } break;

  case 0x7D:
    {
    //OR A, [HL+byte]         0111 1101      Data
    cmd.itype = NEC_78K_0S_or;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_reg;
    cmd.Op2.reg  = rHL;
    cmd.Op2.addr = ua_next_byte();
    cmd.Op2.prepost = 1;
    cmd.Op2.xmode   = 1;
    } break;

  case 0x7F:
    {
    //OR A, [HL]              0111 1111
    cmd.itype = NEC_78K_0S_or;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_reg;
    cmd.Op2.reg  = rHL;
    cmd.Op2.prepost = 1;
    } break;

  case 0x81:
    {
    //ADD saddr, #byte        1000 0001      Saddr-offset    Data
    cmd.itype = NEC_78K_0S_add;
    cmd.Op1.type  = o_mem;
    cmd.Op1.dtyp  = dt_byte;
    cmd.Op1.addr  = saddr(ua_next_byte());
    cmd.Op2.type  = o_imm;
    cmd.Op2.value = ua_next_byte();
    } break;

  case 0x83:
    {
    //ADD A, #byte            1000 0011      Data
    cmd.itype = NEC_78K_0S_add;
    cmd.Op1.type  = o_reg;
    cmd.Op1.reg   = rA;
    cmd.Op2.type  = o_imm;
    cmd.Op2.value = ua_next_byte();
    } break;

  case 0x85:
    {
    //ADD A, saddr            1000 0101      Saddr-offset
    cmd.itype = NEC_78K_0S_add;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_mem;
    cmd.Op2.dtyp = dt_byte;
    cmd.Op2.addr = saddr(ua_next_byte());
    } break;

  case 0x89:
    {
    //ADD A, !addr16          1000 1001      Low addr        High addr
    cmd.itype = NEC_78K_0S_add;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_mem;
    cmd.Op2.dtyp = dt_byte;
    cmd.Op2.addr16 = 1;
    addr16(cmd.Op2);
    } break;

  case 0x8D:
    {
    //ADD A, [HL+byte]        1000 1101      Data
    cmd.itype = NEC_78K_0S_add;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_reg;
    cmd.Op2.reg  = rHL;
    cmd.Op2.addr = ua_next_byte();
    cmd.Op2.prepost = 1;
    cmd.Op2.xmode   = 1;
    } break;

  case 0x8F:
    {
    //ADD A, [HL]             1000 1111
    cmd.itype = NEC_78K_0S_add;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_reg;
    cmd.Op2.reg  = rHL;
    cmd.Op2.prepost = 1;
    } break;

  case 0x80:
  case 0x84:
  case 0x88:
  case 0x8C:
    {
    //INCW rp                 1000 P1 P0 00
    cmd.itype = NEC_78K_0S_incw;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg = ((code>>2)&0x03)+rAX;
    } break;

  case 0x91:
    {
    //SUB saddr, #byte        1001 0001      Saddr-offset    Data
    cmd.itype = NEC_78K_0S_sub;
    cmd.Op1.type  = o_mem;
    cmd.Op1.dtyp  = dt_byte;
    cmd.Op1.addr  = saddr(ua_next_byte());
    cmd.Op2.type  = o_imm;
    cmd.Op2.value = ua_next_byte();
    } break;

  case 0x93:
    {
    //SUB A, #byte            1001 0011      Data
    cmd.itype = NEC_78K_0S_sub;
    cmd.Op1.type  = o_reg;
    cmd.Op1.reg   = rA;
    cmd.Op2.type  = o_imm;
    cmd.Op2.value = ua_next_byte();
    } break;

  case 0x95:
    {
    //SUB A, saddr            1001 0101      Saddr-offset
    cmd.itype = NEC_78K_0S_sub;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_mem;
    cmd.Op2.dtyp = dt_byte;
    cmd.Op2.addr = saddr(ua_next_byte());
    } break;

  case 0x99:
    {
    //SUB A, !addr16          1001 1001      Low addr        High addr
    cmd.itype = NEC_78K_0S_sub;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_mem;
    cmd.Op2.dtyp = dt_byte;
    cmd.Op2.addr16 = 1;
    addr16(cmd.Op2);
    } break;

  case 0x9D:
    {
    //SUB A, [HL+byte]        1001 1101      Data
    cmd.itype = NEC_78K_0S_sub;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_reg;
    cmd.Op2.reg  = rHL;
    cmd.Op2.addr = ua_next_byte();
    cmd.Op2.prepost = 1;
    cmd.Op2.xmode   = 1;
    } break;

  case 0x9F:
    {
    //SUB A, [HL]             1001 1111
    cmd.itype = NEC_78K_0S_sub;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_reg;
    cmd.Op2.reg  = rHL;
    cmd.Op2.prepost = 1;
    } break;

  case 0x90:
  case 0x94:
  case 0x98:
  case 0x9C:
    {
    //DECW rp                 1001 P1 P0 00
    cmd.itype = NEC_78K_0S_decw;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg = ((code>>2)&0x03)+rAX;
    } break;

  case 0xA1:
    {
    //ADDC saddr,#byte        1010 0001      Saddr-offset    Data
    cmd.itype = NEC_78K_0S_addc;
    cmd.Op1.type  = o_mem;
    cmd.Op1.dtyp  = dt_byte;
    cmd.Op1.addr  = saddr(ua_next_byte());
    cmd.Op2.type  = o_imm;
    cmd.Op2.value = ua_next_byte();
    } break;

  case 0xA3:
    {
    //ADDC A, #byte           1010 0011      Data
    cmd.itype = NEC_78K_0S_addc;
    cmd.Op1.type  = o_reg;
    cmd.Op1.reg   = rA;
    cmd.Op2.type  = o_imm;
    cmd.Op2.value = ua_next_byte();
    } break;

  case 0xA5:
    {
    //ADDC A, saddr           1010 0101      Saddr-offset
    cmd.itype = NEC_78K_0S_addc;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_mem;
    cmd.Op2.dtyp = dt_byte;
    cmd.Op2.addr = saddr(ua_next_byte());
    } break;

  case 0xA9:
    {
    //ADDC A, !addr16         1010 1001      Low addr        High addr
    cmd.itype = NEC_78K_0S_addc;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_mem;
    cmd.Op2.dtyp = dt_byte;
    cmd.Op2.addr16 = 1;
    addr16(cmd.Op2);
    } break;

  case 0xAD:
    {
    //ADDC A,[HL+byte]        1010 1101      Data
    cmd.itype = NEC_78K_0S_addc;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_reg;
    cmd.Op2.reg  = rHL;
    cmd.Op2.addr = ua_next_byte();
    cmd.Op2.prepost = 1;
    cmd.Op2.xmode   = 1;
    } break;

  case 0xAF:
    {
    //ADDC A,[HL]             1010 1111
    cmd.itype = NEC_78K_0S_addc;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_reg;
    cmd.Op2.reg  = rHL;
    cmd.Op2.prepost = 1;
    } break;

  case 0xA0:
  case 0xA4:
  case 0xA8:
  case 0xAC:
    {
    //POP rp                  1010 P1 P0 00
    cmd.itype = NEC_78K_0S_pop;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rAX + ((code>>2)&0x03);
    } break;

  case 0xA2:    
  case 0xA6:    
  case 0xAA:    
  case 0xAE:    
    {
    //PUSH rp                 1010 P1 P0 10
    cmd.itype = NEC_78K_0S_push;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rAX + ((code>>2)&0x03);
    } break;

  case 0xB0:    
    {
    //BR AX                   1011 0000
    cmd.itype = NEC_78K_0S_br;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rAX;
    } break;

  case 0xB1:    
    {
    //SUBC saddr,#byte        1011 0001      Saddr-offset   Data
    cmd.itype = NEC_78K_0S_subc;
    cmd.Op1.type  = o_mem;
    cmd.Op1.dtyp  = dt_byte;
    cmd.Op1.addr  = saddr(ua_next_byte());
    cmd.Op2.type  = o_imm;
    cmd.Op2.value = ua_next_byte();
    } break;

  case 0xB2:    
    {
    //BR !addr16              1011 0010      Low addr         High addr
    cmd.itype = NEC_78K_0S_br;
    cmd.Op1.type = o_near;
    cmd.Op1.addr16 = 1;
    addr16(cmd.Op1);
    } break;

  case 0xB3:    
    {
    //SUBC A, #byte           1011 0011      Data
    cmd.itype = NEC_78K_0S_subc;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_imm;
    cmd.Op2.value = ua_next_byte();
    } break;

  case 0xB5:    
    {
    //SUBC A, saddr           1011 0101      Saddr-offset
    cmd.itype = NEC_78K_0S_subc;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_mem;
    cmd.Op2.dtyp = dt_byte;
    cmd.Op2.addr = saddr(ua_next_byte());
    } break;

  case 0xB9:    
    {
    //SUBC A, !addr16         1011 1001      Low addr       High addr
    cmd.itype = NEC_78K_0S_subc;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_mem;
    cmd.Op2.dtyp = dt_byte;
    cmd.Op2.addr16 = 1;
    addr16(cmd.Op2);
    } break;

  case 0xBD:    
    {
    //SUBC A, [HL+byte]       1011 1101      Data
    cmd.itype = NEC_78K_0S_subc;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_reg;
    cmd.Op2.reg  = rHL;
    cmd.Op2.addr = ua_next_byte();
    cmd.Op2.prepost =1;
    cmd.Op2.xmode   =1;
    } break;

  case 0xBF:    
    {
    //SUBC A, [HL]            1011 1111
    cmd.itype = NEC_78K_0S_subc;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_reg;
    cmd.Op2.reg  = rHL;
    cmd.Op2.prepost =1;
    } break;


  case 0xC0:    
    {
    //XCH A, X                1100 0000
    cmd.itype = NEC_78K_0S_xch;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rA;
    cmd.Op2.type = o_reg;
    cmd.Op2.reg  = rX;
    } break;

  case 0xC2:    
    {
    //SUBW AX, #word          1100 0010      Low byte       High byte
    cmd.itype = NEC_78K_0S_subw;
    cmd.Op1.type  = o_reg;
    cmd.Op1.reg   = rAX;
    imm16(cmd.Op2);
    } break;

  case 0xC5:    
    {
    //INC saddr               1100 0101      Saddr-offset
    cmd.itype = NEC_78K_0S_inc;
    cmd.Op1.type = o_mem;
    cmd.Op1.dtyp = dt_byte;
    cmd.Op1.addr = saddr(ua_next_byte());
    } break;

  case 0xC4:    
  case 0xC8:    
  case 0xCC:    
    {
    //XCHW AX, rp             1100 P1 P0 00                  Only when rp = BC, DE, or HL.
    cmd.itype = NEC_78K_0S_xchw;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rAX;
    cmd.Op2.type = o_reg;
    cmd.Op2.reg  = ((code>>2)&0x03)+rAX;
    } break;

  case 0xD2:    
    {
    //ADDW AX, #word          1101 0010      Low byte       High byte
    cmd.itype = NEC_78K_0S_addw;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rAX;
    imm16(cmd.Op2);
    } break;

  case 0xD5:    
    {
    //DEC saddr               1101 0101      Saddr-offset
    cmd.itype = NEC_78K_0S_inc;
    cmd.Op1.type = o_mem;
    cmd.Op1.dtyp = dt_byte;
    cmd.Op1.addr = saddr(ua_next_byte());
    } break;

  case 0xD6:    
    {
    //MOVW AX, saddrp         1101 0110      Saddr-offset
    //MOVW AX, SP             1101 0110      0001 1100
    cmd.itype = NEC_78K_0S_movw;
    uchar tst =  ua_next_byte();
    if(tst==0x1C)
      {
      cmd.Op1.type = o_reg;
      cmd.Op1.reg  = rAX;
      cmd.Op2.type = o_reg;
      cmd.Op2.reg  = rSP;
      }
    else
      {
      if((tst&0x01)!=0)
        {
        // ��室 � �訡��� ERROR
        return 0;
        }
      // movw AX,saddrp
      cmd.Op1.type = o_reg;
      cmd.Op1.reg  = rAX;
      cmd.Op2.type = o_mem;
      cmd.Op2.dtyp = dt_word;
      cmd.Op2.addr = saddr(tst);
      }
    } break;

  case 0xD4:    
  case 0xD8:    
  case 0xDC:    
    {
    //MOVW AX, rp             1101 P1 P0 00                  Only when rp = BC, DE, or HL.
    cmd.itype = NEC_78K_0S_movw;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rAX;
    cmd.Op2.type = o_reg;
    cmd.Op2.reg  = ((code>>2)&0x03)+rAX;
    } break;

  case 0xE2:    
    {
    //CMPW AX, #word          1110 0010      Low byte       High byte
    cmd.itype = NEC_78K_0S_cmpw;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rAX;
    cmd.Op2.type = o_imm;
    imm16(cmd.Op2);
    } break;

  case 0xEB:    
    {
    //MOV [DE], A             1110 1011
    cmd.itype = NEC_78K_0S_mov;
    cmd.Op2.type = o_reg;
    cmd.Op2.reg  = rA;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rDE;
    cmd.Op1.prepost =1;
    } break;

  case 0xEF:    
    {
    //MOV [HL], A             1110 1111
    cmd.itype = NEC_78K_0S_mov;
    cmd.Op2.type = o_reg;
    cmd.Op2.reg  = rA;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = rHL;
    cmd.Op1.prepost =1;
    } break;

  case 0xE9:    
    {
    //MOV !addr16, A          1110 1001      Low addr       High addr
    cmd.itype = NEC_78K_0S_mov;
    cmd.Op1.type = o_mem;
    cmd.Op1.dtyp = dt_byte;
    cmd.Op1.addr16 = 1;
    addr16(cmd.Op1);
    cmd.Op2.type = o_reg;
    cmd.Op2.reg  = rA;
    } break;

  case 0xE5:    
    {
    //MOV saddr, A            1110 0101      Saddr-offset
    //MOV PSW, A              1110 0101      00011110
    cmd.itype = NEC_78K_0S_mov;
    cmd.Op2.type = o_reg;
    cmd.Op2.reg = rA;
    uchar tst = ua_next_byte();
    if(tst == 0x1E)
      {
      cmd.Op1.type = o_reg;
      cmd.Op1.reg  = rPSW;
      }
    else
      {
      cmd.Op1.type = o_mem;
      cmd.Op1.dtyp = dt_byte;
      cmd.Op1.addr = saddr(tst);
      }
    } break;

  case 0xE6:    
    {
    //MOVW saddrp, AX         1110 0110      Saddr-offset
    //MOVW SP, AX             1110 0110      0001 1100
    cmd.itype = NEC_78K_0S_movw;
    uchar byte2 = ua_next_byte();
    if(byte2==0x1C)
      {
      cmd.Op1.type = o_reg;
      cmd.Op1.reg  = rSP;
      cmd.Op2.type = o_reg;
      cmd.Op2.reg  = rAX;
      }
    else
      {
      cmd.Op1.type = o_mem;
      cmd.Op1.dtyp = dt_word;
      cmd.Op1.addr = saddr(byte2);
      cmd.Op2.type = o_reg;
      cmd.Op2.reg  = rAX;
      }
    } break;

  case 0xED:    
    {
    //MOV [HL+byte], A        1110 1101      Data
    cmd.itype = NEC_78K_0S_mov;
    cmd.Op1.type    = o_reg;
    cmd.Op1.reg     = rHL;
    cmd.Op1.value   = ua_next_byte();
    cmd.Op1.prepost = 1;
    cmd.Op1.xmode   = 1; 
    cmd.Op2.type    = o_reg;
    cmd.Op2.reg     = rA;
    } break;

  case 0xE7:    
    {
    //MOV sfr, A              1110 0111      Sfr-offset
    cmd.itype =  NEC_78K_0S_mov;
    cmd.Op1.type = o_mem;
    cmd.Op1.dtyp = dt_byte;
    cmd.Op1.addr = sfr(ua_next_byte());
    cmd.Op2.type = o_reg;
    cmd.Op2.reg  = rA;
    } break;

  case 0xE4:    
  case 0xE8:    
  case 0xEC:    
    {
    //MOVW rp, AX             1110 P1 P0 00                  Only when rp = BC, DE, or HL.
    cmd.itype = NEC_78K_0S_movw;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = ((code>>2)&0x03)+rAX;
    cmd.Op2.type = o_reg;
    cmd.Op2.reg  = rAX;
    } break;

  case 0xF5:    
    {
    //MOV saddr, #byte        1111 0101      Saddr-offset   Data
    //MOV PSW, #byte          1111 0101      00011110       Data
    cmd.itype = NEC_78K_0S_mov;
    uchar tst = ua_next_byte();
    if(tst != 0x1E)
      {
      cmd.Op1.type = o_mem;
      cmd.Op1.dtyp = dt_byte;
      cmd.Op1.addr = saddr(tst);
      }
    else
      {
      cmd.Op1.type = o_reg;
      cmd.Op1.reg  = rPSW;
      }
    cmd.Op2.type  = o_imm;
    cmd.Op2.value = ua_next_byte();
    } break;

  case 0xF7:    
    {
    //MOV sfr, #byte          1111 0111      Sfr-offset     Data
    cmd.itype = NEC_78K_0S_mov;
    cmd.Op1.type  = o_mem;
    cmd.Op1.dtyp  = dt_byte;
    cmd.Op1.addr  = sfr(ua_next_byte());
    cmd.Op2.type  = o_imm;
    cmd.Op2.value = ua_next_byte();
    } break;

  case 0xF0:    
  case 0xF4:    
  case 0xF8:    
  case 0xFC:    
    {
    //MOVW rp, #word          1111 P1 P0 00   Low byte      High byte
    cmd.itype = NEC_78K_0S_movw;
    cmd.Op1.type = o_reg;
    cmd.Op1.reg  = ((code>>2)&0x03)+rAX;
    cmd.Op2.type = o_imm;
    imm16(cmd.Op2);
    } break;

  default:
    {
    if((code&0xC1)==0x40)
      {
      //CALLT [addr5]           01 ta4 to 0 0
      cmd.itype = NEC_78K_0S_callt;
      ulong addr = (code&0x3E)+0x40;
      cmd.Op1.type = o_near;
      cmd.Op1.form = 1;
      cmd.Op1.addr = get_byte(addr)|(get_byte(addr+1)<<8);
      }
    else
      {
      // ��室 � �訡��� ERROR
      return 0;
      }
    }
  } // END main switch
return( cmd.size );
}
//-------------------------------------------------------------------------------