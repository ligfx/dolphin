#include "Common/FileUtil.h"
#include "Common/Logging/Log.h"
#include "Common/Swap.h"
#include "Core/DSP/DSPAnalyzer.h"
#include "Core/DSP/DSPAssembler.h"
#include "Core/DSP/DSPCore.h"
#include "Core/DSP/DSPHWInterface.h"

#include <string>

namespace
{
using namespace DSP;

static bool LoadDSPRom(u16* rom, const std::string& filename, u32 size_in_bytes)
{
  std::string bytes;
  if (!File::ReadFileToString(filename, bytes))
  {
    printf("Couldn't open '%s'", filename.c_str());
    return false;
  }

  if (bytes.size() != size_in_bytes)
  {
    printf("%s has a wrong size (%zu, expected %u)", filename.c_str(), bytes.size(), size_in_bytes);
    return false;
  }

  const u16* words = reinterpret_cast<const u16*>(bytes.c_str());
  for (u32 i = 0; i < size_in_bytes / 2; ++i)
    rom[i] = Common::swap16(words[i]);

  return true;
}

static void RunToIdle()
{
  printf("running\n");
  do
  {
    if (g_dsp.cr & CR_HALT)
    {
      return;
    }

    // printf("g_dsp.pc %04x\n", g_dsp.pc);

    // if (g_dsp.pc == 0x8078) {
    //   printf("Changing PC 8078 -> 804d");
    //   g_dsp.pc = 0x804d;
    // }
    //  else
    // if (g_dsp.pc == 0x807e) {
    //   printf("At wait_for_dsp_mbox_807e, returning\n");
    //   g_dsp.pc = dsp_reg_load_stack(StackRegister::Call);
    //   // g_dsp.pc = 0x8047;
    //   // return;
    // } else if (g_dsp.pc == )
    //   return;
    //   printf("Changing PC 807e -> 804d");
    //   g_dsp.pc = 0x804d;
    // }

    if (g_dsp.pc == 0x1b3)
    {
      g_dsp.cr &= CR_HALT;
      break;
    }

    DSPCore_Step();
    DSPCore_RunCycles(1);
  } while (!(Analyzer::GetCodeFlags(g_dsp.pc) & Analyzer::CODE_IDLE_SKIP));
  printf("done\n");
}

static void WriteMail(u32 mail)
{
  gdsp_mbox_write_h(MAILBOX_CPU, mail >> 16);
  gdsp_mbox_write_l(MAILBOX_CPU, mail & 0xffff);
  RunToIdle();
}

static void ReadMail()
{
  (void)gdsp_mbox_read_l(MAILBOX_DSP);
  RunToIdle();
}

static bool PatchAt(u16* loc, const std::string& patch)
{
  DSPAssembler assembler({});
  std::vector<u16> code;
  if (!assembler.Assemble(patch, code, nullptr))
  {
    printf("ERROR: %s\n", assembler.GetErrorString().c_str());
    return false;
  }
  printf("patched %lu shorts\n", code.size());
  std::copy(code.begin(), code.end(), loc);

  return true;
}

}  // anonymous namespace
