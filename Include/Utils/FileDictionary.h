#pragma once

#include <string>
#include <vector>

class FileDictionary
{
public:
  typedef unsigned int Handle;
  
  class Iterator
  {
  public:
    ~Iterator();
    Handle     operator*() const;
    Iterator&  operator++();
    Iterator   operator++(int);
    bool       operator!=(const Iterator& iter) const;
    bool       operator==(const Iterator& iter) const;
  private:
    Iterator(const FileDictionary& dict, Handle handle);
    const FileDictionary&   m_Dict;
    FileDictionary::Handle  m_Handle;
    friend class FileDictionary;
  };

  enum CompareType
  {
    e_Compare_CaseSensitive,
    e_Compare_NoCase
  };

  FileDictionary(CompareType compareType, unsigned int dictionarySize = 0u);
  ~FileDictionary();
  
  Handle MakeHandle(const std::string& filename);
  const std::string& GetFileName(Handle handle, std::string& filename) const;

  Iterator Begin() const;
  Iterator End() const;
private:

  Handle       GetNextLeafHandle(Handle handle, bool findFirst) const;
  Handle       MakeHandleRecursive(const std::string& path, std::string::const_iterator start, unsigned int nodeId, unsigned int fromEntryId);
  unsigned int AddNode();

  struct Entry
  {
    unsigned int parentEntry;
    unsigned int nextEntry;
    unsigned int nodeId;
    bool         isFileNode;
  };
  CompareType               m_CompareType;
  unsigned int              m_IdCounter;
  std::vector<unsigned int> m_HashMap;
  std::vector<Entry>        m_Entries;
  std::vector<std::string>  m_Dictionary;

  friend class Iterator;
};

