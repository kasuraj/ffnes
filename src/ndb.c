// 包含头文件
#include <stdio.h>
#include "ndb.h"

// 函数实现
void ndb_init(NDB *ndb, CPU *cpu)
{
    ndb->cpu = cpu;
    ndb_reset(ndb);
}

void ndb_free(NDB *ndb)
{
    // do nothing
}

void ndb_reset(NDB *ndb)
{
    ndb->stop = 0;
    ndb->cond = 0;
}

void ndb_cpu_debug(NDB *ndb)
{
    WORD  * wparam = (WORD *)ndb->param;
    DWORD *dwparam = (DWORD*)ndb->param;

    switch (ndb->cond)
    {
    case NDB_CPU_KEEP_RUNNING:
        ndb->stop = 0;
        break;

    case NDB_CPU_RUN_NSTEPS:
        ndb->stop = --(*dwparam) ? 0 : 1;
        break;

    case NDB_CPU_STOP_PCEQU:
        ndb->stop = (ndb->cpu->pc == *wparam) ? 1 : 0;
        break;
    }

    // while loop make ndb spin here
    while (ndb->stop) Sleep(20);
}

void ndb_cpu_runto(NDB *ndb, int cond, void *param)
{
    WORD  * wparam = (WORD *)ndb->param;
    DWORD *dwparam = (DWORD*)ndb->param;

    ndb->cond = cond;
    switch (cond)
    {
    case NDB_CPU_RUN_NSTEPS:
        *dwparam = *(DWORD*)param;
        if (*dwparam == 0)
        {
            *dwparam  = 1;
            ndb->stop = 1;
            return;
        }
        break;

    case NDB_CPU_STOP_PCEQU:
        * wparam = *( WORD*)param;
        break;
    }
    ndb->stop = 0;
}

