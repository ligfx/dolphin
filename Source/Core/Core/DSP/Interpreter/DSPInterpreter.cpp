// Copyright 2008 Dolphin Emulator Project
// Copyright 2004 Duddie & Tratax
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "Core/DSP/Interpreter/DSPInterpreter.h"

#include "Common/CommonTypes.h"
#include "Common/Logging/Log.h"

#include "Core/DSP/DSPAnalyzer.h"
#include "Core/DSP/DSPCore.h"
#include "Core/DSP/DSPHost.h"
#include "Core/DSP/DSPMemoryMap.h"
#include "Core/DSP/DSPTables.h"

namespace DSP
{
namespace Interpreter
{
namespace
{
void ExecuteInstruction(const UDSPInstruction inst)
{
  const DSPOPCTemplate* opcode_template = GetOpTemplate(inst);

  if (opcode_template->extended)
  {
    GetExtOpTemplate(inst)->intFunc(inst);
  }

  opcode_template->intFunc(inst);

  if (opcode_template->extended)
  {
    applyWriteBackLog();
  }
}
}  // Anonymous namespace

// NOTE: These have nothing to do with g_dsp.r.cr !
void WriteCR(u16 val)
{
  // reset
  if (val & 1)
  {
    INFO_LOG(DSPLLE, "DSP_CONTROL RESET");
    DSPCore_Reset();
    val &= ~1;
  }
  // init
  else if (val == 4)
  {
    // HAX!
    // OSInitAudioSystem ucode should send this mail - not DSP core itself
    INFO_LOG(DSPLLE, "DSP_CONTROL INIT");
    g_init_hax = true;
    val |= 0x800;
  }

  // update cr
  g_dsp.cr = val;
}

u16 ReadCR()
{
  if (g_dsp.pc & 0x8000)
  {
    g_dsp.cr |= 0x800;
  }
  else
  {
    g_dsp.cr &= ~0x800;
  }

  return g_dsp.cr;
}

void Step()
{
  DSPCore_CheckExceptions();

  g_dsp.step_counter++;

#if PROFILE
  g_dsp.err_pc = g_dsp.pc;

  ProfilerAddDelta(g_dsp.err_pc, 1);
  if (g_dsp.step_counter == 1)
  {
    ProfilerInit();
  }

  if ((g_dsp.step_counter & 0xFFFFF) == 0)
  {
    ProfilerDump(g_dsp.step_counter);
  }
#endif

  u16 opc = dsp_fetch_code();
  ExecuteInstruction(UDSPInstruction(opc));

  if (Analyzer::GetCodeFlags(static_cast<u16>(g_dsp.pc - 1u)) & Analyzer::CODE_LOOP_END)
    HandleLoop();
}

static bool RunSingleCycle(int& cycles_left)
{
  if (g_dsp.cr & CR_HALT)
  {
    cycles_left = 0;
    return false;
  }

  if (g_dsp.external_interrupt_waiting && Host::OnThread())
  {
    DSPCore_CheckExternalInterrupt();
    DSPCore_SetExternalInterrupt(false);
  }

// Seems to slow things down
#if defined(_DEBUG) || defined(DEBUGFAST)
  if (g_dsp_breakpoints.IsAddressBreakPoint(g_dsp.pc))
  {
    DSPCore_SetState(State::Stepping);
    return false;
  }
#endif

  Step();
  cycles_left--;
  return true;
}

static bool MovePastIdleSkip(int& cycles_left)
{
  u16 idle_start = g_dsp.pc;
  // Largest seen: at 0x06c9 in ucode 2FCDF1EC (Four Swords), there is a nine-instruction idle loop
  for (size_t i = 0; i < 32; i++)
  {
    if (!RunSingleCycle(cycles_left))
      return false;
    if (g_dsp.pc == idle_start)
      return false;
  }
  return true;
}

int RunCyclesThread(int cycles)
{
  while (cycles > 0)
  {
    if (!RunSingleCycle(cycles))
      return cycles;
  }

  // Don't end on an idle skip without running it once
  while (Analyzer::GetCodeFlags(g_dsp.pc) & Analyzer::CODE_IDLE_SKIP)
    if (!MovePastIdleSkip(cycles))
      break;

  return cycles;
}

int RunCycles(int cycles)
{
  while (cycles > 0)
  {
    if (Analyzer::GetCodeFlags(g_dsp.pc) & Analyzer::CODE_IDLE_SKIP)
    {
      if (!MovePastIdleSkip(cycles))
        return cycles;
    }
    else
    {
      if (!RunSingleCycle(cycles))
        return cycles;
    }
  }

  // Don't end on an idle skip without running it once
  while (Analyzer::GetCodeFlags(g_dsp.pc) & Analyzer::CODE_IDLE_SKIP)
    if (!MovePastIdleSkip(cycles))
      break;

  return cycles;
}

void nop(const UDSPInstruction opc)
{
  // The real nop is 0. Anything else is bad.
  if (opc == 0)
    return;

  ERROR_LOG(DSPLLE, "LLE: Unrecognized opcode 0x%04x", opc);
}

}  // namespace Interpreter
}  // namespace DSP
