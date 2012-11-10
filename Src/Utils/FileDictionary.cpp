#include "Precompile.h"
#include "Utils/FileDictionary.h"

namespace FileDictionaryInternal
{
  static const unsigned int HASH_SIZE = 61;
  static const unsigned int NO_INDEX  = 0xFFFFFFFF;

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

FileDictionary::Iterator::Iterator(const FileDictionary& dict, Handle handle)
  : m_Dict(dict)
  , m_Handle(handle)
{

}

FileDictionary::Iterator::~Iterator()
{
}

FileDictionary::Handle FileDictionary::Iterator::operator*() const
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

FileDictionary::Handle FileDictionary::MakeHandle(const std::string& filename)
{
  return MakeHandleRecursive(filename, filename.begin(), 0u, FileDictionaryInternal::NO_INDEX);
}


FileDictionary::Handle FileDictionary::MakeHandleRecursive(const std::string& path, std::string::const_iterator start, unsigned int nodeId, unsigned int fromEntryId)
{
  std::string::const_iterator pos = FileDictionaryInternal::FindDirectoryPos(start, path.end());
  bool leaf = (pos == path.end());
  unsigned int hash = (m_CompareType == e_Compare_NoCase) ? 
                       FileDictionaryInternal::GetHashNoCase(start, pos) : 
                       FileDictionaryInternal::GetHash(start, pos);
  unsigned int hashId = FileDictionaryInternal::GetNodeFirstId(nodeId) + hash;
  FileDictionary::Handle foundEntryId = m_HashMap[hashId];
  FileDictionary::Handle lastFoundEntryId = foundEntryId;
  while( foundEntryId != FileDictionaryInternal::NO_INDEX )
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

  if (foundEntryId == FileDictionaryInternal::NO_INDEX)
  {
    foundEntryId = m_Entries.size();

    m_Entries.push_back(Entry());
    Entry& entry = m_Entries[foundEntryId];
    entry.nextEntry = FileDictionaryInternal::NO_INDEX;
    entry.nodeId = AddNode();
    entry.parentEntry = fromEntryId;
    entry.isFileNode = leaf;
    std::string s(start, pos);
    m_Dictionary.push_back(s);

    if (lastFoundEntryId == FileDictionaryInternal::NO_INDEX)
    {
      m_HashMap[hashId] = foundEntryId;
    }
    else
    {
      m_Entries[lastFoundEntryId].nextEntry = foundEntryId;
    }
  }
  else if(leaf)
  {
    m_Entries[foundEntryId].isFileNode = true;
  }

  if (!leaf)
  {
    return MakeHandleRecursive(path, pos + 1, m_Entries[foundEntryId].nodeId, foundEntryId);
  }
  else
  {
    return foundEntryId;
  }
}
  

unsigned int FileDictionary::AddNode()
{
  m_HashMap.insert( m_HashMap.end(), FileDictionaryInternal::HASH_SIZE, FileDictionaryInternal::NO_INDEX);
  return m_IdCounter++;
}

const std::string& FileDictionary::GetFileName(FileDictionary::Handle handle, std::string& fileName) const
{
  if (m_Entries[handle].parentEntry != FileDictionaryInternal::NO_INDEX)
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

FileDictionary::Handle FileDictionary::GetNextLeafHandle(FileDictionary::Handle handle, bool findFirst) const
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


