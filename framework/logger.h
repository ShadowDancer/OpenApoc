#pragma once

#include "library/strings.h"

#if defined(_MSC_VER) && _MSC_VER > 1400
#include <sal.h>
#endif

#define XSTR(s) STR(s)
#define STR(s) #s

/* If UNIT_TEST is defined don't try to pop up a dialog */

#ifdef UNIT_TEST
#define LOGGER_SHOW_DIALOG false
#else
#define LOGGER_SHOW_DIALOG true
#endif

#if defined(_MSC_VER)
#define NORETURN_FUNCTION __declspec(noreturn)
#else
#define NORETURN_FUNCTION __attribute__((noreturn))
#endif

/* The logger is global state as we want it to be available even if the framework hasn't been
 * successfully initialised */

namespace OpenApoc
{
/* MSVC doesn't ahve __PRETTY_FUNCTION__ but __FUNCSIG__? */
// FIXME: !__GNUC__ isn't the same as MSVC
#ifndef __GNUC__
#define LOGGER_PREFIX __FUNCSIG__
#define PRINTF_ATTR(fmt_string_no, vararg_start_no)
#else
#define LOGGER_PREFIX __PRETTY_FUNCTION__
#define PRINTF_ATTR(fmt_string_no, vararg_start_no)                                                \
	__attribute__((format(printf, fmt_string_no, vararg_start_no)))
#endif

enum class LogLevel
{
	Info,
	Warning,
	Error,
};
// All format strings (%s) are expected to be UTF8
void Log(bool show_dialog, LogLevel level, UString prefix, const char *format, ...)
    PRINTF_ATTR(4, 5);

NORETURN_FUNCTION void _logAssert(bool show_dialog, UString prefix, UString string, int line,
                                  UString file);

// All logger output will be UTF8
}; // namespace OpenApoc

#define LogAssert(X)                                                                               \
	do                                                                                             \
	{                                                                                              \
		if (!(X))                                                                                  \
			OpenApoc::_logAssert(LOGGER_SHOW_DIALOG, LOGGER_PREFIX, STR(X), __LINE__, __FILE__);   \
	} while (0)

//#ifndef __ANDROID__
#if defined(__GNUC__)
// GCC has an extension if __VA_ARGS__ are not supplied to 'remove' the precending comma
#define LogInfo(f, ...)                                                                            \
	OpenApoc::Log(LOGGER_SHOW_DIALOG, OpenApoc::LogLevel::Info, OpenApoc::UString(LOGGER_PREFIX),  \
	              f, ##__VA_ARGS__)
#define LogWarning(f, ...)                                                                         \
	OpenApoc::Log(LOGGER_SHOW_DIALOG, OpenApoc::LogLevel::Warning,                                 \
	              OpenApoc::UString(LOGGER_PREFIX), f, ##__VA_ARGS__)
#define LogError(f, ...)                                                                           \
	OpenApoc::Log(LOGGER_SHOW_DIALOG, OpenApoc::LogLevel::Error, OpenApoc::UString(LOGGER_PREFIX), \
	              f, ##__VA_ARGS__)
#else
// At least msvc automatically removes the comma
#define LogInfo(f, ...)                                                                            \
	OpenApoc::Log(LOGGER_SHOW_DIALOG, OpenApoc::LogLevel::Info, LOGGER_PREFIX, f, __VA_ARGS__)
#define LogWarning(f, ...)                                                                         \
	OpenApoc::Log(LOGGER_SHOW_DIALOG, OpenApoc::LogLevel::Warning, LOGGER_PREFIX, f, __VA_ARGS__)
#define LogError(f, ...)                                                                           \
	OpenApoc::Log(LOGGER_SHOW_DIALOG, OpenApoc::LogLevel::Error, LOGGER_PREFIX, f, __VA_ARGS__)
#endif
//#else
#if 0
#define LogInfo(f, ...)
#define LogWarning(f, ...)
#define LogError(f, ...)
#endif
//#endif
