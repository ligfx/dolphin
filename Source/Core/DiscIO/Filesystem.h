// Copyright 2008 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Common/CommonTypes.h"
#include "DiscIO/Volume.h"

namespace DiscIO
{
class FileInfoIterator;
class FileSystem;

// file info of an FST entry
class FileInfo final
{
  friend class FileInfoIterator;

public:
  FileInfo();
  FileInfo(const FileSystem* filesystem, u32 index);
  FileInfo(const FileInfo& other);
  FileInfo(const FileInfo& other, u32 index);

  bool operator==(const FileInfo& other) const;
  bool operator!=(const FileInfo& other) const;
  FileInfoIterator cbegin() const;
  FileInfoIterator cend() const;
  FileInfoIterator begin() const;
  FileInfoIterator end() const;

  // The offset of a file on the disc (inside the partition, if there is one).
  // Not guaranteed to return a meaningful value for directories.
  u64 GetOffset() const;
  // The size of a file.
  // Not guaranteed to return a meaningful value for directories.
  u32 GetSize() const;
  bool IsDirectory() const;
  // The number of files and directories in a directory, including those in subdirectories.
  // Not guaranteed to return a meaningful value for files.
  u32 GetTotalChildren() const;
  std::string GetName() const;
  // GetPath will find the parents of the current object and call GetName on them,
  // so it's slower than other functions. If you're traversing through folders
  // to get a file and its path, building the path while traversing is faster.
  std::string GetPath() const;

  u32 AsU32() const;

protected:
  const FileSystem* m_filesystem;
  u32 m_index;
};

class FileSystem
{
public:
  virtual ~FileSystem();

  // If IsValid is false, GetRoot must not be called.
  virtual bool IsValid() const = 0;
  // The object returned by GetRoot and all objects created from it
  // are only valid for as long as the file system object is valid.
  virtual const FileInfo& GetRoot() const = 0;
  // Returns nullptr if not found
  virtual std::unique_ptr<FileInfo> FindFileInfo(const std::string& path) const = 0;
  // Returns nullptr if not found
  virtual std::unique_ptr<FileInfo> FindFileInfo(u64 disc_offset) const = 0;

  virtual u64 GetOffset(u32 index) const = 0;
  virtual u32 GetSize(u32 index) const = 0;
  virtual bool IsDirectory(u32 index) const = 0;
  virtual u32 GetTotalChildren(u32 index) const = 0;
  virtual std::string GetName(u32 index) const = 0;
  virtual std::string GetPath(u32 index) const = 0;

  virtual u32 GetFirstChildIndex(u32 index) const = 0;
  // For files, returns the index of the next item. For directories,
  // returns the index of the next item that isn't inside it.
  virtual u32 GetNextIndex(u32 index) const = 0;
};

// Calling Volume::GetFileSystem instead of manually constructing a filesystem is recommended,
// because it will check IsValid for you, will automatically pick the right type of filesystem,
// and will cache the filesystem in case it's needed again later.

class FileInfoIterator final
{
public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = const FileInfo;
  using difference_type = std::ptrdiff_t;
  using pointer = value_type*;
  using reference = value_type&;

  FileInfoIterator();
  FileInfoIterator(FileInfo file_info);
  FileInfoIterator& operator++();
  FileInfoIterator operator++(int);
  bool operator==(const FileInfoIterator& it) const;
  bool operator!=(const FileInfoIterator& it) const;
  // Incrementing or destroying an iterator will invalidate its returned references and
  // pointers, but will not invalidate copies of the iterator or file info object.
  const FileInfo& operator*() const;
  const FileInfo* operator->() const;

private:
  FileInfo m_file_info;
};

}  // namespace
