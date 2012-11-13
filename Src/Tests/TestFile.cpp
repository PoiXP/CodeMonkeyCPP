#include "Precompile.h"
#include "Utils/File.h"

SUITE(TestFile)
{
  TEST(testOpen_Negative)
  {
    const char filename[] = "???";

    File file;
    CHECK_EQUAL(false, boost::filesystem::exists(filename));
    CHECK_EQUAL(false, file.Open(filename));
  }
  TEST(testOpen_Positive)
  {
    const char filename[] = "open.test";
    std::fstream fileStream;
    fileStream.open(filename, std::ios_base::out);
    fileStream << "Test!" << std::endl;
    fileStream.close();

    File file;
    CHECK_EQUAL(true, boost::filesystem::exists(filename));
    CHECK_EQUAL(true, file.Open(filename));
    file.Close();
    boost::filesystem::remove(filename);
  }
  TEST(testReadLine1)
  {
    const char filename[] = "read.test";

    File file;

    std::fstream fileStream;
    fileStream.open(filename, std::ios_base::out);
    const int STRING_LENGTH = 64 * 1024;
    for (int i=0; i < STRING_LENGTH; i++)
    {
      char ch = '0' + ( i % ('Z' - '0' + 1));
      fileStream << ch;
    }
    fileStream.close();

    CHECK_EQUAL(true, file.Open(filename));
    std::string line;
    file.ReadLine(line);
    CHECK_EQUAL(true, file.EndOfFile());
    CHECK_EQUAL(STRING_LENGTH, line.length());
    for (int i=0; i < STRING_LENGTH; i++)
    {
      char ch = '0' + ( i % ('Z' - '0' + 1));
      CHECK_EQUAL(ch, line[i]);
    }
  }
  TEST(testReadLine2)
  {
    const char filename[] = "read.test";

    File file;

    std::fstream fileStream;
    fileStream.open(filename, std::ios_base::out);
    const int STRING_LENGTH = 1024;
    for (int i=0; i < STRING_LENGTH; i++)
    {
      char ch = '0' + ( i % ('Z' - '0' + 1));
      fileStream << ch << std::endl;
    }
    fileStream.close();

    CHECK_EQUAL(true, file.Open(filename));
    std::string line;
    for (int i=0; i < STRING_LENGTH; i++)
    {
      file.ReadLine(line);
      CHECK_EQUAL(1, line.length());
      char ch = '0' + ( i % ('Z' - '0' + 1));
      CHECK_EQUAL(ch, line[0]);
    }
    file.ReadLine(line);
    CHECK_EQUAL(true, file.EndOfFile());
  }


  TEST(testEmptyString)
  {
    const char filename[] = "empty_read.test";

    File file;

    const int LINES_COUNT = 16;

    std::fstream fileStream;
    fileStream.open(filename, std::ios_base::out);
    for (int i = 0; i < LINES_COUNT; i++)
    {
      fileStream << std::endl;
    }
    fileStream.close();

    CHECK_EQUAL(true, file.Open(filename));
    for (int i=0; i < LINES_COUNT; i++)
    {
      std::string line;
      file.ReadLine(line);
      CHECK_EQUAL(true, line.empty());
      CHECK_EQUAL(false, file.EndOfFile());
    }
    std::string line;
    file.ReadLine(line);
    CHECK_EQUAL(true, file.EndOfFile());
  }


}