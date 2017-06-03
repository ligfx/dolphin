#include "Core/DSP/DSPHost.h"

// Stub out the dsplib host stuff, since this is just a simple cmdline tool.
u8 DSP::Host::ReadHostMemory(u32 addr)
{
  return 0;
}
void DSP::Host::WriteHostMemory(u8 value, u32 addr)
{
}
void DSP::Host::OSD_AddMessage(const std::string& str, u32 ms)
{
  printf("OSD: %s\n", str.c_str());
}
bool DSP::Host::OnThread()
{
  return false;
}
bool DSP::Host::IsWiiHost()
{
  return false;
}
void DSP::Host::CodeLoaded(const u8* ptr, int size)
{
}
void DSP::Host::InterruptRequest()
{
  printf("DSP INTERRUPT REQUEST\n");
}
void DSP::Host::UpdateDebugger()
{
}
