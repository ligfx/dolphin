// Copyright 2008 Dolphin Emulator Project
// Copyright 2004 Duddie & Tratax
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "Core/DSP/Interpreter/DSPInterpreter.h"

#include "Common/CommonTypes.h"
#include "Common/Logging/Log.h"

#include "Core/DSP/DSPAnalyzer.h"
#include "Core/DSP/DSPCore.h"
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

// static bool in_rom = false;
static bool in_function_of_interest = false;

// struct DSP_Regs
// {
//   u16 ar[4];
//   u16 ix[4];
//   u16 wr[4];
//   u16 st[4];
//   u16 cr;
//   u16 sr;
//
//   union
//   {
//     u64 val;
//     struct
//     {
//       u16 l;
//       u16 m;
//       u16 h;
//       u16 m2;  // if this gets in the way, drop it.
//     };
//   } prod;
//
//   union
//   {
//     u32 val;
//     struct
//     {
//       u16 l;
//       u16 h;
//     };
//   } ax[2];
//
//   union
//   {
//     u64 val;
//     struct
//     {
//       u16 l;
//       u16 m;
//       u16 h;
//     };
//   } ac[2];
// };

// static DSP_Regs old_regs{};

static void PrintRegs()
{
  printf("== Registers ==\n");
  printf("AR0:   %04x     AR1:   %04x     AR2:   %04x     AR3:   %04x\n", g_dsp.r.ar[0],
         g_dsp.r.ar[1], g_dsp.r.ar[2], g_dsp.r.ar[3]);
  printf("IX0:   %04x     IX1:   %04x     IX2:   %04x     IX3:   %04x\n", g_dsp.r.ix[0],
         g_dsp.r.ix[1], g_dsp.r.ix[2], g_dsp.r.ix[3]);
  printf("AX0.H: %04x     AX0.L: %04x     AX1.H: %04x     AX1.L: %04x\n", g_dsp.r.ax[0].h,
         g_dsp.r.ax[0].l, g_dsp.r.ax[1].h, g_dsp.r.ax[1].l);
  printf("AC0.H: %04x     AC0.M: %04x     AC0.L: %04x\n", g_dsp.r.ac[0].h, g_dsp.r.ac[0].m,
         g_dsp.r.ac[0].l);
  printf("AC1.H: %04x     AC1.M: %04x     AC1.L: %04x\n", g_dsp.r.ac[1].h, g_dsp.r.ac[1].m,
         g_dsp.r.ac[1].l);
  printf("PROD: %016llx\n", g_dsp.r.prod.val);
  printf("SR: %04x\n", g_dsp.r.sr);
}

static u16 last_pc;
// static u16 old_dmem[2];

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

  // if (g_dsp.pc == 0x88e5 && last_pc == 0x0128)
  // if (g_dsp.pc == 0x008c)
  if (g_dsp.pc == 0x808b)
  // if (g_dsp.michael_print && g_dsp.pc & 0x8000 && !(last_pc & 0x8000))
  // if (g_dsp.pc == 0x88e5)
  {
    printf("STARTED %04x -> %04x\n", last_pc, g_dsp.pc);
    // if (last_pc == 0x0128)
    // {
    //   g_dsp.r.ar[0] = 0x0;
    //   g_dsp.r.ar[1] = 0x0;
    //   g_dsp.r.ar[2] = 0x0;
    //   g_dsp.r.ar[3] = 0x0;
    //   g_dsp.r.ix[0] = 0x0;
    //   g_dsp.r.ix[1] = 0x0;
    //   g_dsp.r.ix[2] = 0x0;
    //   g_dsp.r.ix[3] = 0x0;
    //   // g_dsp.r.ax[0].h = 0xff;
    //   g_dsp.r.ax[0].l = 0x0;
    //   g_dsp.r.ax[1].h = 0x0;
    //   g_dsp.r.ax[1].l = 0x0;
    //   g_dsp.r.ac[0].h = 0x0;
    //   g_dsp.r.ac[1].h = 0x0;
    //   g_dsp.r.ac[0].m = 0x0;
    //   g_dsp.r.ac[1].m = 0x0;
    //   // g_dsp.r.ac[0].l = 0xfe00;
    //   g_dsp.r.ac[1].l = 0x0;
    // }
    PrintRegs();
    // old_regs = g_dsp.r;
    // old_dmem[0] = dsp_dmem_read(g_dsp.r.ar[2]);
    // old_dmem[1] = dsp_dmem_read(g_dsp.r.ar[2] + 1);
    // printf("DRAM[%04x] = %04x\n", g_dsp.r.ar[2], old_dmem[0]);
    // printf("DRAM[%04x] = %04x\n", g_dsp.r.ar[2] + 1, old_dmem[1]);
    g_dsp.michael_print = in_function_of_interest = true;
  }

  // if (g_dsp.michael_print)
  //   printf("pc %04x\n", g_dsp.pc);

  u16 opc = dsp_fetch_code();
  ExecuteInstruction(UDSPInstruction(opc));

  if (Analyzer::GetCodeFlags(static_cast<u16>(g_dsp.pc - 1u)) & Analyzer::CODE_LOOP_END)
    HandleLoop();

  if (in_function_of_interest && !(g_dsp.pc & 0x8000))
  // if (in_function_of_interest && g_dsp.pc == 0x0043)
  {
    printf("STOPPED: %04x -> %04x\n", last_pc, g_dsp.pc);
    PrintRegs();
    g_dsp.michael_print = in_function_of_interest = false;

    // u16 ac1m = old_dmem[0];
    // u16 ac1l = old_dmem[1];
    // u16 ac0m = (old_regs.ac[0].m & ~old_regs.ac[0].h) | old_regs.ax[0].h + ac1m;
    // u16 ac0l = ac1l + old_regs.ac[0].l;
    // u16 dmem1 = ac0m;
    // u16 dmem2 = ac0l;
    //
    // printf("AC0.M: calculated %04x actual %04x\n", ac0m, g_dsp.r.ac[0].m);
    // printf("AC0.L: calculated %04x actual %04x\n", ac0l, g_dsp.r.ac[0].l);
    // printf("AC1.M: calculated %04x actual %04x\n", ac1m, g_dsp.r.ac[1].m);
    // printf("AC1.L: calculated %04x actual %04x\n", ac1l, g_dsp.r.ac[1].l);
    // printf("DRAM[%04x]: calculated %04x actual %04x\n", g_dsp.r.ar[2] - 1, dmem1,
    //        dsp_dmem_read(g_dsp.r.ar[2] - 1));
    // printf("DRAM[%04x]: calculated %04x actual %04x\n", g_dsp.r.ar[2], dmem2,
    //        dsp_dmem_read(g_dsp.r.ar[2]));
    // printf("\n");
  }

  last_pc = g_dsp.pc;
}

