#include "Debug.hpp"

#include <stdarg.h>

#define MAX_OUTPUT_DEVICES 10

namespace {
static const char *const g_LogSeverity[] = {
    [static_cast<int>(Debug::Level::Debug)] = "\033[1;30m",
    [static_cast<int>(Debug::Level::Info)] = "\033[0;37m",
    [static_cast<int>(Debug::Level::Warn)] = "\033[1;33m",
    [static_cast<int>(Debug::Level::Error)] = "\033[1;31m",
    [static_cast<int>(Debug::Level::Critical)] = "\033[2;37;41m",
};

static const char *const g_ColorReset = "\033[0m";

struct {
  Debug::Level LogLevel;
  TextDevice *Device;
  bool Colored;
} g_OutputDevices[MAX_OUTPUT_DEVICES];

int g_OutputDevicesCount;
} // namespace

namespace Debug {

void AddOutputDevice(Level minLogLevel, TextDevice *device, bool colorOutput) {
  g_OutputDevices[g_OutputDevicesCount].Device = device;
  g_OutputDevices[g_OutputDevicesCount].LogLevel = minLogLevel;
  g_OutputDevices[g_OutputDevicesCount].Colored = colorOutput;
  g_OutputDevicesCount++;
}

static void Log(const char *module, Level level, const char *fmt,
                va_list args) {
  for (int i = 0; i < g_OutputDevicesCount; i++) {
    if (level < g_OutputDevices[i].LogLevel)
      continue;

    if (g_OutputDevices[i].Colored)
      g_OutputDevices[i].Device->Write(g_LogSeverity[static_cast<int>(level)]);

    g_OutputDevices[i].Device->Format("[%s]: ", module);
    g_OutputDevices[i].Device->VFormat(fmt, args);

    if (g_OutputDevices[i].Colored)
      g_OutputDevices[i].Device->Write(g_ColorReset);

    g_OutputDevices[i].Device->Write("\n");
  }
}

void Debug(const char *module, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  Log(module, Level::Debug, fmt, args);
  va_end(args);
}

void Info(const char *module, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  Log(module, Level::Info, fmt, args);
  va_end(args);
}

void Warn(const char *module, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  Log(module, Level::Warn, fmt, args);
  va_end(args);
}

void Error(const char *module, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  Log(module, Level::Error, fmt, args);
  va_end(args);
}

void Critical(const char *module, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  Log(module, Level::Critical, fmt, args);
  va_end(args);
}

void DebugBuffer(const char *module, const char *msg, void *buffer,
                 size_t size) {
  for (int i = 0; i < g_OutputDevicesCount; i++) {
    if (Level::Debug < g_OutputDevices[i].LogLevel)
      continue;

    if (g_OutputDevices[i].Colored)
      g_OutputDevices[i].Device->Write(
          g_LogSeverity[static_cast<int>(Level::Debug)]);

    g_OutputDevices[i].Device->Format("[%s]: ", module);
    g_OutputDevices[i].Device->FormatBuffer(msg, buffer, size);

    if (g_OutputDevices[i].Colored)
      g_OutputDevices[i].Device->Write(g_ColorReset);

    g_OutputDevices[i].Device->Write("\n");
  }
}
} // namespace Debug
