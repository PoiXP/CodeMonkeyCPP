#ifndef UTILS_LOG_H
#define UTILS_LOG_H
#pragma once

namespace Log
{
  enum LogLevel
  {
    e_LogAll,
    e_LogInfo,
    e_LogWarning,
    e_LogError,
    e_LogOff,
  };

  void Initialize(const char* applicationName, LogLevel logLevel);
  void Shutdown();

  void LogMessage(LogLevel level, const char* format, ...);
  void LogMessage_va(LogLevel level, const char* format, va_list args);
}

#define LOG_INFO(...)     Log::LogMessage(Log::e_LogInfo,    ##__VA_ARGS__ );
#define LOG_WARNING(...)  Log::LogMessage(Log::e_LogWarning, ##__VA_ARGS__ );
#define LOG_ERROR(...)    Log::LogMessage(Log::e_LogError,   ##__VA_ARGS__ );

#endif // UTILS_LOG_H