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
class FileSystemGCWii;
class Volume;
struct Partition;

class FileInfoGCWii : public FileInfo
{
  friend class FileSystemGCWii;

public:
  // None of the constructors take ownership of FST pointers

  // Set everything manually
  FileInfoGCWii(const FileSystemGCWii* filesystem, u32 index);
  // Copy another object
  FileInfoGCWii(const FileInfoGCWii& file_info) = default;
  // Copy data that is common to the whole file system
  FileInfoGCWii(const FileInfoGCWii& file_info, u32 index);

  ~FileInfoGCWii() override;

  std::unique_ptr<FileInfo> clone() const override;
  const_iterator begin() const override;
  const_iterator end() const override;

  u64 GetOffset() const override;
  u32 GetSize() const override;
  bool IsDirectory() const override;
  u32 GetTotalChildren() const override;
  std::string GetName() const override;
  std::string GetPath() const override;

protected:
  uintptr_t GetAddress() const override;
  FileInfo& operator++() override;

private:
  const FileSystemGCWii* m_filesystem;
  u32 m_index;
};

class FileSystemGCWii : public FileSystem
{
  friend class FileInfoGCWii;

public:
  FileSystemGCWii(const Volume* volume, const Partition& partition);
  ~FileSystemGCWii() override;

  bool IsValid() const override { return m_valid; }
  const FileInfo& GetRoot() const override;
  std::unique_ptr<FileInfo> FindFileInfo(const std::string& path) const override;
  std::unique_ptr<FileInfo> FindFileInfo(u64 disc_offset) const override;

  u64 GetOffset(u32 index) const;
  u32 GetSize(u32 index) const;
  bool IsDirectory(u32 index) const;
  u32 GetTotalChildren(u32 index) const;
  std::string GetName(u32 index) const;
  std::string GetPath(u32 index) const;

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

  std::unique_ptr<FileInfo> FindFileInfo(const std::string& path, const FileInfo& file_info) const;

  // Returns one of the three properties of a FST entry.
  // Read the comments in EntryProperty for details.
  u32 Get(u32 index, EntryProperty entry_property) const;
  // Returns the name offset, excluding the directory identification byte
  u64 GetNameOffset(u32 index) const;
  // For files, returns the index of the next item. For directories,
  // returns the index of the next item that isn't inside it.
  u32 GetNextIndex(u32 index) const;

  uintptr_t GetAddress(u32 index) const;
  bool IsValid(u32 index, u64 fst_size, u32 parent_index) const;

  bool m_valid;
  std::vector<u8> m_fst;
  FileInfoGCWii m_root;
  u8 m_offset_shift;
  u32 m_total_file_infos;
  // Maps the end offset of files to FST indexes
  mutable std::map<u64, u32> m_offset_file_info_cache;
};

}  // namespace
