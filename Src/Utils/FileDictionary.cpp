#include "Precompile.h"
#include "Utils/FileDictionary.h"

namespace FileDictionaryInternal
{
  static const unsigned int HASH_SIZE = 61;

  char          ConvertToLower(char ch);
  unsigned int  GetHash(std::string::const_iterator from, 
                        std::string::const_iterator to);
  unsigned int  GetHashNoCase(std::string::const_iterator from, 
                              std::string::const_iterator to);
  bool          Match(std::string::const_iterator begin1, 
                      std::string::const_iterator end1,
                      std::string::const_iterator begin2, 
                      std::string::const_iterator end2);
  bool          MatchNoCase(std::string::const_iterator begin1, 
                            std::string::const_iterator end1,
                            std::string::const_iterator begin2, 
                            std::string::const_iterator end2);
  std::string::const_iterator FindDirectoryPos(std::string::const_iterator begin, 
                                               std::string::const_iterator end);
  unsigned int GetNodeFirstId(unsigned int nodeId);
}

char FileDictionaryInternal::ConvertToLower(char ch)
{
  if ('A' <= ch && ch <= 'Z') return 'a' + (ch - 'A');
  return ch;
}

unsigned int FileDictionaryInternal::GetHash(std::string::const_iterator from, std::string::const_iterator to)
{
  unsigned int hash = 0u;
  for (auto it = from; it != to; ++it)
  {
    hash <<= 8;
    hash += static_cast<unsigned char>(*it);
    hash %= HASH_SIZE;
  }
  return hash;
}

unsigned int FileDictionaryInternal::GetHashNoCase(std::string::const_iterator from, std::string::const_iterator to)
{
  unsigned int hash = 0u;
  for (auto it = from; it != to; ++it)
  {
    hash <<= 8;
    hash += static_cast<unsigned char>(ConvertToLower(*it));
    hash %= FileDictionaryInternal::HASH_SIZE;
  }
  return hash;
}

bool FileDictionaryInternal::Match(std::string::const_iterator begin1, std::string::const_iterator end1,
  std::string::const_iterator begin2, std::string::const_iterator end2)
{
  auto it1 = begin1;
  auto it2 = begin2;
  for (it1 = begin1, it2 = begin2; it1 != end1 && it2 != end2; ++it1, ++it2)
  {
    if (*it1 != *it2) return false;
  }
  return it1 == end1 && it2 == end2;
}
bool FileDictionaryInternal::MatchNoCase(std::string::const_iterator begin1, std::string::const_iterator end1,
  std::string::const_iterator begin2, std::string::const_iterator end2)
{
  auto it1 = begin1;
  auto it2 = begin2;
  for (it1 = begin1, it2 = begin2; it1 != end1 && it2 != end2; ++it1, ++it2)
  {
    if (ConvertToLower(*it1) != ConvertToLower(*it2)) return false;
  }
  return it1 == end1 && it2 == end2;
}
std::string::const_iterator FileDictionaryInternal::FindDirectoryPos(std::string::const_iterator begin, std::string::const_iterator end)
{
  auto it = begin;
  for (; it != end; ++it)
  {
    if( *it == '\\' || *it == '/' ) break;
  }
  return it;
}
unsigned int FileDictionaryInternal::GetNodeFirstId(unsigned int nodeId)
{ 
  return nodeId * HASH_SIZE;
}

// ----------------------------IMPLEMENTATION----------------------------------

FileDictionary::Iterator::Iterator(const FileDictionary& dict, size_t handle)
  : m_Dict(dict)
  , m_Handle(handle)
{

}

FileDictionary::Iterator::~Iterator()
{
}

size_t FileDictionary::Iterator::operator*() const
{
  return m_Handle;
}

FileDictionary::Iterator& FileDictionary::Iterator::operator++()
{
  m_Handle = m_Dict.GetNextLeafHandle(m_Handle, false);
  return *this;
}

FileDictionary::Iterator FileDictionary::Iterator::operator++(int)
{
  Iterator temp(*this);
  operator++();
  return temp;
}

bool FileDictionary::Iterator::operator!=(const FileDictionary::Iterator& iter) const
{
  return !(*this == iter);
}

bool FileDictionary::Iterator::operator==(const FileDictionary::Iterator& iter) const
{
  return m_Handle == iter.m_Handle;
}


FileDictionary::FileDictionary(FileDictionary::CompareType compareType, unsigned int dictionarySize)
  : m_CompareType(compareType)
  , m_IdCounter(0)
{
  m_Dictionary.reserve(dictionarySize);
  m_Entries.reserve(dictionarySize);
  AddNode();
}

FileDictionary::~FileDictionary()
{

}

size_t FileDictionary::MakeHandle(const std::string& filename)
{
  return MakeHandleRecursive(filename, filename.begin(), 0u, NO_INDEX);
}

size_t FileDictionary::FindHandle(const std::string& filename) const
{
  return FindHandleRecursive(filename, filename.begin(), 0u, NO_INDEX);
}

