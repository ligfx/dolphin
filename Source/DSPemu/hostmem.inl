#include "Common/FileUtil.h"

#include <vector>

static std::vector<u8> s_cpu_ram(16384);

static bool LoadFileToRam(u32 address, const std::string& filename)
{
  std::string bytes;
  if (!File::ReadFileToString(filename, bytes))
  {
    printf("Couldn't open '%s'", filename.c_str());
    return false;
  }
  memcpy(s_cpu_ram.data() + address, bytes.data(), bytes.size());
  return true;
}

static void CopyToRam(u32 address, const std::vector<u8>& data)
{
  std::copy(data.begin(), data.end(), s_cpu_ram.begin() + address);
}
