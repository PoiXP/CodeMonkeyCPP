#include "Precompile.h"
#include "Utils/FileDictionary.h"

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
  return MakeHandleRecursive(filename, filename.begin(), 0u, NO_INDEX);
}

namespace
{
  char ConvertToLower(char ch)
  {
    if ('A' <= ch && ch <= 'Z') return 'a' + (ch - 'A');
    return ch;
  }

  bool Match(std::string::const_iterator begin1, std::string::const_iterator end1,
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
  bool MatchNoCase(std::string::const_iterator begin1, std::string::const_iterator end1,
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
  std::string::const_iterator FindDirectoryPos(std::string::const_iterator begin, std::string::const_iterator end)
  {
    auto it = begin;
    for (; it != end; ++it)
    {
      if( *it == '\\' || *it == '/' ) break;
    }
    return it;
  }
}


FileDictionary::Handle FileDictionary::MakeHandleRecursive(const std::string& path, std::string::const_iterator start, unsigned int nodeId, unsigned int fromEntryId)
{
  std::string::const_iterator pos = FindDirectoryPos(start, path.end());
  bool leaf = (pos == path.end());
  unsigned int hash = (m_CompareType == e_Compare_NoCase) ? GetHashNoCase(start, pos) : GetHash(start, pos);
  unsigned int hashId = GetNodeFirstId(nodeId) + hash;
  FileDictionary::Handle foundEntryId = m_HashMap[hashId];
  FileDictionary::Handle lastFoundEntryId = foundEntryId;
  while( foundEntryId != NO_INDEX )
  {
    bool match = false;
    switch(m_CompareType)
    {
    case e_Compare_CaseSensitive:
      match = Match(m_Dictionary[foundEntryId].begin(), m_Dictionary[foundEntryId].end(), start, pos );
      break;
    case e_Compare_NoCase:
      match = MatchNoCase(m_Dictionary[foundEntryId].begin(), m_Dictionary[foundEntryId].end(), start, pos );
      break;
    }
    if (match) break;
    lastFoundEntryId = foundEntryId;
    foundEntryId = m_Entries[foundEntryId].nextEntry;
  }

  if (foundEntryId == NO_INDEX)
  {
    foundEntryId = m_Entries.size();

    m_Entries.push_back(Entry());
    Entry& entry = m_Entries[foundEntryId];
    entry.nextEntry = NO_INDEX;
    entry.nodeId = AddNode();
    entry.parentEntry = fromEntryId;
    std::string s(start, pos);
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
  m_HashMap.insert( m_HashMap.end(), HASH_SIZE, NO_INDEX);
  return m_IdCounter++;
}

const std::string& FileDictionary::GetFileName(FileDictionary::Handle handle, std::string& fileName) const
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

unsigned int FileDictionary::GetHash(std::string::const_iterator from, std::string::const_iterator to)
{
  unsigned int hash = 0u;
  for (auto it = from; it != to; ++it)
  {
    hash <<= 8;
    hash += static_cast<unsigned int>(*it);
    hash %= HASH_SIZE;
  }
  return hash;
}

unsigned int FileDictionary::GetHashNoCase(std::string::const_iterator from, std::string::const_iterator to)
{
  unsigned int hash = 0u;
  for (auto it = from; it != to; ++it)
  {
    hash <<= 8;
    hash += static_cast<unsigned int>(ConvertToLower(*it));
    hash %= HASH_SIZE;
  }
  return hash;
}

