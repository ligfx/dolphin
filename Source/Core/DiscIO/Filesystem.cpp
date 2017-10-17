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

FileInfoIterator FileInfo::cbegin() const
{
  return begin();
}
FileInfoIterator FileInfo::cend() const
{
  return end();
}

FileInfoIterator FileInfo::begin() const
{
  return FileInfoIterator(FileInfo(m_filesystem, m_filesystem->GetFirstChildIndex(m_index)));
}

FileInfoIterator FileInfo::end() const
{
  return FileInfoIterator(FileInfo(m_filesystem, m_filesystem->GetNextIndex(m_index)));
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

FileSystem::~FileSystem() = default;

FileInfoIterator::FileInfoIterator() : m_file_info()
{
}
FileInfoIterator::FileInfoIterator(FileInfo file_info) : m_file_info(std::move(file_info))
{
}
FileInfoIterator& FileInfoIterator::operator++()
{
  m_file_info.m_index = m_file_info.m_filesystem->GetNextIndex(m_file_info.m_index);
  return *this;
}
FileInfoIterator FileInfoIterator::operator++(int)
{
  FileInfoIterator old = *this;
  ++*this;
  return old;
}
bool FileInfoIterator::operator==(const FileInfoIterator& it) const
{
  return m_file_info == it.m_file_info;
}
bool FileInfoIterator::operator!=(const FileInfoIterator& it) const
{
  return !operator==(it);
}
// Incrementing or destroying an iterator will invalidate its returned references and
// pointers, but will not invalidate copies of the iterator or file info object.
const FileInfo& FileInfoIterator::operator*() const
{
  return m_file_info;
}
const FileInfo* FileInfoIterator::operator->() const
{
  return &m_file_info;
}

}  // namespace
