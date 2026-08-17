#pragma once

#include "Base.h"
#include "Log.h"

#if defined(LLAX_PLATFORM_WINDOWS) || defined(_WIN32)
    #define LLAX_DEBUGBREAK() __debugbreak()
#elif defined(__clang__) || defined(__GNUC__)
    #define LLAX_DEBUGBREAK() __builtin_trap()
#else
    #define LLAX_DEBUGBREAK()
#endif

#ifndef LLAX_ENABLE_ASSERTS
    #define LLAX_ENABLE_ASSERTS
#endif

#ifdef LLAX_ENABLE_ASSERTS
    #define LLAX_ASSERT(x, ...) { if(!(x)) { LLAX_ERROR("Assertion Failed: {0}", __VA_ARGS__); LLAX_DEBUGBREAK(); } }
    #define LLAX_CORE_ASSERT(x, ...) { if(!(x)) { LLAX_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); LLAX_DEBUGBREAK(); } }
#else
    #define LLAX_ASSERT(x, ...)
    #define LLAX_CORE_ASSERT(x, ...)
#endif
