#include <cassert>
#include <functional>
#include <numeric>
#include <set>
#include <vector>

#include "Common/Common.h"
#include "Common/File.h"
#include "Common/Logging/Log.h"
#include "Common/Logging/LogManager.h"
#include "Common/StringUtil.h"
#include "Common/Swap.h"
#include "Core/DSP/DSPCore.h"
#include "Core/DSP/DSPDisassembler.h"
#include "Core/DSP/DSPMemoryMap.h"
#include "Core/DSP/Interpreter/DSPInterpreter.h"

#include <OptionParser.h>

// #include "dspmem.inl"
// #include "hostmem.inl"
#include "stubs.inl"
// #include "tables.inl"
//
// #include "Patch81F4.inl"
// #include "Patch8458.inl"
// #include "Patch8723.inl"
// #include "Patch8809.inl"
// #include "Patch88E5.inl"
// #include "SimpleGBAPatches.inl"

using namespace DSP;
//
// template <typename F>
// static bool RunDSP(const std::string& rom, const std::string& coefs, const std::string& ucode,
//                    const std::vector<u8>& init_ram, const F& done_func)
// {
//   DSPInitOptions dsp_options;
//   dsp_options.core_type = DSPInitOptions::CoreType::CORE_INTERPRETER;
//
//   if (!LoadDSPRom(dsp_options.irom_contents.data(), rom, DSP::DSP_IROM_BYTE_SIZE))
//     return false;
//   if (!LoadDSPRom(dsp_options.coef_contents.data(), coefs, DSP::DSP_COEF_BYTE_SIZE))
//     return false;
//
//   // if (!SimpleGBAPatches(dsp_options.irom_contents.data()))
//   //   return false;
//   //
//   // if (!Patch8809(dsp_options.irom_contents.data()))
//   //   return false;
//   // if (!Patch8723(dsp_options.irom_contents.data()))
//   //   return false;
//   // if (!Patch81F4(dsp_options.irom_contents.data()))
//   //   return false;
//   // if (!Patch8458(dsp_options.irom_contents.data()))
//   //   return false;
//   // if (!Patch88E5(dsp_options.irom_contents.data()))
//   //   return false;
//
//   InitInstructionTable();
//   if (!DSPCore_Init(dsp_options))
//   {
//     printf("ERROR: Couldn't initialize DSPCore\n");
//     return false;
//   };
//
//   // accessed at 0x80000000
//   g_dsp.cpu_ram = s_cpu_ram.data();
//
//   Interpreter::WriteCR(0x0801);
//   RunToIdle();
//
//   if (!LoadFileToRam(0x0, ucode))
//   {
//     return false;
//   };
//
//   // address
//   WriteMail(0x80f3a001);
//   WriteMail(0x0);
//
//   // ?
//   WriteMail(0x80f3c002);
//   WriteMail(0x0);
//
//   // size
//   WriteMail(0x80f3a002);
//   WriteMail(0x1d20);
//
//   // ?
//   WriteMail(0x80f3b002);
//   WriteMail(0x0);
//
//   // jump address
//   WriteMail(0x80f3d001);
//   WriteMail(0x10);
//
//   // confirm we're ready
//   ReadMail();
//
//   // read parameters from 0x0, and go!
//   CopyToRam(0x0, init_ram);
//   WriteMail(0xabba0000);
//   WriteMail(0);
//   ReadMail();
//
//   bool result = done_func();
//
//   // // reset
//   // WriteMail(0xcdd10002);
//
//   DSPCore_Shutdown();
//
//   return result;
// }
//
// static bool CheckDmem(u16 address, const std::vector<u16>& expected)
// {
//   g_dsp.michael_print = false;
//
//   bool all_matched = true;
//   for (u16 i = 0; i < expected.size(); i++)
//   {
//     u16 got = dsp_dmem_read(address + i);
//     if (expected[i] != got)
//     {
//       printf("ERROR: @%04x: expected %04x got %04x\n", address + i, expected[i], got);
//       all_matched = false;
//     }
//   }
//   return all_matched;
// }

