#include "Core/DSP/DSPTables.h"

#if 0

namespace
{
using namespace DSP;

static const char* PartypeToString(partype_t type)
{
  switch (type)
  {
  case P_NONE:
    return "P_NONE";
  case P_VAL:
    return "P_VAL";
  case P_IMM:
    return "P_IMM";
  case P_MEM:
    return "P_MEM";
  case P_STR:
    return "P_STR";
  case P_ADDR_I:
    return "P_ADDR_I";
  case P_ADDR_D:
    return "P_ADDR_D";
  case P_REG:
    return "P_REG";
  case P_REG04:
    return "P_REG04";
  case P_REG08:
    return "P_REG08";
  case P_REG18:
    return "P_REG18";
  case P_REGM18:
    return "P_REGM18";
  case P_REG19:
    return "P_REG19";
  case P_REGM19:
    return "P_REGM19";
  case P_REG1A:
    return "P_REG1A";
  // case P_REG1C:
  //   return "P_REG1C";
  case P_ACCL:
    return "P_ACCL";
  case P_ACCM:
    return "P_ACCM";
  case P_ACCM_D:
    return "P_ACCM_D";
  case P_ACC:
    return "P_ACC";
  case P_ACC_D:
    return "P_ACC_D";
  case P_AX:
    return "P_AX";
  case P_REGS_MASK:
    return "P_REGS_MASK";
  // case P_REF:
  //   return "P_REF";
  case P_PRG:
    return "P_PRG";
  default:
    printf("ERROR: unknown partype %i\n", type);
    abort();
  }
}

const std::vector<DSPOPCTemplate> s_opcodes = {{
    //                                                      # of parameters----+   {type, size, loc,
    //                                                      lshift, mask}
    //                                                      branch        reads PC       //
    //                                                      instruction approximation
    // name      opcode  mask    interpreter function  JIT function    size-V  V   param 1
    // param 2                       param 3                    extendable    uncond.       updates
    // SR
    // {"NOP",      0x0000, 0xfffc, nullptr,     nullptr,    1, 0, {},
    // false, false, false, false, false}, // no operation

    {"DAR",
     0x0004,
     0xfffc,
     nullptr,
     nullptr,
     1,
     1,
     {{P_REG, 1, 0, 0, 0x0003}},
     false,
     false,
     false,
     false,
     false},  // $arD--
    {"IAR",
     0x0008,
     0xfffc,
     nullptr,
     nullptr,
     1,
     1,
     {{P_REG, 1, 0, 0, 0x0003}},
     false,
     false,
     false,
     false,
     false},  // $arD++
    {"SUBARN",
     0x000c,
     0xfffc,
     nullptr,
     nullptr,
     1,
     1,
     {{P_REG, 1, 0, 0, 0x0003}},
     false,
     false,
     false,
     false,
     false},  // $arD -= $ixS
    {"ADDARN",
     0x0010,
     0xfff0,
     nullptr,
     nullptr,
     1,
     2,
     {{P_REG, 1, 0, 0, 0x0003}, {P_REG04, 1, 0, 2, 0x000c}},
     false,
     false,
     false,
     false,
     false},  // $arD += $ixS

    // {"HALT",     0x0021, 0xffff, nullptr,    nullptr,   1, 0, {},
    // false, true, true, false, false}, // halt until reset

    // {"RETGE",    0x02d0, 0xffff, nullptr,     nullptr,    1, 0, {},
    // false, true, false, true, false}, // return if greater or equal
    // {"RETL",     0x02d1, 0xffff, nullptr,     nullptr,    1, 0, {},
    // false, true, false, true, false}, // return if less
    // {"RETG",     0x02d2, 0xffff, nullptr,     nullptr,    1, 0, {},
    // false, true, false, true, false}, // return if greater
    // {"RETLE",    0x02d3, 0xffff, nullptr,     nullptr,    1, 0, {},
    // false, true, false, true, false}, // return if less or equal
    // {"RETNZ",    0x02d4, 0xffff, nullptr,     nullptr,    1, 0, {},
    // false, true, false, true, false}, // return if not zero
    // {"RETZ",     0x02d5, 0xffff, nullptr,     nullptr,    1, 0, {},
    // false, true, false, true, false}, // return if zero
    // {"RETNC",    0x02d6, 0xffff, nullptr,     nullptr,    1, 0, {},
    // false, true, false, true, false}, // return if not carry
    // {"RETC",     0x02d7, 0xffff, nullptr,     nullptr,    1, 0, {},
    // false, true, false, true, false}, // return if carry
    // {"RETx8",    0x02d8, 0xffff, nullptr,     nullptr,    1, 0, {},
    // false, true, false, true, false}, // return if TODO
    // {"RETx9",    0x02d9, 0xffff, nullptr,     nullptr,    1, 0, {},
    // false, true, false, true, false}, // return if TODO
    // {"RETxA",    0x02da, 0xffff, nullptr,     nullptr,    1, 0, {},
    // false, true, false, true, false}, // return if TODO
    // {"RETxB",    0x02db, 0xffff, nullptr,     nullptr,    1, 0, {},
    // false, true, false, true, false}, // return if TODO
    // {"RETLNZ",   0x02dc, 0xffff, nullptr,     nullptr,    1, 0, {},
    // false, true, false, true, false}, // return if logic not zero
    // {"RETLZ",    0x02dd, 0xffff, nullptr,     nullptr,    1, 0, {},
    // false, true, false, true, false}, // return if logic zero
    // {"RETO",     0x02de, 0xffff, nullptr,     nullptr,    1, 0, {},
    // false, true, false, true, false}, // return if overflow
    // {"RET",      0x02df, 0xffff, nullptr,     nullptr,    1, 0, {},
    // false, true, true, false, false}, // unconditional return

    // {"RTI",      0x02ff, 0xffff, nullptr,     nullptr,    1, 0, {},
    // false, true, true, false, false}, // return from interrupt

    // {"CALLGE",   0x02b0, 0xffff, nullptr,    nullptr,   2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}},
    // false, true, false, true, false}, // call if greater or equal
    // {"CALLL",    0x02b1, 0xffff, nullptr,    nullptr,   2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}},
    // false, true, false, true, false}, // call if less
    // {"CALLG",    0x02b2, 0xffff, nullptr,    nullptr,   2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}},
    // false, true, false, true, false}, // call if greater
    // {"CALLLE",   0x02b3, 0xffff, nullptr,    nullptr,   2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}},
    // false, true, false, true, false}, // call if less or equal
    // {"CALLNZ",   0x02b4, 0xffff, nullptr,    nullptr,   2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}},
    // false, true, false, true, false}, // call if not zero
    // {"CALLZ",    0x02b5, 0xffff, nullptr,    nullptr,   2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}},
    // false, true, false, true, false}, // call if zero
    // {"CALLNC",   0x02b6, 0xffff, nullptr,    nullptr,   2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}},
    // false, true, false, true, false}, // call if not carry
    // {"CALLC",    0x02b7, 0xffff, nullptr,    nullptr,   2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}},
    // false, true, false, true, false}, // call if carry
    // {"CALLx8",   0x02b8, 0xffff, nullptr,    nullptr,   2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}},
    // false, true, false, true, false}, // call if TODO
    // {"CALLx9",   0x02b9, 0xffff, nullptr,    nullptr,   2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}},
    // false, true, false, true, false}, // call if TODO
    // {"CALLxA",   0x02ba, 0xffff, nullptr,    nullptr,   2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}},
    // false, true, false, true, false}, // call if TODO
    // {"CALLxB",   0x02bb, 0xffff, nullptr,    nullptr,   2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}},
    // false, true, false, true, false}, // call if TODO
    // {"CALLLNZ",  0x02bc, 0xffff, nullptr,    nullptr,   2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}},
    // false, true, false, true, false}, // call if logic not zero
    // {"CALLLZ",   0x02bd, 0xffff, nullptr,    nullptr,   2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}},
    // false, true, false, true, false}, // call if logic zero
    // {"CALLO",    0x02be, 0xffff, nullptr,    nullptr,   2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}},
    // false, true, false, true, false}, // call if overflow
    // {"CALL",     0x02bf, 0xffff, nullptr,    nullptr,   2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}},
    // false, true, true, true, false},  // unconditional call

    // {"IFGE",     0x0270, 0xffff, nullptr,    nullptr,   1, 0, {},
    // false, true, false, true, false}, // if greater or equal
    // {"IFL",      0x0271, 0xffff, nullptr,    nullptr,   1, 0, {},
    // false, true, false, true, false}, // if less
    // {"IFG",      0x0272, 0xffff, nullptr,    nullptr,   1, 0, {},
    // false, true, false, true, false}, // if greater
    // {"IFLE",     0x0273, 0xffff, nullptr,    nullptr,   1, 0, {},
    // false, true, false, true, false}, // if less or equal
    // {"IFNZ",     0x0274, 0xffff, nullptr,    nullptr,   1, 0, {},
    // false, true, false, true, false}, // if not zero
    // {"IFZ",      0x0275, 0xffff, nullptr,    nullptr,   1, 0, {},
    // false, true, false, true, false}, // if zero
    // {"IFNC",     0x0276, 0xffff, nullptr,    nullptr,   1, 0, {},
    // false, true, false, true, false}, // if not carry
    // {"IFC",      0x0277, 0xffff, nullptr,    nullptr,   1, 0, {},
    // false, true, false, true, false}, // if carry
    // {"IFx8",     0x0278, 0xffff, nullptr,    nullptr,   1, 0, {},
    // false, true, false, true, false}, // if TODO
    // {"IFx9",     0x0279, 0xffff, nullptr,    nullptr,   1, 0, {},
    // false, true, false, true, false}, // if TODO
    // {"IFxA",     0x027a, 0xffff, nullptr,    nullptr,   1, 0, {},
    // false, true, false, true, false}, // if TODO
    // {"IFxB",     0x027b, 0xffff, nullptr,    nullptr,   1, 0, {},
    // false, true, false, true, false}, // if TODO
    // {"IFLNZ",    0x027c, 0xffff, nullptr,    nullptr,   1, 0, {},
    // false, true, false, true, false}, // if logic not zero
    // {"IFLZ",     0x027d, 0xffff, nullptr,    nullptr,   1, 0, {},
    // false, true, false, true, false}, // if logic zero
    // {"IFO",      0x027e, 0xffff, nullptr,    nullptr,   1, 0, {},
    // false, true, false, true, false}, // if overflow
    // {"IF",       0x027f, 0xffff, nullptr,    nullptr,   1, 0, {},
    // false, true, true, true, false},  // what is this, I don't even...

    // {"JGE",      0x0290, 0xffff, nullptr,     nullptr,    2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}},
    // false, true, false, true, false}, // jump if greater or equal
    // {"JL",       0x0291, 0xffff, nullptr,     nullptr,    2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}},
    // false, true, false, true, false}, // jump if less
    // {"JG",       0x0292, 0xffff, nullptr,     nullptr,    2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}},
    // false, true, false, true, false}, // jump if greater
    // {"JLE",      0x0293, 0xffff, nullptr,     nullptr,    2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}},
    // false, true, false, true, false}, // jump if less or equal
    // {"JNZ",      0x0294, 0xffff, nullptr,     nullptr,    2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}},
    // false, true, false, true, false}, // jump if not zero
    // {"JZ",       0x0295, 0xffff, nullptr,     nullptr,    2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}},
    // false, true, false, true, false}, // jump if zero
    // {"JNC",      0x0296, 0xffff, nullptr,     nullptr,    2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}},
    // false, true, false, true, false}, // jump if not carry
    // {"JC",       0x0297, 0xffff, nullptr,     nullptr,    2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}},
    // false, true, false, true, false}, // jump if carry
    // {"JMPx8",    0x0298, 0xffff, nullptr,     nullptr,    2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}},
    // false, true, false, true, false}, // jump if TODO
    // {"JMPx9",    0x0299, 0xffff, nullptr,     nullptr,    2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}},
    // false, true, false, true, false}, // jump if TODO
    // {"JMPxA",    0x029a, 0xffff, nullptr,     nullptr,    2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}},
    // false, true, false, true, false}, // jump if TODO
    // {"JMPxB",    0x029b, 0xffff, nullptr,     nullptr,    2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}},
    // false, true, false, true, false}, // jump if TODO
    // {"JLNZ",     0x029c, 0xffff, nullptr,     nullptr,    2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}},
    // false, true, false, true, false}, // jump if logic not zero
    // {"JLZ",      0x029d, 0xffff, nullptr,     nullptr,    2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}},
    // false, true, false, true, false}, // jump if logic zero
    // {"JO",       0x029e, 0xffff, nullptr,     nullptr,    2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}},
    // false, true, false, true, false}, // jump if overflow
    // {"JMP",      0x029f, 0xffff, nullptr,     nullptr,    2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}},
    // false, true, true, true, false},  // unconditional jump

    // {"JRGE",     0x1700, 0xff1f, nullptr,  nullptr, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}},
    // false, true, false, false, false}, // jump to $R if greater or equal
    // {"JRL",      0x1701, 0xff1f, nullptr,  nullptr, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}},
    // false, true, false, false, false}, // jump to $R if less
    // {"JRG",      0x1702, 0xff1f, nullptr,  nullptr, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}},
    // false, true, false, false, false}, // jump to $R if greater
    // {"JRLE",     0x1703, 0xff1f, nullptr,  nullptr, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}},
    // false, true, false, false, false}, // jump to $R if less or equal
    // {"JRNZ",     0x1704, 0xff1f, nullptr,  nullptr, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}},
    // false, true, false, false, false}, // jump to $R if not zero
    // {"JRZ",      0x1705, 0xff1f, nullptr,  nullptr, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}},
    // false, true, false, false, false}, // jump to $R if zero
    // {"JRNC",     0x1706, 0xff1f, nullptr,  nullptr, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}},
    // false, true, false, false, false}, // jump to $R if not carry
    // {"JRC",      0x1707, 0xff1f, nullptr,  nullptr, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}},
    // false, true, false, false, false}, // jump to $R if carry
    // {"JMPRx8",   0x1708, 0xff1f, nullptr,  nullptr, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}},
    // false, true, false, false, false}, // jump to $R if TODO
    // {"JMPRx9",   0x1709, 0xff1f, nullptr,  nullptr, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}},
    // false, true, false, false, false}, // jump to $R if TODO
    // {"JMPRxA",   0x170a, 0xff1f, nullptr,  nullptr, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}},
    // false, true, false, false, false}, // jump to $R if TODO
    // {"JMPRxB",   0x170b, 0xff1f, nullptr,  nullptr, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}},
    // false, true, false, false, false}, // jump to $R if TODO
    // {"JRLNZ",    0x170c, 0xff1f, nullptr,  nullptr, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}},
    // false, true, false, false, false}, // jump to $R if logic not zero
    // {"JRLZ",     0x170d, 0xff1f, nullptr,  nullptr, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}},
    // false, true, false, false, false}, // jump to $R if logic zero
    // {"JRO",      0x170e, 0xff1f, nullptr,  nullptr, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}},
    // false, true, false, false, false}, // jump to $R if overflow
    // {"JMPR",     0x170f, 0xff1f, nullptr,  nullptr, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}},
    // false, true, true, false, false},  // jump to $R

    // {"CALLRGE",  0x1710, 0xff1f, nullptr,   nullptr,  1, 1, {{P_REG, 1, 0, 5, 0x00e0}},
    // false, true, false, true, false}, // call $R if greater or equal
    // {"CALLRL",   0x1711, 0xff1f, nullptr,   nullptr,  1, 1, {{P_REG, 1, 0, 5, 0x00e0}},
    // false, true, false, true, false}, // call $R if less
    // {"CALLRG",   0x1712, 0xff1f, nullptr,   nullptr,  1, 1, {{P_REG, 1, 0, 5, 0x00e0}},
    // false, true, false, true, false}, // call $R if greater
    // {"CALLRLE",  0x1713, 0xff1f, nullptr,   nullptr,  1, 1, {{P_REG, 1, 0, 5, 0x00e0}},
    // false, true, false, true, false}, // call $R if less or equal
    // {"CALLRNZ",  0x1714, 0xff1f, nullptr,   nullptr,  1, 1, {{P_REG, 1, 0, 5, 0x00e0}},
    // false, true, false, true, false}, // call $R if not zero
    // {"CALLRZ",   0x1715, 0xff1f, nullptr,   nullptr,  1, 1, {{P_REG, 1, 0, 5, 0x00e0}},
    // false, true, false, true, false}, // call $R if zero
    // {"CALLRNC",  0x1716, 0xff1f, nullptr,   nullptr,  1, 1, {{P_REG, 1, 0, 5, 0x00e0}},
    // false, true, false, true, false}, // call $R if not carry
    // {"CALLRC",   0x1717, 0xff1f, nullptr,   nullptr,  1, 1, {{P_REG, 1, 0, 5, 0x00e0}},
    // false, true, false, true, false}, // call $R if carry
    // {"CALLRx8",  0x1718, 0xff1f, nullptr,   nullptr,  1, 1, {{P_REG, 1, 0, 5, 0x00e0}},
    // false, true, false, true, false}, // call $R if TODO
    // {"CALLRx9",  0x1719, 0xff1f, nullptr,   nullptr,  1, 1, {{P_REG, 1, 0, 5, 0x00e0}},
    // false, true, false, true, false}, // call $R if TODO
    // {"CALLRxA",  0x171a, 0xff1f, nullptr,   nullptr,  1, 1, {{P_REG, 1, 0, 5, 0x00e0}},
    // false, true, false, true, false}, // call $R if TODO
    // {"CALLRxB",  0x171b, 0xff1f, nullptr,   nullptr,  1, 1, {{P_REG, 1, 0, 5, 0x00e0}},
    // false, true, false, true, false}, // call $R if TODO
    // {"CALLRLNZ", 0x171c, 0xff1f, nullptr,   nullptr,  1, 1, {{P_REG, 1, 0, 5, 0x00e0}},
    // false, true, false, true, false}, // call $R if logic not zero
    // {"CALLRLZ",  0x171d, 0xff1f, nullptr,   nullptr,  1, 1, {{P_REG, 1, 0, 5, 0x00e0}},
    // false, true, false, true, false}, // call $R if logic zero
    // {"CALLRO",   0x171e, 0xff1f, nullptr,   nullptr,  1, 1, {{P_REG, 1, 0, 5, 0x00e0}},
    // false, true, false, true, false}, // call $R if overflow
    // {"CALLR",    0x171f, 0xff1f, nullptr,   nullptr,  1, 1, {{P_REG, 1, 0, 5, 0x00e0}},
    // false, true, true, true, false},  // call $R

    // {"SBCLR",    0x1200, 0xff00, nullptr,   nullptr,  1, 1, {{P_IMM, 1, 0, 0, 0x0007}},
    // false, false, false, false, false}, // $sr &= ~(I + 6)
    // {"SBSET",    0x1300, 0xff00, nullptr,   nullptr,  1, 1, {{P_IMM, 1, 0, 0, 0x0007}},
    // false, false, false, false, false}, // $sr |= (I + 6)

    {"LSL",
     0x1400,
     0xfec0,
     nullptr,
     nullptr,
     1,
     2,
     {{P_ACC, 1, 0, 8, 0x0100}, {P_IMM, 1, 0, 0, 0x003f}},
     false,
     false,
     false,
     false,
     true},  // $acR <<= I
    {"LSR",
     0x1440,
     0xfec0,
     nullptr,
     nullptr,
     1,
     2,
     {{P_ACC, 1, 0, 8, 0x0100}, {P_IMM, 1, 0, 0, 0x003f}},
     false,
     false,
     false,
     false,
     true},  // $acR >>= I (shifting in zeros)
    {"ASL",
     0x1480,
     0xfec0,
     nullptr,
     nullptr,
     1,
     2,
     {{P_ACC, 1, 0, 8, 0x0100}, {P_IMM, 1, 0, 0, 0x003f}},
     false,
     false,
     false,
     false,
     true},  // $acR <<= I
    {"ASR",
     0x14c0,
     0xfec0,
     nullptr,
     nullptr,
     1,
     2,
     {{P_ACC, 1, 0, 8, 0x0100}, {P_IMM, 1, 0, 0, 0x003f}},
     false,
     false,
     false,
     false,
     true},  // $acR >>= I (shifting in sign bits)

    // these two were discovered by ector
    {"LSRN",
     0x02ca,
     0xffff,
     nullptr,
     nullptr,
     1,
     0,
     {},
     false,
     false,
     false,
     false,
     true},  // $ac0 >>=/<<= $ac1.m[0-6]
    {"ASRN",
     0x02cb,
     0xffff,
     nullptr,
     nullptr,
     1,
     0,
     {},
     false,
     false,
     false,
     false,
     true},  // $ac0 >>=/<<= $ac1.m[0-6] (arithmetic)

    {"LRI",
     0x0080,
     0xffe0,
     nullptr,
     nullptr,
     2,
     2,
     {{P_REG, 1, 0, 0, 0x001f}, {P_IMM, 2, 1, 0, 0xffff}},
     false,
     false,
     false,
     true,
     false},  // $D = I
    {"LR",
     0x00c0,
     0xffe0,
     nullptr,
     nullptr,
     2,
     2,
     {{P_REG, 1, 0, 0, 0x001f}, {P_MEM, 2, 1, 0, 0xffff}},
     false,
     false,
     false,
     true,
     false},  // $D = MEM[M]
    {"SR",
     0x00e0,
     0xffe0,
     nullptr,
     nullptr,
     2,
     2,
     {{P_MEM, 2, 1, 0, 0xffff}, {P_REG, 1, 0, 0, 0x001f}},
     false,
     false,
     false,
     true,
     false},  // MEM[M] = $S

    {"MRR",
     0x1c00,
     0xfc00,
     nullptr,
     nullptr,
     1,
     2,
     {{P_REG, 1, 0, 5, 0x03e0}, {P_REG, 1, 0, 0, 0x001f}},
     false,
     false,
     false,
     false,
     false},  // $D = $S

    {"SI",
     0x1600,
     0xff00,
     nullptr,
     nullptr,
     2,
     2,
     {{P_MEM, 1, 0, 0, 0x00ff}, {P_IMM, 2, 1, 0, 0xffff}},
     false,
     false,
     false,
     true,
     false},  // MEM[M] = I

    {"ADDIS",
     0x0400,
     0xfe00,
     nullptr,
     nullptr,
     1,
     2,
     {{P_ACCM, 1, 0, 8, 0x0100}, {P_IMM, 1, 0, 0, 0x00ff}},
     false,
     false,
     false,
     false,
     true},  // $acD.hm += I
    {"CMPIS",
     0x0600,
     0xfe00,
     nullptr,
     nullptr,
     1,
     2,
     {{P_ACCM, 1, 0, 8, 0x0100}, {P_IMM, 1, 0, 0, 0x00ff}},
     false,
     false,
     false,
     false,
     true},  // FLAGS($acD - I)
    {"LRIS",
     0x0800,
     0xf800,
     nullptr,
     nullptr,
     1,
     2,
     {{P_REG18, 1, 0, 8, 0x0700}, {P_IMM, 1, 0, 0, 0x00ff}},
     false,
     false,
     false,
     false,
     true},  // $(D+24) = I

    {"ADDI",
     0x0200,
     0xfeff,
     nullptr,
     nullptr,
     2,
     2,
     {{P_ACCM, 1, 0, 8, 0x0100}, {P_IMM, 2, 1, 0, 0xffff}},
     false,
     false,
     false,
     true,
     true},  // $acD.hm += I
    {"XORI",
     0x0220,
     0xfeff,
     nullptr,
     nullptr,
     2,
     2,
     {{P_ACCM, 1, 0, 8, 0x0100}, {P_IMM, 2, 1, 0, 0xffff}},
     false,
     false,
     false,
     true,
     true},  // $acD.m ^= I
    {"ANDI",
     0x0240,
     0xfeff,
     nullptr,
     nullptr,
     2,
     2,
     {{P_ACCM, 1, 0, 8, 0x0100}, {P_IMM, 2, 1, 0, 0xffff}},
     false,
     false,
     false,
     true,
     true},  // $acD.m &= I
    {"ORI",
     0x0260,
     0xfeff,
     nullptr,
     nullptr,
     2,
     2,
     {{P_ACCM, 1, 0, 8, 0x0100}, {P_IMM, 2, 1, 0, 0xffff}},
     false,
     false,
     false,
     true,
     true},  // $acD.m |= I
    {"CMPI",
     0x0280,
     0xfeff,
     nullptr,
     nullptr,
     2,
     2,
     {{P_ACCM, 1, 0, 8, 0x0100}, {P_IMM, 2, 1, 0, 0xffff}},
     false,
     false,
     false,
     true,
     true},  // FLAGS(($acD.hm - I) | $acD.l)

    {"ANDF",
     0x02a0,
     0xfeff,
     nullptr,
     nullptr,
     2,
     2,
     {{P_ACCM, 1, 0, 8, 0x0100}, {P_IMM, 2, 1, 0, 0xffff}},
     false,
     false,
     false,
     true,
     true},  // $sr.LZ = ($acD.m & I) == 0 ? 1 : 0
    {"ANDCF",
     0x02c0,
     0xfeff,
     nullptr,
     nullptr,
     2,
     2,
     {{P_ACCM, 1, 0, 8, 0x0100}, {P_IMM, 2, 1, 0, 0xffff}},
     false,
     false,
     false,
     true,
     true},  // $sr.LZ = ($acD.m & I) == I ? 1 : 0

    // {"ILRR",     0x0210, 0xfefc, nullptr,    nullptr,   1, 2, {{P_ACCM, 1, 0, 8, 0x0100},
    // {P_PRG, 1, 0, 0, 0x0003}},                               false, false, false, false, false},
    // // $acD.m = IMEM[$arS]
    // {"ILRRD",    0x0214, 0xfefc, nullptr,   nullptr,  1, 2, {{P_ACCM, 1, 0, 8, 0x0100},
    // {P_PRG, 1, 0, 0, 0x0003}},                               false, false, false, false, false},
    // // $acD.m = IMEM[$arS--]
    // {"ILRRI",    0x0218, 0xfefc, nullptr,   nullptr,  1, 2, {{P_ACCM, 1, 0, 8, 0x0100},
    // {P_PRG, 1, 0, 0, 0x0003}},                               false, false, false, false, false},
    // // $acD.m = IMEM[$arS++]
    // {"ILRRN",    0x021c, 0xfefc, nullptr,   nullptr,  1, 2, {{P_ACCM, 1, 0, 8, 0x0100},
    // {P_PRG, 1, 0, 0, 0x0003}},                               false, false, false, false, false},
    // // $acD.m = IMEM[$arS]; $arS += $ixS

    // LOOPS
    // {"LOOP",     0x0040, 0xffe0, nullptr,    nullptr,   1, 1, {{P_REG, 1, 0, 0, 0x001f}},
    // false, true, true, true, false}, // run next instruction $R times
    // {"BLOOP",    0x0060, 0xffe0, nullptr,   nullptr,  2, 2, {{P_REG, 1, 0, 0, 0x001f},
    // {P_ADDR_I, 2, 1, 0, 0xffff}},                            false, true, true, true, false}, //
    // COMEFROM addr $R times
    // {"LOOPI",    0x1000, 0xff00, nullptr,   nullptr,  1, 1, {{P_IMM, 1, 0, 0, 0x00ff}},
    // false, true, true, true, false}, // run next instruction I times
    // {"BLOOPI",   0x1100, 0xff00, nullptr,  nullptr, 2, 2, {{P_IMM, 1, 0, 0, 0x00ff},
    // {P_ADDR_I, 2, 1, 0, 0xffff}},                            false, true, true, true, false}, //
    // COMEFROM addr I times

    // load and store value pointed by indexing reg and increment; LRR/SRR variants
    {"LRR",
     0x1800,
     0xff80,
     nullptr,
     nullptr,
     1,
     2,
     {{P_REG, 1, 0, 0, 0x001f}, {P_PRG, 1, 0, 5, 0x0060}},
     false,
     false,
     false,
     false,
     false},  // $D = MEM[$arS]
    {"LRRD",
     0x1880,
     0xff80,
     nullptr,
     nullptr,
     1,
     2,
     {{P_REG, 1, 0, 0, 0x001f}, {P_PRG, 1, 0, 5, 0x0060}},
     false,
     false,
     false,
     false,
     false},  // $D = MEM[$arS--]
    {"LRRI",
     0x1900,
     0xff80,
     nullptr,
     nullptr,
     1,
     2,
     {{P_REG, 1, 0, 0, 0x001f}, {P_PRG, 1, 0, 5, 0x0060}},
     false,
     false,
     false,
     false,
     false},  // $D = MEM[$arS++]
    {"LRRN",
     0x1980,
     0xff80,
     nullptr,
     nullptr,
     1,
     2,
     {{P_REG, 1, 0, 0, 0x001f}, {P_PRG, 1, 0, 5, 0x0060}},
     false,
     false,
     false,
     false,
     false},  // $D = MEM[$arS]; $arS += $ixS

    {"SRR",
     0x1a00,
     0xff80,
     nullptr,
     nullptr,
     1,
     2,
     {{P_PRG, 1, 0, 5, 0x0060}, {P_REG, 1, 0, 0, 0x001f}},
     false,
     false,
     false,
     false,
     false},  // MEM[$arD] = $S
    {"SRRD",
     0x1a80,
     0xff80,
     nullptr,
     nullptr,
     1,
     2,
     {{P_PRG, 1, 0, 5, 0x0060}, {P_REG, 1, 0, 0, 0x001f}},
     false,
     false,
     false,
     false,
     false},  // MEM[$arD--] = $S
    {"SRRI",
     0x1b00,
     0xff80,
     nullptr,
     nullptr,
     1,
     2,
     {{P_PRG, 1, 0, 5, 0x0060}, {P_REG, 1, 0, 0, 0x001f}},
     false,
     false,
     false,
     false,
     false},  // MEM[$arD++] = $S
    {"SRRN",
     0x1b80,
     0xff80,
     nullptr,
     nullptr,
     1,
     2,
     {{P_PRG, 1, 0, 5, 0x0060}, {P_REG, 1, 0, 0, 0x001f}},
     false,
     false,
     false,
     false,
     false},  // MEM[$arD] = $S; $arD += $ixD

    // 2
    {"LRS",
     0x2000,
     0xf800,
     nullptr,
     nullptr,
     1,
     2,
     {{P_REG18, 1, 0, 8, 0x0700}, {P_MEM, 1, 0, 0, 0x00ff}},
     false,
     false,
     false,
     false,
     false},  // $(D+24) = MEM[($cr[0-7] << 8) | I]
    {"SRS",
     0x2800,
     0xf800,
     nullptr,
     nullptr,
     1,
     2,
     {{P_MEM, 1, 0, 0, 0x00ff}, {P_REG18, 1, 0, 8, 0x0700}},
     false,
     false,
     false,
     false,
     false},  // MEM[($cr[0-7] << 8) | I] = $(S+24)

    // opcodes that can be extended

    // 3 - main opcode defined by 9 bits, extension defined by last 7 bits!!
    {"XORR",
     0x3000,
     0xfc80,
     nullptr,
     nullptr,
     1,
     2,
     {{P_ACCM, 1, 0, 8, 0x0100}, {P_REG1A, 1, 0, 9, 0x0200}},
     true,
     false,
     false,
     false,
     true},  // $acD.m ^= $axS.h
    {"ANDR",
     0x3400,
     0xfc80,
     nullptr,
     nullptr,
     1,
     2,
     {{P_ACCM, 1, 0, 8, 0x0100}, {P_REG1A, 1, 0, 9, 0x0200}},
     true,
     false,
     false,
     false,
     true},  // $acD.m &= $axS.h
    {"ORR",
     0x3800,
     0xfc80,
     nullptr,
     nullptr,
     1,
     2,
     {{P_ACCM, 1, 0, 8, 0x0100}, {P_REG1A, 1, 0, 9, 0x0200}},
     true,
     false,
     false,
     false,
     true},  // $acD.m |= $axS.h
    {"ANDC",
     0x3c00,
     0xfe80,
     nullptr,
     nullptr,
     1,
     2,
     {{P_ACCM, 1, 0, 8, 0x0100}, {P_ACCM_D, 1, 0, 8, 0x0100}},
     true,
     false,
     false,
     false,
     true},  // $acD.m &= $ac(1-D).m
    {"ORC",
     0x3e00,
     0xfe80,
     nullptr,
     nullptr,
     1,
     2,
     {{P_ACCM, 1, 0, 8, 0x0100}, {P_ACCM_D, 1, 0, 8, 0x0100}},
     true,
     false,
     false,
     false,
     true},  // $acD.m |= $ac(1-D).m
    {"XORC",
     0x3080,
     0xfe80,
     nullptr,
     nullptr,
     1,
     2,
     {{P_ACCM, 1, 0, 8, 0x0100}, {P_ACCM_D, 1, 0, 8, 0x0100}},
     true,
     false,
     false,
     false,
     true},  // $acD.m ^= $ac(1-D).m
    {"NOT",
     0x3280,
     0xfe80,
     nullptr,
     nullptr,
     1,
     1,
     {{P_ACCM, 1, 0, 8, 0x0100}},
     true,
     false,
     false,
     false,
     true},  // $acD.m = ~$acD.m
    {"LSRNRX",
     0x3480,
     0xfc80,
     nullptr,
     nullptr,
     1,
     2,
     {{P_ACC, 1, 0, 8, 0x0100}, {P_REG1A, 1, 0, 9, 0x0200}},
     true,
     false,
     false,
     false,
     true},  // $acD >>=/<<= $axS.h[0-6]
    {"ASRNRX",
     0x3880,
     0xfc80,
     nullptr,
     nullptr,
     1,
     2,
     {{P_ACC, 1, 0, 8, 0x0100}, {P_REG1A, 1, 0, 9, 0x0200}},
     true,
     false,
     false,
     false,
     true},  // $acD >>=/<<= $axS.h[0-6] (arithmetic)
    {"LSRNR",
     0x3c80,
     0xfe80,
     nullptr,
     nullptr,
     1,
     2,
     {{P_ACC, 1, 0, 8, 0x0100}, {P_ACCM_D, 1, 0, 8, 0x0100}},
     true,
     false,
     false,
     false,
     true},  // $acD >>=/<<= $ac(1-D).m[0-6]
    {"ASRNR",
     0x3e80,
     0xfe80,
     nullptr,
     nullptr,
     1,
     2,
     {{P_ACC, 1, 0, 8, 0x0100}, {P_ACCM_D, 1, 0, 8, 0x0100}},
     true,
     false,
     false,
     false,
     true},  // $acD >>=/<<= $ac(1-D).m[0-6] (arithmetic)

    // 4
    {"ADDR",
     0x4000,
     0xf800,
     nullptr,
     nullptr,
     1,
     2,
     {{P_ACC, 1, 0, 8, 0x0100}, {P_REG18, 1, 0, 9, 0x0600}},
     true,
     false,
     false,
     false,
     true},  // $acD += $(S+24)
    {"ADDAX",
     0x4800,
     0xfc00,
     nullptr,
     nullptr,
     1,
     2,
     {{P_ACC, 1, 0, 8, 0x0100}, {P_AX, 1, 0, 9, 0x0200}},
     true,
     false,
     false,
     false,
     true},  // $acD += $axS
    {"ADD",
     0x4c00,
     0xfe00,
     nullptr,
     nullptr,
     1,
     2,
     {{P_ACC, 1, 0, 8, 0x0100}, {P_ACC_D, 1, 0, 8, 0x0100}},
     true,
     false,
     false,
     false,
     true},  // $acD += $ac(1-D)
    {"ADDP",
     0x4e00,
     0xfe00,
     nullptr,
     nullptr,
     1,
     1,
     {{P_ACC, 1, 0, 8, 0x0100}},
     true,
     false,
     false,
     false,
     true},  // $acD += $prod

    // 5
    {"SUBR",
     0x5000,
     0xf800,
     nullptr,
     nullptr,
     1,
     2,
     {{P_ACC, 1, 0, 8, 0x0100}, {P_REG18, 1, 0, 9, 0x0600}},
     true,
     false,
     false,
     false,
     true},  // $acD -= $(S+24)
    {"SUBAX",
     0x5800,
     0xfc00,
     nullptr,
     nullptr,
     1,
     2,
     {{P_ACC, 1, 0, 8, 0x0100}, {P_AX, 1, 0, 9, 0x0200}},
     true,
     false,
     false,
     false,
     true},  // $acD -= $axS
    {"SUB",
     0x5c00,
     0xfe00,
     nullptr,
     nullptr,
     1,
     2,
     {{P_ACC, 1, 0, 8, 0x0100}, {P_ACC_D, 1, 0, 8, 0x0100}},
     true,
     false,
     false,
     false,
     true},  // $acD -= $ac(1-D)
    {"SUBP",
     0x5e00,
     0xfe00,
     nullptr,
     nullptr,
     1,
     1,
     {{P_ACC, 1, 0, 8, 0x0100}},
     true,
     false,
     false,
     false,
     true},  // $acD -= $prod

    // 6
    {"MOVR",
     0x6000,
     0xf800,
     nullptr,
     nullptr,
     1,
     2,
     {{P_ACC, 1, 0, 8, 0x0100}, {P_REG18, 1, 0, 9, 0x0600}},
     true,
     false,
     false,
     false,
     true},  // $acD.hm = $(S+24); $acD.l = 0
    {"MOVAX",
     0x6800,
     0xfc00,
     nullptr,
     nullptr,
     1,
     2,
     {{P_ACC, 1, 0, 8, 0x0100}, {P_AX, 1, 0, 9, 0x0200}},
     true,
     false,
     false,
     false,
     true},  // $acD = $axS
    {"MOV",
     0x6c00,
     0xfe00,
     nullptr,
     nullptr,
     1,
     2,
     {{P_ACC, 1, 0, 8, 0x0100}, {P_ACC_D, 1, 0, 8, 0x0100}},
     true,
     false,
     false,
     false,
     true},  // $acD = $ax(1-D)
    {"MOVP",
     0x6e00,
     0xfe00,
     nullptr,
     nullptr,
     1,
     1,
     {{P_ACC, 1, 0, 8, 0x0100}},
     true,
     false,
     false,
     false,
     true},  // $acD = $prod

    // 7
    {"ADDAXL",
     0x7000,
     0xfc00,
     nullptr,
     nullptr,
     1,
     2,
     {{P_ACC, 1, 0, 8, 0x0100}, {P_REG18, 1, 0, 9, 0x0200}},
     true,
     false,
     false,
     false,
     true},  // $acD += $axS.l
    {"INCM",
     0x7400,
     0xfe00,
     nullptr,
     nullptr,
     1,
     1,
     {{P_ACCM, 1, 0, 8, 0x0100}},
     true,
     false,
     false,
     false,
     true},  // $acsD++
    {"INC",
     0x7600,
     0xfe00,
     nullptr,
     nullptr,
     1,
     1,
     {{P_ACC, 1, 0, 8, 0x0100}},
     true,
     false,
     false,
     false,
     true},  // $acD++
    {"DECM",
     0x7800,
     0xfe00,
     nullptr,
     nullptr,
     1,
     1,
     {{P_ACCM, 1, 0, 8, 0x0100}},
     true,
     false,
     false,
     false,
     true},  // $acsD--
    {"DEC",
     0x7a00,
     0xfe00,
     nullptr,
     nullptr,
     1,
     1,
     {{P_ACC, 1, 0, 8, 0x0100}},
     true,
     false,
     false,
     false,
     true},  // $acD--
    {"NEG",
     0x7c00,
     0xfe00,
     nullptr,
     nullptr,
     1,
     1,
     {{P_ACC, 1, 0, 8, 0x0100}},
     true,
     false,
     false,
     false,
     true},  // $acD = -$acD
    {"MOVNP",
     0x7e00,
     0xfe00,
     nullptr,
     nullptr,
     1,
     1,
     {{P_ACC, 1, 0, 8, 0x0100}},
     true,
     false,
     false,
     false,
     true},  // $acD = -$prod

    // 8
    // {"NX",       0x8000, 0xf700, nullptr,      nullptr,     1, 0, {},
    // true, false, false, false, false}, // extendable nop
    {"CLR",
     0x8100,
     0xf700,
     nullptr,
     nullptr,
     1,
     1,
     {{P_ACC, 1, 0, 11, 0x0800}},
     true,
     false,
     false,
     false,
     true},  // $acD = 0
    // {"CMP",      0x8200, 0xff00, nullptr,     nullptr,    1, 0, {},
    // true, false, false, false, true},  // FLAGS($ac0 - $ac1)
    {"MULAXH",
     0x8300,
     0xff00,
     nullptr,
     nullptr,
     1,
     0,
     {},
     true,
     false,
     false,
     false,
     true},  // $prod = $ax0.h * $ax0.h
    {"CLRP", 0x8400, 0xff00, nullptr, nullptr, 1, 0, {}, true, false, false, false, true},  // $prod
                                                                                            // = 0
    // {"TSTPROD",  0x8500, 0xff00, nullptr, nullptr,1, 0, {},
    // true, false, false, false, true},  // FLAGS($prod)
    // {"TSTAXH",   0x8600, 0xfe00, nullptr,  nullptr, 1, 1, {{P_REG1A, 1, 0, 8, 0x0100}},
    // true, false, false, false, true},  // FLAGS($axR.h)
    // {"M2",       0x8a00, 0xff00, nullptr,  nullptr, 1, 0, {},
    // true, false, false, false, false}, // enable "$prod *= 2" after every multiplication
    // {"M0",       0x8b00, 0xff00, nullptr,  nullptr, 1, 0, {},
    // true, false, false, false, false}, // disable "$prod *= 2" after every multiplication
    // {"CLR15",    0x8c00, 0xff00, nullptr,  nullptr, 1, 0, {},
    // true, false, false, false, false}, // set normal multiplication
    // {"SET15",    0x8d00, 0xff00, nullptr,  nullptr, 1, 0, {},
    // true, false, false, false, false}, // set unsigned multiplication in MUL
    // {"SET16",    0x8e00, 0xff00, nullptr,  nullptr, 1, 0, {},
    // true, false, false, false, false}, // set 16 bit sign extension width
    // {"SET40",    0x8f00, 0xff00, nullptr,  nullptr, 1, 0, {},
    // true, false, false, false, false}, // set 40 bit sign extension width

    // 9
    {"MUL",
     0x9000,
     0xf700,
     nullptr,
     nullptr,
     1,
     2,
     {{P_REG18, 1, 0, 11, 0x0800}, {P_REG1A, 1, 0, 11, 0x0800}},
     true,
     false,
     false,
     false,
     true},  // $prod = $axS.l * $axS.h
    {"ASR16",
     0x9100,
     0xf700,
     nullptr,
     nullptr,
     1,
     1,
     {{P_ACC, 1, 0, 11, 0x0800}},
     true,
     false,
     false,
     false,
     true},  // $acD >>= 16 (shifting in sign bits)
    {"MULMVZ",
     0x9200,
     0xf600,
     nullptr,
     nullptr,
     1,
     3,
     {{P_REG18, 1, 0, 11, 0x0800}, {P_REG1A, 1, 0, 11, 0x0800}, {P_ACC, 1, 0, 8, 0x0100}},
     true,
     false,
     false,
     false,
     true},  // $acR.hm = $prod.hm; $acR.l = 0; $prod = $axS.l * $axS.h
    {"MULAC",
     0x9400,
     0xf600,
     nullptr,
     nullptr,
     1,
     3,
     {{P_REG18, 1, 0, 11, 0x0800}, {P_REG1A, 1, 0, 11, 0x0800}, {P_ACC, 1, 0, 8, 0x0100}},
     true,
     false,
     false,
     false,
     true},  // $acR += $prod; $prod = $axS.l * $axS.h
    {"MULMV",
     0x9600,
     0xf600,
     nullptr,
     nullptr,
     1,
     3,
     {{P_REG18, 1, 0, 11, 0x0800}, {P_REG1A, 1, 0, 11, 0x0800}, {P_ACC, 1, 0, 8, 0x0100}},
     true,
     false,
     false,
     false,
     true},  // $acR = $prod; $prod = $axS.l * $axS.h

    // a-b
    {"MULX",
     0xa000,
     0xe700,
     nullptr,
     nullptr,
     1,
     2,
     {{P_REGM18, 1, 0, 11, 0x1000}, {P_REGM19, 1, 0, 10, 0x0800}},
     true,
     false,
     false,
     false,
     true},  // $prod = $ax0.S * $ax1.T
    {"ABS",
     0xa100,
     0xf700,
     nullptr,
     nullptr,
     1,
     1,
     {{P_ACC, 1, 0, 11, 0x0800}},
     true,
     false,
     false,
     false,
     true},  // $acD = abs($acD)
    {"MULXMVZ",
     0xa200,
     0xe600,
     nullptr,
     nullptr,
     1,
     3,
     {{P_REGM18, 1, 0, 11, 0x1000}, {P_REGM19, 1, 0, 10, 0x0800}, {P_ACC, 1, 0, 8, 0x0100}},
     true,
     false,
     false,
     false,
     true},  // $acR.hm = $prod.hm; $acR.l = 0; $prod = $ax0.S * $ax1.T
    {"MULXAC",
     0xa400,
     0xe600,
     nullptr,
     nullptr,
     1,
     3,
     {{P_REGM18, 1, 0, 11, 0x1000}, {P_REGM19, 1, 0, 10, 0x0800}, {P_ACC, 1, 0, 8, 0x0100}},
     true,
     false,
     false,
     false,
     true},  // $acR += $prod; $prod = $ax0.S * $ax1.T
    {"MULXMV",
     0xa600,
     0xe600,
     nullptr,
     nullptr,
     1,
     3,
     {{P_REGM18, 1, 0, 11, 0x1000}, {P_REGM19, 1, 0, 10, 0x0800}, {P_ACC, 1, 0, 8, 0x0100}},
     true,
     false,
     false,
     false,
     true},  // $acR = $prod; $prod = $ax0.S * $ax1.T
    // {"TST",      0xb100, 0xf700, nullptr,     nullptr,    1, 1, {{P_ACC,    1, 0, 11, 0x0800}},
    // true, false, false, false, true}, // FLAGS($acR)

    // c-d
    {"MULC",
     0xc000,
     0xe700,
     nullptr,
     nullptr,
     1,
     2,
     {{P_ACCM, 1, 0, 12, 0x1000}, {P_REG1A, 1, 0, 11, 0x0800}},
     true,
     false,
     false,
     false,
     true},  // $prod = $acS.m * $axS.h
    {"CMPAR",
     0xc100,
     0xe700,
     nullptr,
     nullptr,
     1,
     2,
     {{P_ACC, 1, 0, 11, 0x0800}, {P_REG1A, 1, 0, 12, 0x1000}},
     true,
     false,
     false,
     false,
     true},  // FLAGS($acS - axR.h)
    {"MULCMVZ",
     0xc200,
     0xe600,
     nullptr,
     nullptr,
     1,
     3,
     {{P_ACCM, 1, 0, 12, 0x1000}, {P_REG1A, 1, 0, 11, 0x0800}, {P_ACC, 1, 0, 8, 0x0100}},
     true,
     false,
     false,
     false,
     true},  // $acR.hm, $acR.l, $prod = $prod.hm, 0, $acS.m * $axS.h
    {"MULCAC",
     0xc400,
     0xe600,
     nullptr,
     nullptr,
     1,
     3,
     {{P_ACCM, 1, 0, 12, 0x1000}, {P_REG1A, 1, 0, 11, 0x0800}, {P_ACC, 1, 0, 8, 0x0100}},
     true,
     false,
     false,
     false,
     true},  // $acR, $prod = $acR + $prod, $acS.m * $axS.h
    {"MULCMV",
     0xc600,
     0xe600,
     nullptr,
     nullptr,
     1,
     3,
     {{P_ACCM, 1, 0, 12, 0x1000}, {P_REG1A, 1, 0, 11, 0x0800}, {P_ACC, 1, 0, 8, 0x0100}},
     true,
     false,
     false,
     false,
     true},  // $acR, $prod = $prod, $acS.m * $axS.h

    // e
    {"MADDX",
     0xe000,
     0xfc00,
     nullptr,
     nullptr,
     1,
     2,
     {{P_REGM18, 1, 0, 8, 0x0200}, {P_REGM19, 1, 0, 7, 0x0100}},
     true,
     false,
     false,
     false,
     true},  // $prod += $ax0.S * $ax1.T
    {"MSUBX",
     0xe400,
     0xfc00,
     nullptr,
     nullptr,
     1,
     2,
     {{P_REGM18, 1, 0, 8, 0x0200}, {P_REGM19, 1, 0, 7, 0x0100}},
     true,
     false,
     false,
     false,
     true},  // $prod -= $ax0.S * $ax1.T
    {"MADDC",
     0xe800,
     0xfc00,
     nullptr,
     nullptr,
     1,
     2,
     {{P_ACCM, 1, 0, 9, 0x0200}, {P_REG19, 1, 0, 7, 0x0100}},
     true,
     false,
     false,
     false,
     true},  // $prod += $acS.m * $axT.h
    {"MSUBC",
     0xec00,
     0xfc00,
     nullptr,
     nullptr,
     1,
     2,
     {{P_ACCM, 1, 0, 9, 0x0200}, {P_REG19, 1, 0, 7, 0x0100}},
     true,
     false,
     false,
     false,
     true},  // $prod -= $acS.m * $axT.h

    // f
    {"LSL16",
     0xf000,
     0xfe00,
     nullptr,
     nullptr,
     1,
     1,
     {{P_ACC, 1, 0, 8, 0x0100}},
     true,
     false,
     false,
     false,
     true},  // $acR <<= 16
    {"MADD",
     0xf200,
     0xfe00,
     nullptr,
     nullptr,
     1,
     2,
     {{P_REG18, 1, 0, 8, 0x0100}, {P_REG1A, 1, 0, 8, 0x0100}},
     true,
     false,
     false,
     false,
     true},  // $prod += $axS.l * $axS.h
    {"LSR16",
     0xf400,
     0xfe00,
     nullptr,
     nullptr,
     1,
     1,
     {{P_ACC, 1, 0, 8, 0x0100}},
     true,
     false,
     false,
     false,
     true},  // $acR >>= 16
    {"MSUB",
     0xf600,
     0xfe00,
     nullptr,
     nullptr,
     1,
     2,
     {{P_REG18, 1, 0, 8, 0x0100}, {P_REG1A, 1, 0, 8, 0x0100}},
     true,
     false,
     false,
     false,
     true},  // $prod -= $axS.l * $axS.h
    {"ADDPAXZ",
     0xf800,
     0xfc00,
     nullptr,
     nullptr,
     1,
     2,
     {{P_ACC, 1, 0, 9, 0x0200}, {P_AX, 1, 0, 8, 0x0100}},
     true,
     false,
     false,
     false,
     true},  // $acD.hm = $prod.hm + $ax.h; $acD.l = 0
    {"CLRL",
     0xfc00,
     0xfe00,
     nullptr,
     nullptr,
     1,
     1,
     {{P_ACCL, 1, 0, 11, 0x0800}},
     true,
     false,
     false,
     false,
     true},  // $acR.l = 0
    {"MOVPZ",
     0xfe00,
     0xfe00,
     nullptr,
     nullptr,
     1,
     1,
     {{P_ACC, 1, 0, 8, 0x0100}},
     true,
     false,
     false,
     false,
     true},  // $acD.hm = $prod.hm; $acD.l = 0
}};

}  // anonymous namespace

#endif
