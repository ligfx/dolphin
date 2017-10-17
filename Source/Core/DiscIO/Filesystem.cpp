// Copyright 2008 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "DiscIO/Filesystem.h"

namespace DiscIO
{
FileInfo::FileInfo() : m_filesystem(nullptr), m_index(){};
FileInfo::FileInfo(const FileSystem* filesystem, u32 index)
    : m_filesystem(filesystem), m_index(index){};
FileInfo::FileInfo(const FileInfo& other) = default;
FileInfo::FileInfo(const FileInfo& other, u32 index) : FileInfo(other.m_filesystem, index){};

bool FileInfo::operator==(const FileInfo& other) const
{
  return m_index == other.m_index;
}
bool FileInfo::operator!=(const FileInfo& other) const
{
  return !(*this == other);
}

u32 FileInfo::GetSize() const
{
  return m_filesystem->GetSize(m_index);
}

u64 FileInfo::GetOffset() const
{
  return m_filesystem->GetOffset(m_index);
}

bool FileInfo::IsDirectory() const
{
  return m_filesystem->IsDirectory(m_index);
}

u32 FileInfo::GetTotalChildren() const
{
  return m_filesystem->GetTotalChildren(m_index);
}

std::string FileInfo::GetName() const
{
  return m_filesystem->GetName(m_index);
}

std::string FileInfo::GetPath() const
{
  return m_filesystem->GetPath(m_index);
}

FileInfo::~FileInfo() = default;

FileSystem::~FileSystem() = default;

}  // namespace