uint32_t GetOpParameterValue(const DSP::param2_t& param, u16 op1, u16 op2)
{
  u32 val = (param.loc >= 1) ? op2 : op1;
  val &= param.mask;
  if (param.lshift < 0)
    val = val << (-param.lshift);
  else
    val = val >> param.lshift;

  u32 type = param.type;
  if ((type & 0xff) == 0x10)
    type &= 0xff00;

  if (type & P_REG)
  {
    // Check for _D parameter - if so flip.
    if ((type == P_ACC_D) || (type == P_ACCM_D))  // Used to be P_ACCM_D TODO verify
      val = (~val & 0x1) | ((type & P_REGS_MASK) >> 8);
    else
      val |= (type & P_REGS_MASK) >> 8;
    type &= ~P_REGS_MASK;
  }

  switch (type)
  {
  case P_IMM:
    // TODO: should this modify val?
    // if (param.size != 2)
    // {
    //   if (param.mask == 0x003f)  // LSL, LSR, ASL, ASR
    //     buf += StringFromFormat("#%d",
    //                             (val & 0x20) ? (val | 0xFFFFFFC0) : val);  // 6-bit sign
    //                             extension
    //   else
    //     buf += StringFromFormat("#0x%02x", val);
    // }
    // else
    // {
    //   buf += StringFromFormat("#0x%04x", val);
    // }
    break;

  case P_MEM:
    if (param.size != 2)
      val = (u16)(s16)(s8)val;
    break;

  case P_REG:
  case P_PRG:
  case P_VAL:
  case P_ADDR_I:
  case P_ADDR_D:
    break;

  default:
    assert(false);
    // ERROR_LOG(DSPLLE, "Unknown parameter type: %x", opc.params[j].type);
    break;
  }

  return val;
}

struct Inst
{
  uint16_t pc = 0;
  uint16_t size = 0;
  const DSP::DSPOPCTemplate* opc = nullptr;
  const DSP::DSPOPCTemplate* opc_ext = nullptr;
  uint16_t op1 = 0;
  uint16_t op2 = 0;
};

struct Block
{
  uint16_t pc = 0;
  std::vector<Inst> instructions;
};

