#pragma once

#include <string>
#include <vector>

class FileDictionary
{
public:
  class Iterator
  {
  public:
    ~Iterator();
    size_t     operator*() const;
    Iterator&  operator++();
    Iterator   operator++(int);
    bool       operator!=(const Iterator& iter) const;
    bool       operator==(const Iterator& iter) const;
  private:
    Iterator(const FileDictionary& dict, size_t handle);
    const FileDictionary&   m_Dict;
    size_t                  m_Handle;
    friend class FileDictionary;
  };

  enum CompareType
  {
    e_Compare_CaseSensitive,
    e_Compare_NoCase
  };

  static const unsigned int NO_INDEX  = 0xFFFFFFFF;

  FileDictionary(CompareType compareType, unsigned int dictionarySize = 0u);
  ~FileDictionary();
  
  size_t MakeHandle(const std::string& filename);
  size_t FindHandle(const std::string& filename) const;
  const std::string& GetFileName(size_t handle, std::string& filename) const;

  Iterator Begin() const;
  Iterator End() const;
private:

  size_t       GetNextLeafHandle(size_t handle, bool findFirst) const;
  size_t       FindHandleInternal(const std::string& path, std::string::const_iterator& start, unsigned int nodeId, unsigned int fromEntryId, bool& isLeaf, size_t& lastFoundEntryId, unsigned int& hashId) const;
  size_t       FindHandleRecursive(const std::string& path, std::string::const_iterator& start, unsigned int nodeId, unsigned int fromEntryId) const;
  size_t       MakeHandleRecursive(const std::string& path, std::string::const_iterator start, unsigned int nodeId, unsigned int fromEntryId);
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

