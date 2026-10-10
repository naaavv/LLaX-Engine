#pragma once

#include "Base.h"
#include "Log.h"

// ── Platform debug-break ──────────────────────────────────────────────────
#if defined(_WIN32)
    #define LLAX_DEBUGBREAK() __debugbreak()
#elif defined(__clang__) || defined(__GNUC__)
    #define LLAX_DEBUGBREAK() __builtin_trap()
#else
    #define LLAX_DEBUGBREAK() ((void)0)
#endif

// ── Assertions ────────────────────────────────────────────────────────────
// Always compiled in Debug; stripped to a no-op in Release unless
// LLAX_ENABLE_ASSERTS is explicitly defined by the build system.
#ifdef NDEBUG
    // Release build: asserts compile away to nothing
    #define LLAX_ASSERT(x, ...)      ((void)0)
    #define LLAX_CORE_ASSERT(x, ...) ((void)0)
#else
    // Debug build: log the failed expression, file, and line, then break.
    #define LLAX_ASSERT(x, ...)                                                  \
        do {                                                                     \
            if (!(x)) {                                                          \
                LLAX_ERROR("Assertion failed: ({}) — {}\n  at {}:{}", #x,       \
                    __VA_ARGS__, __FILE__, __LINE__);                            \
                LLAX_DEBUGBREAK();                                               \
            }                                                                    \
        } while(0)

    #define LLAX_CORE_ASSERT(x, ...)                                             \
        do {                                                                     \
            if (!(x)) {                                                          \
                LLAX_CORE_ERROR("Assertion failed: ({}) — {}\n  at {}:{}", #x,  \
                    __VA_ARGS__, __FILE__, __LINE__);                            \
                LLAX_DEBUGBREAK();                                               \
            }                                                                    \
        } while(0)
#endif