int main(int argc, char** argv)
{
  // auto parser = std::make_unique<optparse::OptionParser>();
  // parser->usage("usage: %prog [options]... [FILE]...").version("DSPemu - " + scm_rev_str);
  // parser->add_option("--rom").action("store");
  // parser->add_option("--coefs").action("store");
  // parser->add_option("--ucode").action("store");
  // optparse::Values& options = parser->parse_args(argc, argv);

  // if (options["rom"].empty())
  // {
  //   printf("ERROR: missing required argument 'rom'\n");
  //   return 1;
  // }
  // if (options["coefs"].empty())
  // {
  //   printf("ERROR: missing required argument 'coefs'\n");
  //   return 1;
  // }
  //
  // if (options["ucode"].empty())
  // {
  //   printf("ERROR: missing required argument 'ucode'\n");
  //   return 1;
  // }

  // LogManager::Init();

  File::IOFile file("DSP_UC_4E8A8B21.bin", "rb");

  std::vector<uint16_t> ucode(file.GetSize() / sizeof(uint16_t));
  file.ReadArray(ucode.data(), ucode.size());

  for (size_t i = 0; i < ucode.size(); i++)
  {
    ucode[i] = Common::swap16(ucode[i]);
  }

  std::vector<Block> blocks;
  std::vector<uint16_t> next_addresses;
  size_t next_address_index = 0;

  // all interrupt handlers
  // TODO: I assume it sets the PC to the interrupt index, so it continues running until a HALT or
  // RTI.
  for (size_t i = 0; i < 0x10; i += 2)
    next_addresses.emplace_back(i);

  // entry point
  next_addresses.emplace_back(0x10);

  while (next_address_index < next_addresses.size())
  {
    uint16_t start_pc = next_addresses[next_address_index++];

    Block* existing_block = nullptr;
    for (auto& block : blocks)
    {
      uint16_t block_size =
          std::accumulate(block.instructions.begin(), block.instructions.end(), 0,
                          [](uint16_t sum, const auto& value) { return sum + value.size; });
      if (start_pc >= block.pc && start_pc < block.pc + block_size)
      {
        existing_block = &block;
        break;
      }
    }
    if (existing_block)
    {
      if (existing_block->pc == start_pc)
        continue;

      // printf("splitting existing block at %04x\n", start_pc);

      Block block;
      block.pc = start_pc;

      auto it =
          std::find_if(existing_block->instructions.begin(), existing_block->instructions.end(),
                       [&](const auto& inst) { return inst.pc == start_pc; });

      block.instructions.insert(block.instructions.begin(), it, existing_block->instructions.end());
      existing_block->instructions.erase(it, existing_block->instructions.end());

      blocks.emplace_back(block);

      continue;
    }

    blocks.emplace_back();
    auto& block = blocks.back();
    block.pc = start_pc;

    // printf("Analyzing address: %04x\n", block.pc);

    uint16_t pc = block.pc;
    while (true)
    {
      block.instructions.emplace_back();
      auto& inst = block.instructions.back();
      inst.pc = pc;

      inst.op1 = ucode[pc];
      inst.opc = DSP::FindOpInfoByOpcode(inst.op1);
      if (!inst.opc)
      {
        // treat an unknown op as a HALT
        inst.op1 = 0x0021;
        inst.opc = FindOpInfoByOpcode(inst.op1);
      }

      bool is_extended = false;
      bool is_only_7_bit_ext = false;

      if (((inst.opc->opcode >> 12) == 0x3) && (inst.op1 & 0x007f))
      {
        is_extended = true;
        is_only_7_bit_ext = true;
      }
      else if (((inst.opc->opcode >> 12) > 0x3) && (inst.op1 & 0x00ff))
      {
        is_extended = true;
      }

      if (is_extended)
      {
        const u16 extended_opcode = is_only_7_bit_ext ? inst.op1 & 0x7F : inst.op1;
        inst.opc_ext = FindExtOpInfoByOpcode(extended_opcode);
      }

      // Size 2 - the op has a large immediate.
      inst.op2 = inst.opc->size == 2 ? ucode[pc + 1] & 0x0fff : 0;  // TODO: why mask ?

      inst.size = is_extended ? inst.opc_ext->size : inst.opc->size;
      pc += inst.size;

      // TODO: what is this number? are these guaranteed to be unique?
      auto opcode = inst.opc->opcode;

      // CALL* ops
      if (opcode >= 0x02b0 && opcode < 0x02c0)
      {
        next_addresses.emplace_back(GetOpParameterValue(inst.opc->params[0], inst.op1, inst.op2));
      }

      // J* ops
      if (opcode >= 0x0290 && opcode < 0x0300)
      {
        next_addresses.emplace_back(GetOpParameterValue(inst.opc->params[0], inst.op1, inst.op2));
      }

      if (inst.opc->branch)
      {
        if (!inst.opc->uncond_branch)
          next_addresses.emplace_back(pc);
        break;
      }
    }
    // printf("\n");
  }

  std::sort(blocks.begin(), blocks.end(),
            [](const auto& left, const auto& right) { return left.pc < right.pc; });

  DSP::AssemblerSettings settings;
  // settings.show_pc = true;
  // settings.show_hex = true;
  DSP::DSPDisassembler disassembler(settings);

  struct IntermediateInst
  {
    std::string repr;
  };

  for (const auto& b : blocks)
  {
    std::vector<IntermediateInst> intermediate_instructions;

    for (const auto& inst : b.instructions)
    {
      // skip NOPs
      if (inst.opc->opcode == 0x0000)
        continue;

      intermediate_instructions.emplace_back();
      auto& inter_inst = intermediate_instructions.back();

      // SBCLR
      if (inst.opc->opcode == 0x1200)
      {
        inter_inst.repr =
            StringFromFormat("status_disable 0x%x",
                             GetOpParameterValue(inst.opc->params[0], inst.op1, inst.op2) + 6);
        continue;
      }

      // SBSET
      if (inst.opc->opcode == 0x1300)
      {
        inter_inst.repr = StringFromFormat(
            "status_enable 0x%x", GetOpParameterValue(inst.opc->params[0], inst.op1, inst.op2) + 6);
        continue;
      }

      // SET16
      if (inst.opc->opcode == 0x8e00)
      {
        inter_inst.repr = "status_disable 0xe";
        continue;
      }

      // CLR15
      if (inst.opc->opcode == 0x8c00)
      {
        inter_inst.repr = "status_disable 0xf";
        continue;
      }

      // M0
      if (inst.opc->opcode == 0x8b00)
      {
        inter_inst.repr = "status_enable 0xd";
        continue;
      }

      uint16_t disassemble_pc = inst.pc;
      disassembler.DisassembleOpcode(ucode.data(), &disassemble_pc, inter_inst.repr);
    }

    printf("Block at %04x\n", b.pc);
    for (const auto& inst : intermediate_instructions)
    {
      printf("    %s\n", inst.repr.c_str());
    }

    printf("\n");
  }

  // std::set<uint16_t> unknown_locations;
  // std::map<uint16_t, uint16_t> interrupt_locations_and_number;
  // std::set<uint16_t> block_locations;
  // std::set<uint16_t> function_locations;
  // block_locations.emplace(0x0);
  // function_locations.emplace(0x10);
  //
  // {
  //   uint16_t i = 0;
  //   while (i < ucode.size())
  //   {
  //     const u16 op1 = ucode[i];
  //     const DSPOPCTemplate* opc = DSP::FindOpInfoByOpcode(op1);
  //
  //     if (!opc)
  //     {
  //       i += 1;
  //       continue;
  //     }
  //
  //     bool is_extended = false;
  //     bool is_only_7_bit_ext = false;
  //
  //     if (((opc->opcode >> 12) == 0x3) && (op1 & 0x007f))
  //     {
  //       is_extended = true;
  //       is_only_7_bit_ext = true;
  //     }
  //     else if (((opc->opcode >> 12) > 0x3) && (op1 & 0x00ff))
  //     {
  //       is_extended = true;
  //     }
  //
  //     const DSPOPCTemplate* opc_ext = nullptr;
  //     if (is_extended)
  //     {
  //       // opcode has an extension
  //       const u16 extended_opcode = is_only_7_bit_ext ? op1 & 0x7F : op1;
  //       opc_ext = FindExtOpInfoByOpcode(extended_opcode);
  //     }
  //
  //     // Size 2 - the op has a large immediate.
  //     // TODO: why mask ?
  //     u16 op2 = opc->size == 2 ? ucode[i + 1] & 0x0fff : 0;
  //
  //     //  std::string opname = opc->name;
  //     // if (is_extended)
  //     //   opname += StringFromFormat("%c%s", settings_.ext_separator, opc_ext->name);
  //
  //     // printf("is_extended %i opc_ext %p opc %p\n", is_extended, opc_ext, opc);
  //
  //     i += is_extended ? opc_ext->size : opc->size;
  //
  //     // TODO: what is this number? are these guaranteed to be unique?
  //     auto opcode = opc->opcode;
  //
  //     // LOOP ops
  //     if (opcode == 0x0040 || opcode == 0x0060 || opcode == 0x1000 || opcode == 0x1100)
  //     {
  //       // TODO
  //       continue;
  //     }
  //
  //     // CALL* ops
  //     if (opcode >= 0x02b0 && opcode < 0x02c0)
  //     {
  //       function_locations.emplace(GetOpParameterValue(opc->params[0], op1, op2));
  //       continue;
  //     }
  //
  //     if (!opc->branch)
  //       continue;
  //
  //     // Most unconditional branches don't tell us anything about the following data
  //     if (opc->uncond_branch)
  //     {
  //       unknown_locations.emplace(i);
  //     }
  //     else
  //     {
  //       // The next instruction is the beginning of a block
  //       block_locations.emplace(i);
  //     }
  //
  //     // Branch ops with arguments:
  //     // IF* ops
  //     if (opcode >= 0x0270 && opcode < 0x0280)
  //     {
  //       // TODO: parse next instruction to figure out how big it is...
  //     }
  //     // J* ops
  //     else if (opcode >= 0x0290 && opcode < 0x0300)
  //     {
  //       auto address = GetOpParameterValue(opc->params[0], op1, op2);
  //       if (i < 0x10)
  //         interrupt_locations_and_number.emplace(address, i);
  //       else
  //         block_locations.emplace(address);
  //     }
  //     // JR* ops
  //     else if (opcode >= 0x1700 && opcode < 0x1710)
  //     {
  //       // Impossible?
  //     }
  //     // CALLR* ops
  //     else if (opcode >= 0x1710 && opcode < 0x1720)
  //     {
  //       // Impossible?
  //     }
  //   }
  // }

  // {
  //   DSP::AssemblerSettings settings;
  //   // settings.show_pc = true;
  //   // settings.show_hex = true;
  //   DSP::DSPDisassembler disassembler(settings);
  //
  //   uint16_t i = 0x10;  // skip exception table
  //   while (i < ucode.size())
  //   {
  //     if (unknown_locations.count(i) || function_locations.count(i) || block_locations.count(i)
  //     ||
  //         interrupt_locations_and_number.count(i))
  //     {
  //       printf("\n");
  //     }
  //     if (unknown_locations.count(i) &&
  //         !(function_locations.count(i) || block_locations.count(i) ||
  //           interrupt_locations_and_number.count(i)))
  //       printf("unk_%04x:\n", i);
  //     if (function_locations.count(i))
  //       printf("func_%04x:\n", i);
  //     if (block_locations.count(i))
  //       printf("block_%04x:\n", i);
  //     if (interrupt_locations_and_number.count(i))
  //       printf("interr_%04x (%02x):\n", i, interrupt_locations_and_number[i]);
  //     std::string s;
  //     disassembler.DisassembleOpcode(ucode.data(), &i, s);
  //     printf("    %s\n", s.c_str());
  //   }
  // }

  // {
  //   // Four Swords Adventures trace #1
  //   printf("FOUR SWORDS ADVENTURES TRACE #1\n");
  //   if (!RunDSP(options["rom"], options["coefs"], options["ucode"],
  //               {{
  //                   0xc3, 0xcf,  // 0x0 challenge nonce part 1
  //                   0xa4, 0xb2,  // 0x1 challenge nonce part 2
  //                   0x00, 0x00,  // 0x2, not used?
  //                   0x00, 0x02,  // 0x3 logo palette
  //                   0x00, 0x00,  // 0x4, not used?
  //                   0x00, 0x02,  // 0x5 logo speed
  //                   0x00, 0x00,  // 0x6 joyboot length hi
  //                   0x82, 0x9c,  // 0x7 joyboot length lo
  //                   0x80, 0x4b,  // 0x8 dest addr hi
  //                   0x9a, 0xc0,  // 0x9 dest addr lo
  //               }},
  //               [] {
  //                 return CheckDmem(0x10, {{
  //                                            0x0000,  // 0x10
  //                                            0x0022,  // 0x11
  //                                            0x2014,  // 0x12
  //                                            0x0000,  // 0x13
  //                                            0x829c,  // 0x14
  //                                            0x0000,  // 0x15
  //                                            0x0000,  // 0x16
  //                                            0x80a0,  // 0x17
  //                                        }}) &&
  //                        CheckDmem(0x1e, {{
  //                                            0x6f64,  // 0x1e
  //                                            0xb2a4,  // 0x1f
  //                                            0xddc0,  // 0x20
  //                                            0xaab0,  // 0x21
  //                                            0xb7d5,  // 0x22
  //                                            0xc1df,  // 0x23
  //                                        }});
  //               }))
  //     return 1;
  // }

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
