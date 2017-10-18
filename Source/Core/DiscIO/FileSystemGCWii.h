// Copyright 2008 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <cstddef>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Common/CommonTypes.h"
#include "DiscIO/Filesystem.h"

namespace DiscIO
{
class Volume;
struct Partition;

class FileSystemGCWii : public FileSystem
{
public:
  FileSystemGCWii(const Volume* volume, const Partition& partition);
  ~FileSystemGCWii() override;

  bool IsValid() const override { return m_valid; }
  FileInfo GetRoot() const override;
  std::optional<FileInfo> FindFileInfo(const std::string& path) const override;
  std::optional<FileInfo> FindFileInfo(u64 disc_offset) const override;

  u64 GetOffset(u32 index) const override;
  u32 GetSize(u32 index) const override;
  bool IsDirectory(u32 index) const override;
  u32 GetTotalChildren(u32 index) const override;
  std::string GetName(u32 index) const override;
  std::string GetPath(u32 index) const override;

  u32 GetFirstChildIndex(u32 index) const override;
  // For files, returns the index of the next item. For directories,
  // returns the index of the next item that isn't inside it.
  u32 GetNextIndex(u32 index) const override;

private:
  enum class EntryProperty
  {
    // NAME_OFFSET's lower 3 bytes are the name's offset within the name table.
    // NAME_OFFSET's upper 1 byte is 1 for directories and 0 for files.
    NAME_OFFSET = 0,
    // For files, FILE_OFFSET is the file offset in the partition,
    // and for directories, it's the FST index of the parent directory.
    // The root directory has its parent directory index set to 0.
    FILE_OFFSET = 1,
    // For files, FILE_SIZE is the file size, and for directories,
    // it's the FST index of the next entry that isn't in the directory.
    FILE_SIZE = 2
  };

  std::optional<FileInfo> FindFileInfo(const std::string& path, const FileInfo& file_info) const;

  // Returns one of the three properties of a FST entry.
  // Read the comments in EntryProperty for details.
  u32 Get(u32 index, EntryProperty entry_property) const;
  // Returns the name offset, excluding the directory identification byte
  u64 GetNameOffset(u32 index) const;

  bool IsValid(u32 index, u64 fst_size, u32 parent_index) const;

  bool m_valid;
  std::vector<u8> m_fst;
  u8 m_offset_shift;
  u32 m_total_file_infos;
  // Maps the end offset of files to FST indexes
  mutable std::map<u64, u32> m_offset_file_info_cache;
};

}  // namespace
