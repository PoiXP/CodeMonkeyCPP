#pragma once

#include <string>

namespace FileUtils
{
  unsigned int GetFileSize(const std::string& filename);
  // unsigned int GetPreprocessorTokensCount(const std::string& filename);
}