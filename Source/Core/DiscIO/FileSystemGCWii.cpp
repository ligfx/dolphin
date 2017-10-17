// Copyright 2008 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include <algorithm>
#include <cinttypes>
#include <cstddef>
#include <cstring>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Common/CommonFuncs.h"
#include "Common/CommonTypes.h"
#include "Common/File.h"
#include "Common/Logging/Log.h"
#include "Common/MsgHandler.h"
#include "Common/StringUtil.h"
#include "DiscIO/DiscExtractor.h"
#include "DiscIO/FileSystemGCWii.h"
#include "DiscIO/Filesystem.h"
#include "DiscIO/Volume.h"

namespace DiscIO
{
constexpr u32 FST_ENTRY_SIZE = 4 * 3;  // An FST entry consists of three 32-bit integers

FileSystemGCWii::FileSystemGCWii(const Volume* volume, const Partition& partition)
    : m_valid(false), m_root(nullptr, 0)
{
  // Check if this is a GameCube or Wii disc
  if (volume->ReadSwapped<u32>(0x18, partition) == u32(0x5D1C9EA3))
    m_offset_shift = 2;  // Wii file system
  else if (volume->ReadSwapped<u32>(0x1c, partition) == u32(0xC2339F3D))
    m_offset_shift = 0;  // GameCube file system
  else
    return;  // Invalid partition (maybe someone removed its data but not its partition table entry)

  const std::optional<u64> fst_offset = GetFSTOffset(*volume, partition);
  const std::optional<u64> fst_size = GetFSTSize(*volume, partition);
  if (!fst_offset || !fst_size)
    return;
  if (*fst_size < FST_ENTRY_SIZE)
  {
    ERROR_LOG(DISCIO, "File system is too small");
    return;
  }

  // 128 MiB is more than the total amount of RAM in a Wii.
  // No file system should use anywhere near that much.
  static const u32 ARBITRARY_FILE_SYSTEM_SIZE_LIMIT = 128 * 1024 * 1024;
  if (*fst_size > ARBITRARY_FILE_SYSTEM_SIZE_LIMIT)
  {
    // Without this check, Dolphin can crash by trying to allocate too much
    // memory when loading a disc image with an incorrect FST size.

    ERROR_LOG(DISCIO, "File system is abnormally large! Aborting loading");
    return;
  }

  // Read the whole FST
  m_fst.resize(*fst_size);
  if (!volume->Read(*fst_offset, *fst_size, m_fst.data(), partition))
  {
    ERROR_LOG(DISCIO, "Couldn't read file system table");
    return;
  }

  // Create the root object
  m_root = FileInfo(this, 0);
  if (!m_root.IsDirectory())
  {
    ERROR_LOG(DISCIO, "File system root is not a directory");
    return;
  }
  m_total_file_infos = m_root.GetSize();

  if (FST_ENTRY_SIZE * m_total_file_infos > *fst_size)
  {
    ERROR_LOG(DISCIO, "File system has too many entries for its size");
    return;
  }

  // If the FST's final byte isn't 0, FileInfoGCWii::GetName() can read past the end
  if (m_fst[*fst_size - 1] != 0)
  {
    ERROR_LOG(DISCIO, "File system does not end with a null byte");
    return;
  }

  m_valid = IsValid(0, *fst_size, 0);
}

FileSystemGCWii::~FileSystemGCWii() = default;

const FileInfo& FileSystemGCWii::GetRoot() const
{
  return m_root;
}

std::unique_ptr<FileInfo> FileSystemGCWii::FindFileInfo(const std::string& path) const
{
  if (!IsValid())
    return nullptr;

  return FindFileInfo(path, m_root);
}

std::unique_ptr<FileInfo> FileSystemGCWii::FindFileInfo(const std::string& path,
                                                        const FileInfo& file_info) const
{
  // Given a path like "directory1/directory2/fileA.bin", this function will
  // find directory1 and then call itself to search for "directory2/fileA.bin".

  const size_t name_start = path.find_first_not_of('/');
  if (name_start == std::string::npos)
    return std::make_unique<FileInfo>(file_info);  // We're done

  const size_t name_end = path.find('/', name_start);
  const std::string name = path.substr(name_start, name_end - name_start);
  const std::string rest_of_path = (name_end != std::string::npos) ? path.substr(name_end + 1) : "";

  for (const FileInfo& child : file_info)
  {
    if (!strcasecmp(child.GetName().c_str(), name.c_str()))
    {
      // A match is found. The rest of the path is passed on to finish the search.
      std::unique_ptr<FileInfo> result = FindFileInfo(rest_of_path, child);

      // If the search wasn't successful, the loop continues, just in case there's a second
      // file info that matches searching_for (which probably won't happen in practice)
      if (result)
        return result;
    }
  }

  return nullptr;
}

std::unique_ptr<FileInfo> FileSystemGCWii::FindFileInfo(u64 disc_offset) const
{
  if (!IsValid())
    return nullptr;

  // Build a cache (unless there already is one)
  if (m_offset_file_info_cache.empty())
  {
    u32 fst_entries = m_root.GetSize();
    for (u32 i = 0; i < fst_entries; i++)
    {
      FileInfo file_info(m_root, i);
      if (!file_info.IsDirectory())
      {
        const u32 size = file_info.GetSize();
        if (size != 0)
          m_offset_file_info_cache.emplace(file_info.GetOffset() + size, i);
      }
    }
  }

  // Get the first file that ends after disc_offset
  const auto it = m_offset_file_info_cache.upper_bound(disc_offset);
  if (it == m_offset_file_info_cache.end())
    return nullptr;
  std::unique_ptr<FileInfo> result(std::make_unique<FileInfo>(m_root, it->second));

  // If the file's start isn't after disc_offset, success
  if (result->GetOffset() <= disc_offset)
    return result;

  return nullptr;
}

u32 FileSystemGCWii::Get(u32 index, EntryProperty entry_property) const
{
  return Common::swap32(m_fst.data() + FST_ENTRY_SIZE * index +
                        sizeof(u32) * static_cast<int>(entry_property));
}

u64 FileSystemGCWii::GetNameOffset(u32 index) const
{
  return static_cast<u64>(FST_ENTRY_SIZE) * m_total_file_infos +
         (Get(index, EntryProperty::NAME_OFFSET) & 0xFFFFFF);
}

u32 FileSystemGCWii::GetFirstChildIndex(u32 index) const
{
  return index + 1;
}

u32 FileSystemGCWii::GetNextIndex(u32 index) const
{
  return IsDirectory(index) ? GetSize(index) : index + 1;
}

bool FileSystemGCWii::IsDirectory(u32 index) const
{
  return (Get(index, EntryProperty::NAME_OFFSET) & 0xFF000000) != 0;
}

u32 FileSystemGCWii::GetSize(u32 index) const
{
  return Get(index, EntryProperty::FILE_SIZE);
}

u64 FileSystemGCWii::GetOffset(u32 index) const
{
  return static_cast<u64>(Get(index, EntryProperty::FILE_OFFSET)) << m_offset_shift;
}

u32 FileSystemGCWii::GetTotalChildren(u32 index) const
{
  return Get(index, EntryProperty::FILE_SIZE) - (index + 1);
}

std::string FileSystemGCWii::GetName(u32 index) const
{
  // TODO: Should we really always use SHIFT-JIS?
  // Some names in Pikmin (NTSC-U) don't make sense without it, but is it correct?
  return SHIFTJISToUTF8(reinterpret_cast<const char*>(m_fst.data() + GetNameOffset(index)));
}

std::string FileSystemGCWii::GetPath(u32 index) const
{
  // The root entry doesn't have a name
  if (index == 0)
    return "";

  if (IsDirectory(index))
  {
    u32 parent_directory_index = Get(index, EntryProperty::FILE_OFFSET);
    return GetPath(parent_directory_index) + GetName(index) + "/";
  }
  else
  {
    // The parent directory can be found by searching backwards
    // for a directory that contains this file. The search cannot fail,
    // because the root directory at index 0 contains all files.
    u32 potential_parent_index = index - 1;
    while (!(IsDirectory(potential_parent_index) &&
             Get(potential_parent_index, EntryProperty::FILE_SIZE) > index))
    {
      --potential_parent_index;
    }
    return GetPath(potential_parent_index) + GetName(index);
  }
}

bool FileSystemGCWii::IsValid(u32 index, u64 fst_size, u32 parent_index) const
{
  if (GetNameOffset(index) >= fst_size)
  {
    ERROR_LOG(DISCIO, "Impossibly large name offset in file system");
    return false;
  }

  if (IsDirectory(index))
  {
    if (Get(index, EntryProperty::FILE_OFFSET) != parent_index)
    {
      ERROR_LOG(DISCIO, "Incorrect parent offset in file system");
      return false;
    }

    u32 size = Get(index, EntryProperty::FILE_SIZE);

    if (size <= index)
    {
      ERROR_LOG(DISCIO, "Impossibly small directory size in file system");
      return false;
    }

    if (size > Get(parent_index, EntryProperty::FILE_SIZE))
    {
      ERROR_LOG(DISCIO, "Impossibly large directory size in file system");
      return false;
    }

    for (const FileInfo& child : FileInfo(this, index))
    {
      if (!IsValid(child.AsU32(), fst_size, index))
        return false;
    }
  }

  return true;
}

}  // namespace
