#pragma once
#include <Arduino.h>

inline void error(const char *message)
{
  Serial.println("Communication with WiFi module failed!");
  while (true)
    ;
}