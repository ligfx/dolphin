// Copyright 2008 Dolphin Emulator Project
// Copyright 2004 Duddie & Tratax
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "Core/DSP/DSPMemoryMap.h"

#include "Common/Logging/Log.h"

#include "Core/DSP/DSPCore.h"
#include "Core/DSP/DSPHWInterface.h"
#include "Core/DSP/DSPTables.h"

namespace DSP
{
u16 dsp_imem_read(u16 addr)
{
  switch (addr >> 12)
  {
  case 0:  // 0xxx IRAM
    return g_dsp.iram[addr & DSP_IRAM_MASK];

  case 8:  // 8xxx IROM - contains code to receive code for IRAM, and a bunch of mixing loops.
    // if (g_dsp.michael_print)
    // printf("irom read %04x\n", addr);
    return g_dsp.irom[addr & DSP_IROM_MASK];

  default:  // Unmapped/non-existing memory
    ERROR_LOG(DSPLLE, "%04x DSP ERROR: Executing from invalid (%04x) memory", g_dsp.pc, addr);
    return 0;
  }
}

u16 dsp_dmem_read(u16 addr)
{
  u16 value;
  switch (addr >> 12)
  {
  case 0x0:  // 0xxx DRAM
    value = g_dsp.dram[addr & DSP_DRAM_MASK];
    break;

  case 0x1:  // 1xxx COEF
    if (g_dsp.michael_print)
      printf("%04x : Coefficient Read @ %04x\n", g_dsp.pc, addr);
    value = g_dsp.coef[addr & DSP_COEF_MASK];
    break;

  case 0xf:  // Fxxx HW regs
    value = gdsp_ifx_read(addr);
    break;

  default:  // Unmapped/non-existing memory
    ERROR_LOG(DSPLLE, "%04x DSP ERROR: Read from UNKNOWN (%04x) memory", g_dsp.pc, addr);
    value = 0;
    break;
  }
  if (g_dsp.michael_print)
    printf("%04x : dsp dmem read %04x = %04x\n", g_dsp.pc, addr, value);
  return value;
}

void dsp_dmem_write(u16 addr, u16 val)
{
  if (g_dsp.michael_print)
    printf("%04x : dsp dmem write %04x = %04x\n", g_dsp.pc, addr, val);
  switch (addr >> 12)
  {
  case 0x0:  // 0xxx DRAM
    g_dsp.dram[addr & DSP_DRAM_MASK] = val;
    break;

  case 0xf:  // Fxxx HW regs
    gdsp_ifx_write(addr, val);
    break;

  default:  // Unmapped/non-existing memory
    ERROR_LOG(DSPLLE, "%04x DSP ERROR: Write to UNKNOWN (%04x) memory", g_dsp.pc, addr);
    break;
  }
}

u16 dsp_fetch_code()
{
  u16 opc = dsp_imem_read(g_dsp.pc);

  g_dsp.pc++;
  return opc;
}

u16 dsp_peek_code()
{
  return dsp_imem_read(g_dsp.pc);
}

void dsp_skip_inst()
{
  g_dsp.pc += GetOpTemplate(dsp_peek_code())->size;
}
}  // namespace DSP
