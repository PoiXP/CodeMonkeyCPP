#include "Precompile.h"
#include "Utils/File.h"

File::File()
{

}

bool File::Open(const char* filename)
{
  file.open(filename, std::ios_base::in);
  m_CurrentPosition = m_EndPosition = NULL;
  m_Buffer[BUFFER_LENGTH] = '\0';
  return file.is_open();
}

void File::Close()
{
  file.close();
}

bool File::EndOfFile()
{ 
  return (m_CurrentPosition == NULL) && file.eof();
}

void File::ReadLine(std::string& line)
{
  line.clear();
  line.reserve(1024);
  ReadLineInternal(line);
}

void File::ReadLineInternal(std::string& line)
{
  if (m_CurrentPosition != m_EndPosition)
  {
    const char* textPos = m_CurrentPosition;
    while( m_CurrentPosition != m_EndPosition )
    {
      switch(*m_CurrentPosition)
      {
      case '\0':
      case '\n':
        {
          line.insert(std::distance(line.begin(), line.end()), textPos, std::distance(textPos, m_CurrentPosition));
          ++m_CurrentPosition;
          return;
        }
        break;
      default:
        ++m_CurrentPosition;
      }
    }
    line.insert(std::distance(line.begin(), line.end()), textPos, std::distance(textPos, m_CurrentPosition));
    if (m_CurrentPosition == m_EndPosition) ReadLineInternal(line);
  }
  else
  {
    m_CurrentPosition = m_EndPosition = NULL;
    if (EndOfFile())
    {
      return;
    }
    file.read(m_Buffer, BUFFER_LENGTH);

    m_CurrentPosition = m_Buffer;
    m_EndPosition     = m_Buffer + file.gcount();
    return ReadLineInternal(line);
  }
}