// Used by thread mode.
int RunCyclesThread(int cycles)
{
  while (true)
  {
    if (g_dsp.cr & CR_HALT)
      return 0;

    if (g_dsp.external_interrupt_waiting)
    {
      DSPCore_CheckExternalInterrupt();
      DSPCore_SetExternalInterrupt(false);
    }

    Step();
    cycles--;
    if (cycles < 0)
      return 0;
  }
}

// This one has basic idle skipping, and checks breakpoints.
int RunCyclesDebug(int cycles)
{
  // First, let's run a few cycles with no idle skipping so that things can progress a bit.
  for (int i = 0; i < 8; i++)
  {
    if (g_dsp.cr & CR_HALT)
      return 0;
    if (g_dsp_breakpoints.IsAddressBreakPoint(g_dsp.pc))
    {
      DSPCore_SetState(State::Stepping);
      return cycles;
    }
    Step();
    cycles--;
    if (cycles < 0)
      return 0;
  }

  while (true)
  {
    // Next, let's run a few cycles with idle skipping, so that we can skip
    // idle loops.
    for (int i = 0; i < 8; i++)
    {
      if (g_dsp.cr & CR_HALT)
        return 0;
      if (g_dsp_breakpoints.IsAddressBreakPoint(g_dsp.pc))
      {
        DSPCore_SetState(State::Stepping);
        return cycles;
      }
      // Idle skipping.
      if (Analyzer::GetCodeFlags(g_dsp.pc) & Analyzer::CODE_IDLE_SKIP)
        return 0;
      Step();
      cycles--;
      if (cycles < 0)
        return 0;
    }

    // Now, lets run some more without idle skipping.
    for (int i = 0; i < 200; i++)
    {
      if (g_dsp_breakpoints.IsAddressBreakPoint(g_dsp.pc))
      {
        DSPCore_SetState(State::Stepping);
        return cycles;
      }
      Step();
      cycles--;
      if (cycles < 0)
        return 0;
      // We don't bother directly supporting pause - if the main emu pauses,
      // it just won't call this function anymore.
    }
  }
}

// Used by non-thread mode. Meant to be efficient.
int RunCycles(int cycles)
{
  // First, let's run a few cycles with no idle skipping so that things can
  // progress a bit.
  for (int i = 0; i < 8; i++)
  {
    if (g_dsp.cr & CR_HALT)
      return 0;
    Step();
    cycles--;
    if (cycles < 0)
      return 0;
  }

  while (true)
  {
    // Next, let's run a few cycles with idle skipping, so that we can skip
    // idle loops.
    for (int i = 0; i < 8; i++)
    {
      if (g_dsp.cr & CR_HALT)
        return 0;
      // Idle skipping.
      if (Analyzer::GetCodeFlags(g_dsp.pc) & Analyzer::CODE_IDLE_SKIP)
        return 0;
      Step();
      cycles--;
      if (cycles < 0)
        return 0;
    }

    // Now, lets run some more without idle skipping.
    for (int i = 0; i < 200; i++)
    {
      Step();
      cycles--;
      if (cycles < 0)
        return 0;
      // We don't bother directly supporting pause - if the main emu pauses,
      // it just won't call this function anymore.
    }
  }
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
