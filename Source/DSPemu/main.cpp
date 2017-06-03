#include <cassert>
#include <functional>
#include <vector>

#include "Common/Common.h"
#include "Common/Logging/Log.h"
#include "Common/Logging/LogManager.h"
#include "Core/DSP/DSPCore.h"
#include "Core/DSP/DSPMemoryMap.h"
#include "Core/DSP/Interpreter/DSPInterpreter.h"

#include <OptionParser.h>

#include "dspmem.inl"
#include "hostmem.inl"
#include "stubs.inl"
#include "tables.inl"

#include "Patch81F4.inl"
#include "Patch8458.inl"
#include "Patch8723.inl"
#include "Patch8809.inl"
#include "Patch88E5.inl"
#include "SimpleGBAPatches.inl"

using namespace DSP;

template <typename F>
static bool RunDSP(const std::string& rom, const std::string& coefs, const std::string& ucode,
                   const std::vector<u8>& init_ram, const F& done_func)
{
  DSPInitOptions dsp_options;
  dsp_options.core_type = DSPInitOptions::CoreType::CORE_INTERPRETER;

  if (!LoadDSPRom(dsp_options.irom_contents.data(), rom, DSP::DSP_IROM_BYTE_SIZE))
    return false;
  if (!LoadDSPRom(dsp_options.coef_contents.data(), coefs, DSP::DSP_COEF_BYTE_SIZE))
    return false;

  // if (!SimpleGBAPatches(dsp_options.irom_contents.data()))
  //   return false;
  //
  // if (!Patch8809(dsp_options.irom_contents.data()))
  //   return false;
  // if (!Patch8723(dsp_options.irom_contents.data()))
  //   return false;
  // if (!Patch81F4(dsp_options.irom_contents.data()))
  //   return false;
  // if (!Patch8458(dsp_options.irom_contents.data()))
  //   return false;
  // if (!Patch88E5(dsp_options.irom_contents.data()))
  //   return false;

  InitInstructionTable();
  if (!DSPCore_Init(dsp_options))
  {
    printf("ERROR: Couldn't initialize DSPCore\n");
    return false;
  };

  // accessed at 0x80000000
  g_dsp.cpu_ram = s_cpu_ram.data();

  Interpreter::WriteCR(0x0801);
  RunToIdle();

  if (!LoadFileToRam(0x0, ucode))
  {
    return false;
  };

  // address
  WriteMail(0x80f3a001);
  WriteMail(0x0);

  // ?
  WriteMail(0x80f3c002);
  WriteMail(0x0);

  // size
  WriteMail(0x80f3a002);
  WriteMail(0x1d20);

  // ?
  WriteMail(0x80f3b002);
  WriteMail(0x0);

  // jump address
  WriteMail(0x80f3d001);
  WriteMail(0x10);

  // confirm we're ready
  ReadMail();

  // read parameters from 0x0, and go!
  CopyToRam(0x0, init_ram);
  WriteMail(0xabba0000);
  WriteMail(0);
  ReadMail();

  bool result = done_func();

  // // reset
  // WriteMail(0xcdd10002);

  DSPCore_Shutdown();

  return result;
}

static bool CheckDmem(u16 address, const std::vector<u16>& expected)
{
  g_dsp.michael_print = false;

  bool all_matched = true;
  for (u16 i = 0; i < expected.size(); i++)
  {
    u16 got = dsp_dmem_read(address + i);
    if (expected[i] != got)
    {
      printf("ERROR: @%04x: expected %04x got %04x\n", address + i, expected[i], got);
      all_matched = false;
    }
  }
  return all_matched;
}

