#ifndef DEBUG_HPP
#define DEBUG_HPP

#include "Arduino.h"

enum class DebugLevel {
  NONE,
  INFO,
  WARN,
  ERROR
};

class Debug {
  public:
    static void setDebugLevel(DebugLevel level);
    static void Log(DebugLevel level, const String& message);
  
  private:
    static DebugLevel debugLevel;
};

#endif