size_t FileDictionary::FindHandleInternal(const std::string& path, std::string::const_iterator& start, unsigned int nodeId, unsigned int fromEntryId, bool& isLeaf, size_t& lastFoundEntryId, unsigned int& hashId) const
{
  std::string::const_iterator pos = FileDictionaryInternal::FindDirectoryPos(start, path.end());
  isLeaf = (pos == path.end());
  unsigned int hash = (m_CompareType == e_Compare_NoCase) ? 
                       FileDictionaryInternal::GetHashNoCase(start, pos) : 
                       FileDictionaryInternal::GetHash(start, pos);
  hashId = FileDictionaryInternal::GetNodeFirstId(nodeId) + hash;
  size_t foundEntryId = m_HashMap[hashId];
  lastFoundEntryId = NO_INDEX;
  while( foundEntryId != NO_INDEX )
  {
    bool match = false;
    switch(m_CompareType)
    {
    case e_Compare_CaseSensitive:
      match = FileDictionaryInternal::Match(m_Dictionary[foundEntryId].begin(), m_Dictionary[foundEntryId].end(), start, pos );
      break;
    case e_Compare_NoCase:
      match = FileDictionaryInternal::MatchNoCase(m_Dictionary[foundEntryId].begin(), m_Dictionary[foundEntryId].end(), start, pos );
      break;
    }
    if (match) break;
    lastFoundEntryId = foundEntryId;
    foundEntryId = m_Entries[foundEntryId].nextEntry;
  }
  start = pos;
  return foundEntryId;
}

size_t FileDictionary::FindHandleRecursive(const std::string& path, std::string::const_iterator& start, unsigned int nodeId, unsigned int fromEntryId) const
{
  bool isLeaf;
  size_t lastFoundEntryId;
  unsigned int hashId;
  size_t foundEntryId = FindHandleInternal(path, start, nodeId, fromEntryId, isLeaf, lastFoundEntryId, hashId);

  if (foundEntryId != NO_INDEX)
  {
    return isLeaf ? foundEntryId : FindHandleRecursive(path, start + 1, m_Entries[foundEntryId].nodeId, foundEntryId);
  }
  else
  {
    return NO_INDEX;
  }
}

size_t FileDictionary::MakeHandleRecursive(const std::string& path, std::string::const_iterator start, unsigned int nodeId, unsigned int fromEntryId)
{
  bool isLeaf;
  size_t lastFoundEntryId;
  unsigned int hashId;
  std::string::const_iterator nextPos = start;
  size_t foundEntryId = FindHandleInternal(path, nextPos, nodeId, fromEntryId, isLeaf, lastFoundEntryId, hashId);
  if (foundEntryId == NO_INDEX)
  {
    foundEntryId = m_Entries.size();

    m_Entries.push_back(Entry());
    Entry& entry = m_Entries[foundEntryId];
    entry.nextEntry = NO_INDEX;
    entry.nodeId = AddNode();
    entry.parentEntry = fromEntryId;
    entry.isFileNode = isLeaf;
    std::string s(start, nextPos);
    m_Dictionary.push_back(s);

    if (lastFoundEntryId == NO_INDEX)
    {
      m_HashMap[hashId] = foundEntryId;
    }
    else
    {
      m_Entries[lastFoundEntryId].nextEntry = foundEntryId;
    }
  }
  else if(isLeaf)
  {
    m_Entries[foundEntryId].isFileNode = true;
  }

  if (!isLeaf)
  {
    return MakeHandleRecursive(path, nextPos + 1, m_Entries[foundEntryId].nodeId, foundEntryId);
  }
  else
  {
    return foundEntryId;
  }
}

  

unsigned int FileDictionary::AddNode()
{
  m_HashMap.insert( m_HashMap.end(), FileDictionaryInternal::HASH_SIZE, NO_INDEX);
  return m_IdCounter++;
}

const std::string& FileDictionary::GetFileName(size_t handle, std::string& fileName) const
{
  if (m_Entries[handle].parentEntry != NO_INDEX)
  {
    GetFileName(m_Entries[handle].parentEntry, fileName);
    fileName += '\\' + m_Dictionary[handle];
  }
  else
  {
    fileName += m_Dictionary[handle];
  }
  return fileName;
}

size_t FileDictionary::GetNextLeafHandle(size_t handle, bool findFirst) const
{
  if (!findFirst)
  {
    handle += 1;
  }
  size_t entryCount = m_Entries.size();
  for (size_t i = handle; i < entryCount; ++i)
  {
    if (m_Entries[i].isFileNode)
    {
      return i;
    }
  }
  return entryCount;
}


FileDictionary::Iterator FileDictionary::Begin() const
{
  return Iterator(*this, GetNextLeafHandle(0, true));
}

FileDictionary::Iterator FileDictionary::End() const
{
  return Iterator(*this, m_Entries.size());
}


