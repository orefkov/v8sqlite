#pragma once

#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif //_WINDOWS

#if defined(__linux__) || defined(__APPLE__)
#define LINUX_OR_MACOS
#endif

#include <simstr/sstring.h>
using namespace simstr;

#include "AddInDefBase.h"
#include "ComponentBase.h"
#include "IMemoryManager.h"
#include <sqlite3.h>