int ndb_cpu_dasm(NDB *ndb, WORD pc, BYTE bytes[3], char *str)
{
    static char *s_opcode_strs[256] =
    {
        "BRK", "ORA", " t ", "SLO", "NOP", "ORA", "ASL", "SLO", "PHP", "ORA", "ASL", "ANC", "NOP", "ORA", "ASL", "SLO",
        "BPL", "ORA", " t ", "SLO", "NOP", "ORA", "ASL", "SLO", "CLC", "ORA", "NOP", "SLO", "NOP", "ORA", "ASL", "SLO",
        "JSR", "AND", " t ", "RLA", "BIT", "AND", "ROL", "RLA", "PLP", "AND", "ROL", "ANC", "BIT", "AND", "ROL", "RLA",
        "BMI", "AND", " t ", "RLA", "NOP", "AND", "ROL", "RLA", "SEC", "AND", "NOP", "RLA", "NOP", "AND", "ROL", "RLA",
        "RTI", "EOR", " t ", "SRE", "NOP", "EOR", "LSR", "SRE", "PHA", "EOR", "LSR", "ASR", "JMP", "EOR", "LSR", "SRE",
        "BVC", "EOR", " t ", "SRE", "NOP", "EOR", "LSR", "SRE", "CLI", "EOR", "NOP", "SRE", "NOP", "EOR", "LSR", "SRE",
        "RTS", "ADC", " t ", "RRA", "NOP", "ADC", "ROR", "RRA", "PLA", "ADC", "ROR", "ARR", "JMP", "ADC", "ROR", "RRA",
        "BVS", "ADC", " t ", "RRA", "NOP", "ADC", "ROR", "RRA", "SEI", "ADC", "NOP", "RRA", "NOP", "ADC", "ROR", "RRA",
        "NOP", "STA", "NOP", "SAX", "STY", "STA", "STX", "SAX", "DEY", "NOP", "TXA", "ANE", "STY", "STA", "STX", "SAX",
        "BCC", "STA", " t ", "SHA", "STY", "STA", "STX", "SAX", "TYA", "STA", "TXS", "SHS", "SHY", "STA", "SHX", "SHA",
        "LDY", "LDA", "LDX", "LAX", "LDY", "LDA", "LDX", "LAX", "TAY", "LDA", "TAX", "LXA", "LDY", "LDA", "LDX", "LAX",
        "BCS", "LDA", " t ", "LAX", "LDY", "LDA", "LDX", "LAX", "CLV", "LDA", "TSX", "LAS", "LDY", "LDA", "LDX", "LAX",
        "CPY", "CMP", "NOP", "DCP", "CPY", "CMP", "DEC", "DCP", "INY", "CMP", "DEX", "SBX", "CPY", "CMP", "DEC", "DCP",
        "BNE", "CMP", " t ", "DCP", "NOP", "CMP", "DEC", "DCP", "CLD", "CMP", "NOP", "DCP", "NOP", "CMP", "DEC", "DCP",
        "CPX", "SBC", "NOP", "ISB", "CPX", "SBC", "INC", "ISB", "INX", "SBC", "NOP", "SBC", "CPX", "SBC", "INC", "ISB",
        "BEQ", "SBC", " t ", "ISB", "NOP", "SBC", "INC", "ISB", "SED", "SBC", "NOP", "ISB", "NOP", "SBC", "INC", "ISB",
    };
    static int addr_mode_map[32] = { 1, 7, 1, 7, 2, 2, 2, 2, 0, 1, 0, 1, 4, 4, 4, 4, 10, 8, 9, 8, 3, 3, 3, 3, 0, 6, 0, 6, 5, 5, 5, 5 };

    int am = 0;

    bytes[0] = bus_readb(ndb->cpu->cbus, pc + 0);
    bytes[1] = bus_readb(ndb->cpu->cbus, pc + 1);
    bytes[2] = bus_readb(ndb->cpu->cbus, pc + 2);

    // addressing mode
    am = addr_mode_map[bytes[0] & 0x1f];

    // for special opcode
    switch (bytes[0])
    {
    case 0x00:
    case 0x40:
    case 0x60:
    case 0x89: am =  0; break;

    case 0x96:
    case 0x97:
    case 0xb6:
    case 0xb7: am = 11; break;

    case 0x9e:
    case 0x9f:
    case 0xbe:
    case 0xbf: am =  6; break;

    case 0x20: am =  4; break;
    case 0x6c: am = 12; break;
    }

    // for different addressing mode
    switch (am)
    {
    case  0: sprintf(str, "%s"             , s_opcode_strs[bytes[0]]                     ); return 1; // Implied
    case  1: sprintf(str, "%s #%02X"       , s_opcode_strs[bytes[0]], bytes[1]           ); return 2; // immediate
    case  2: sprintf(str, "%s $%02X"       , s_opcode_strs[bytes[0]], bytes[1]           ); return 2; // zeropage
    case  3: sprintf(str, "%s $%02X, X"    , s_opcode_strs[bytes[0]], bytes[1]           ); return 2; // zeropage,x
    case  4: sprintf(str, "%s $%02X%02X"   , s_opcode_strs[bytes[0]], bytes[2], bytes[1] ); return 3; // absolute
    case  5: sprintf(str, "%s $%02X%02X, X", s_opcode_strs[bytes[0]], bytes[2], bytes[1] ); return 3; // absolute,x
    case  6: sprintf(str, "%s $%02X%02X, Y", s_opcode_strs[bytes[0]], bytes[2], bytes[1] ); return 3; // absolute,y
    case  7: sprintf(str, "%s ($%02X, X)"  , s_opcode_strs[bytes[0]], bytes[1]           ); return 2; // (indir,x)
    case  8: sprintf(str, "%s ($%02X), Y"  , s_opcode_strs[bytes[0]], bytes[1]           ); return 2; // (indir),y
    case  9: sprintf(str, "%s"             , s_opcode_strs[bytes[0]]                     ); return 1; // ?
    case 10: sprintf(str, "%s $%04X"       , s_opcode_strs[bytes[0]], pc+2+(char)bytes[1]); return 2; // relative
    case 11: sprintf(str, "%s $%02X, Y"    , s_opcode_strs[bytes[0]], bytes[1]           ); return 2; // zeropage,y
    case 12: sprintf(str, "%s ($%02X%02X)" , s_opcode_strs[bytes[0]], bytes[2], bytes[1] ); return 3; // indirect, JMP ()
    default: return -1; // this will never happen
    }
}

void ndb_cpu_dump(NDB *ndb, char *str)
{
    static char psflag_chars[8] = {'C', 'Z', 'I', 'D', 'B', '-', 'V', 'N'};
    int  i;

    sprintf(str, "%04X  %02X  %02X  %02X  %02X  ",
        ndb->cpu->pc, ndb->cpu->ax, ndb->cpu->xi, ndb->cpu->yi, ndb->cpu->sp);

    str += strlen(str);
    for (i=7; i>=0; i--)
    {
        if (ndb->cpu->ps & (1 << i)) {
            *str++ = psflag_chars[i];
        }
        else *str++ = '-';
    }
    *str++ = '\0';
}

