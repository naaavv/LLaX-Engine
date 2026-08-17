#pragma once

#include <memory>

#if defined(LLAX_PLATFORM_WINDOWS) || defined(_WIN32)
    #define LLAX_DEBUGBREAK() __debugbreak()
#elif defined(__clang__) || defined(__GNUC__)
    #define LLAX_DEBUGBREAK() __builtin_trap()
#else
    #define LLAX_DEBUGBREAK()
#endif

#ifdef LLAX_DEBUG
    #define LLAX_ENABLE_ASSERTS
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

#define BIT(x) (1 << (x))
#define LLAX_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

namespace LLaX
{
    template<typename T>
    using Scope = std::unique_ptr<T>;
    template<typename T, typename ... Args>
    constexpr Scope<T> CreateScope(Args&& ... args)
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template<typename T>
    using Ref = std::shared_ptr<T>;
    template<typename T, typename ... Args>
    constexpr Ref<T> CreateRef(Args&& ... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }
}
