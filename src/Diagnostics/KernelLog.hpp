#ifndef ASUS_ACPI_KERNEL_LOG_HPP
#define ASUS_ACPI_KERNEL_LOG_HPP

#include <stdarg.h>

class KernelLog {
public:
    static void info(const char *component, const char *operation, const char *format, ...) __attribute__((format(printf, 3, 4)));
    static void warning(const char *component, const char *operation, const char *format, ...) __attribute__((format(printf, 3, 4)));
    static void error(const char *component, const char *operation, const char *format, ...) __attribute__((format(printf, 3, 4)));

private:
    static void write(const char *level, const char *component, const char *operation, const char *format, va_list arguments);
};

#endif
