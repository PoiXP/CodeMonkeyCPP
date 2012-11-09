#include "Precompile.h"
#include "Utils/FileUtils.h"
#include <boost/filesystem.hpp>

namespace FileUtils
{

unsigned int GetFileSize(const std::string& filename)
{
  boost::filesystem::path fullpath(filename);
  if ( !boost::filesystem::exists(fullpath))
  {
    return 0u;
  }
  return static_cast<unsigned int>(boost::filesystem::file_size(fullpath));
}

//unsigned int GetPreprocessorTokensCount(const std::string& filename)
//{
//  boost::filesystem::path fullpath(filename);
//  if ( !boost::filesystem::exists(fullpath))
//  {
//    return 0u;
//  }
//  File file;
//  file.Open(filename.c_str());
//
//  enum State
//  {
//    e_WaitForNewToken,
//    e_Identifier,
//    e_Number,
//    e_FloatNumber,
//    e_String,
//    e_OpenSlash,
//    e_MultiLineComment,
//  };
//
//  unsigned int result = 0u;
//  State state = e_WaitForNewToken;
//  const std::string delimiters("-+=*/<>(){}!~:%#;,^[]'?");
//  while (!file.EndOfFile())
//  {
//    std::string line;
//    file.ReadLine(line);
//    auto it=line.cbegin();
//    while(it != line.cend())
//    {
//      switch(state)
//      {
//      case e_WaitForNewToken:
//        if (isalpha(*it) || *it == '_')
//        {
//          state = e_Identifier;
//        }
//        else if(isdigit(*it) || *it == '.' )
//        {
//          state = e_Number;
//        }
//        else if(*it == '/')
//        {
//          state = e_OpenSlash;
//          ++it;
//        }
//        else if(delimiters.find(*it) != std::string::npos)
//        {
//          result += 1;
//          ++it;
//        }
//        else if (*it == '"')
//        {
//          state = e_String;
//          ++it;
//        }
//        else
//        {
//          ++it;
//        }
//        break;
//      case e_Identifier:
//        if (isalpha(*it) || isdigit(*it) || *it == '_' )
//        {
//          ++it;
//        }
//        else
//        {
//          result += 1;
//          state = e_WaitForNewToken;
//        }
//        break;
//      case e_Number:
//        if (isdigit(*it))
//        {
//          ++it;
//        }
//        else if (*it == '.')
//        {
//          state = e_FloatNumber;
//          ++it;
//        }
//        else if(*it == 'e')
//        {
//          state = e_WaitForNewToken;
//          ++it;
//        }
//        else
//        {
//          result += 1;
//          state = e_WaitForNewToken;
//        }
//        break;
//      case e_FloatNumber:
//        if (isdigit(*it) || *it == 'f')
//        {
//          ++it;
//        }
//        else
//        {
//          state = e_WaitForNewToken;
//          result += 1;
//        }
//        break;
//      case e_String:
//        if (*it == '"')
//        {
//          state = e_WaitForNewToken;
//          result += 1;
//          ++it;
//        }
//        else if (*it == '\\')
//        {
//          ++it;
//          if (it != line.cend())
//          {
//            ++it;
//          }
//        }
//        else
//        {
//          ++it;
//        }
//        break;
//      case e_OpenSlash:
//        if (*it == '/' )
//        {
//          it = line.cend();
//          result += 1;
//          state = e_WaitForNewToken;
//        }
//        else if (*it == '*')
//        {
//          result += 1;
//          ++it;
//          state = e_MultiLineComment; 
//        }
//        else
//        {
//          result += 1;
//          state = e_WaitForNewToken;
//        }
//        break;
//      case e_MultiLineComment:
//        if (*it == '*')
//        {
//          ++it;
//          if (it != line.cend() && *it == '/')
//          {
//            ++it;
//            state = e_WaitForNewToken;
//          }
//        }
//        else
//        {
//          ++it;
//        }
//      }
//    }
//  }
//  return result;
//}
//
//
}