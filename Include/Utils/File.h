#pragma once

#include <string>
#include <fstream>

class File
{
public:
  File();
  bool Open(const char* filename);
  void Close();
  bool EndOfFile();
  void ReadLine(std::string& line);
private:
  void ReadLineInternal(std::string& line);

  static const int BUFFER_LENGTH = 4096;
  /// Handle to file
  std::fstream file;
  /// Internal read buffer
  char m_Buffer[BUFFER_LENGTH+1];
  /// Pointer to current position in the buffer
  const char* m_CurrentPosition;
  /// Pointer to the end of the buffer
  const char* m_EndPosition;
};