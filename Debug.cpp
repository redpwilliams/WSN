#include "Debug.hpp"

DebugLevel Debug::debugLevel = DebugLevel::INFO;

static void Debug::setDebugLevel(DebugLevel level) {
  debugLevel = level;
}

static void Debug::Log(DebugLevel level, const String& message) {
  if (level == debugLevel) Serial.println(message);
}