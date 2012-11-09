#include "Precompile.h"
#include "Utils/Log.h"

namespace
{
  class LogFormatter : public logog::FormatterMSVC
  {
    virtual TOPIC_FLAGS GetTopicFlags( const logog::Topic &topic )
    {
      return TOPIC_TIMESTAMP_FLAG | TOPIC_LEVEL_FLAG | TOPIC_MESSAGE_FLAG;
    }
  };

  static logog::LogFile* s_logFile;
  static logog::Cout*    s_logCout;
  static LogFormatter*   s_Formatter;
  static Log::LogLevel   s_LogLevel;
}

void Log::Initialize(const char* applicationName, LogLevel logLevel)
{
  LOGOG_INITIALIZE();
  std::string filename(applicationName);
  filename += ".log";
  s_Formatter = new LogFormatter;
  s_Formatter->SetShowTimeOfDay(true);
  s_logFile = new logog::LogFile(filename.c_str());
  s_logFile->SetFormatter(*s_Formatter);
  s_logCout = new logog::Cout();
  s_logCout->SetFormatter(*s_Formatter);
  s_LogLevel = logLevel;
}

void Log::Shutdown()
{
  delete s_logCout;
  delete s_logFile;
  delete s_Formatter;
  LOGOG_SHUTDOWN();
}

void Log::LogMessage(Log::LogLevel level, const char* format, ...)
{
  va_list args;
  va_start(args, format);
  LogMessage_va(level, format, args);
  va_end(args);
}

void Log::LogMessage_va(Log::LogLevel level, const char* format, va_list args)
{
  if (level < s_LogLevel)
  {
    return;
  }
  logog::String message("");
  message.format_va(format, args);

  switch(level)
  {
  case e_LogInfo:     INFO(message); break;
  case e_LogWarning:  WARN(message); break;
  case e_LogError:    ERR(message); break;
  }

}
