#pragma once

#include <string>
#include <vector>

class FileDictionary
{
public:
  enum CompareType
  {
    e_Compare_CaseSensitive,
    e_Compare_NoCase
  };

  typedef unsigned int Handle;

  FileDictionary(CompareType compareType, unsigned int dictionarySize = 0u);
  ~FileDictionary();

  Handle MakeHandle(const std::string& filename);
  const std::string& GetFileName(Handle handle, std::string& filename) const;
private:
  static const unsigned int HASH_SIZE = 61;
  static const unsigned int NO_INDEX  = 0xFFFFFFFF; 
  Handle MakeHandleRecursive(const std::string& path, std::string::const_iterator start, unsigned int nodeId, unsigned int fromEntryId);
  unsigned int GetNodeFirstId(unsigned int nodeId) { return nodeId * HASH_SIZE; }
  unsigned int AddNode();
  unsigned int GetHash(std::string::const_iterator from, std::string::const_iterator to);
  unsigned int GetHashNoCase(std::string::const_iterator from, std::string::const_iterator to);
  CompareType m_CompareType;
  unsigned int m_IdCounter;
  std::vector<unsigned int> m_HashMap;
  struct Entry
  {
    unsigned int parentEntry;
    unsigned int nextEntry;
    unsigned int nodeId;
  };
  std::vector<Entry>        m_Entries;
  std::vector<std::string>  m_Dictionary;
};
