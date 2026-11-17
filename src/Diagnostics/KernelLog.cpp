#include "KernelLog.hpp"
#include <IOKit/IOLib.h>
#include <libkern/libkern.h>

void KernelLog::write(const char *level, const char *component, const char *operation, const char *format, va_list arguments)
{
    char message[512] = {};
    if (format) vsnprintf(message, sizeof(message), format, arguments);
    IOLog("AsusACPIService [%s] [%s] %s: %s\n", level, component ? component : "Unknown", operation ? operation : "unknown", message);
}

void KernelLog::info(const char *component, const char *operation, const char *format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    write("INFO", component, operation, format, arguments);
    va_end(arguments);
}

void KernelLog::warning(const char *component, const char *operation, const char *format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    write("WARN", component, operation, format, arguments);
    va_end(arguments);
}

void KernelLog::error(const char *component, const char *operation, const char *format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    write("ERROR", component, operation, format, arguments);
    va_end(arguments);
}