int main(int argc, char** argv)
{
  auto parser = std::make_unique<optparse::OptionParser>();
  parser->usage("usage: %prog [options]... [FILE]...").version("DSPemu - " + scm_rev_str);
  parser->add_option("--rom").action("store");
  parser->add_option("--coefs").action("store");
  parser->add_option("--ucode").action("store");
  optparse::Values& options = parser->parse_args(argc, argv);

  if (options["rom"].empty())
  {
    printf("ERROR: missing required argument 'rom'\n");
    return 1;
  }
  if (options["coefs"].empty())
  {
    printf("ERROR: missing required argument 'coefs'\n");
    return 1;
  }

  if (options["ucode"].empty())
  {
    printf("ERROR: missing required argument 'ucode'\n");
    return 1;
  }

  LogManager::Init();

  {
    // Four Swords Adventures trace #1
    printf("FOUR SWORDS ADVENTURES TRACE #1\n");
    if (!RunDSP(options["rom"], options["coefs"], options["ucode"],
                {{
                    0xc3, 0xcf,  // 0x0 challenge nonce part 1
                    0xa4, 0xb2,  // 0x1 challenge nonce part 2
                    0x00, 0x00,  // 0x2, not used?
                    0x00, 0x02,  // 0x3 logo palette
                    0x00, 0x00,  // 0x4, not used?
                    0x00, 0x02,  // 0x5 logo speed
                    0x00, 0x00,  // 0x6 joyboot length hi
                    0x82, 0x9c,  // 0x7 joyboot length lo
                    0x80, 0x4b,  // 0x8 dest addr hi
                    0x9a, 0xc0,  // 0x9 dest addr lo
                }},
                [] {
                  return CheckDmem(0x10, {{
                                             0x0000,  // 0x10
                                             0x0022,  // 0x11
                                             0x2014,  // 0x12
                                             0x0000,  // 0x13
                                             0x829c,  // 0x14
                                             0x0000,  // 0x15
                                             0x0000,  // 0x16
                                             0x80a0,  // 0x17
                                         }}) &&
                         CheckDmem(0x1e, {{
                                             0x6f64,  // 0x1e
                                             0xb2a4,  // 0x1f
                                             0xddc0,  // 0x20
                                             0xaab0,  // 0x21
                                             0xb7d5,  // 0x22
                                             0xc1df,  // 0x23
                                         }});
                }))
      return 1;
  }

  // {
  //   // Crystal Chronicles trace #1
  //   printf("CRYSTAL CHRONICLES TRACE #1\n");
  //   if (!RunDSP(options["rom"], options["coefs"], options["ucode"],
  //               {{
  //                   0xd6, 0x9f,  // 0x0 challenge nonce part 1
  //                   0x85, 0x86,  // 0x1 challenge nonce part 2
  //                   0x00, 0x00,  // 0x2, not used?
  //                   0x00, 0x00,  // 0x3 logo palette
  //                   0x00, 0x00,  // 0x4, not used?
  //                   0x00, 0x02,  // 0x5 logo speed
  //                   0x00, 0x02,  // 0x6 joyboot length hi
  //                   0xe0, 0x58,  // 0x7 joyboot length lo
  //                   0x80, 0x32,  // 0x8 dest addr hi
  //                   0x72, 0x20,  // 0x9 dest addr lo
  //               }},
  //               [] {
  //                 return true;
  //                 return CheckDmem(0x10, {{
  //                                            0x0000,  // 0x10
  //                                            0x0003,  // 0x11
  //                                            0x374b,  // 0x12
  //                                            0x0000,  // 0x13
  //                                            0xe058,  // 0x14
  //                                            0x0002,  // 0x15
  //                                            0x0002,  // 0x16
  //                                            0xde58,  // 0x17
  //                                        }}) &&
  //                        CheckDmem(0x1e, {{
  //                                            0x6f64,  // 0x1e
  //                                            0x8685,  // 0x1f
  //                                            0xe9e1,  // 0x20
  //                                            0xfaa5,  // 0x21
  //                                            0xeae7,  // 0x22
  //                                            0xd2b8,  // 0x23
  //                                        }});
  //               }))
  //     return 1;
  // }

  return 0;
}
