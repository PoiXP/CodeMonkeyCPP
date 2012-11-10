#include "Precompile.h"
#include "Utils/FileDictionary.h"

#define CHECK_NODE_NAME(name, handle) \
    { \
      std::string s; \
      dict.GetFileName(handle, s); \
      CHECK_EQUAL(name, s); \
    } \


SUITE(TestFile)
{
  TEST(testMakeHandle_NoCase)
  {
    FileDictionary dict(FileDictionary::e_Compare_NoCase, 32u);
    CHECK_EQUAL(0u, dict.MakeHandle("File1"));
    CHECK_EQUAL(1u, dict.MakeHandle("File2"));
    CHECK_EQUAL(2u, dict.MakeHandle("File3"));
    CHECK_EQUAL(1u, dict.MakeHandle("FILE2"));
    CHECK_EQUAL(5u, dict.MakeHandle("A\\B\\C"));
  }

  TEST(testMakeHandle_CaseSensitive)
  {
    FileDictionary dict(FileDictionary::e_Compare_CaseSensitive, 32u);
    CHECK_EQUAL(5u, dict.MakeHandle("a\\b\\c\\d\\e\\f"));
    CHECK_EQUAL(11u, dict.MakeHandle("A\\b\\C\\d\\E\\f"));
    CHECK_EQUAL(16u, dict.MakeHandle("a\\B\\c\\D\\e\\F"));
  }

  TEST(testMakeHandle_GetFileName)
  {
    FileDictionary dict(FileDictionary::e_Compare_NoCase, 32u);
    CHECK_EQUAL(0u, dict.MakeHandle("File1"));
    CHECK_EQUAL(1u, dict.MakeHandle("File2"));
    CHECK_EQUAL(2u, dict.MakeHandle("File3"));
    CHECK_EQUAL(3u, dict.MakeHandle("File3.bak"));

    CHECK_NODE_NAME("File1", 0u);
    CHECK_NODE_NAME("File2", 1u);
    CHECK_NODE_NAME("File3", 2u);
    CHECK_NODE_NAME("File3.bak", 3u);
  }

  TEST(testIterator_Walk)
  {
    FileDictionary dict(FileDictionary::e_Compare_NoCase, 32u);
    dict.MakeHandle("A\\B");
    dict.MakeHandle("A\\C\\G\\H");
    dict.MakeHandle("A");
    dict.MakeHandle("E\\F\\D");

    FileDictionary::Iterator it = dict.Begin();
    CHECK_NODE_NAME("A", *it++);
    CHECK_NODE_NAME("A\\B", *it++);
    CHECK_NODE_NAME("A\\C\\G\\H", *it++);
    CHECK_NODE_NAME("E\\F\\D", *it++);
    CHECK_EQUAL(true, dict.End() == it);
  }

}