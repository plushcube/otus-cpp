#pragma once

#include <cstddef>
#include <string>

#include <parser.h>

// Export/import macro: only these functions become visible in libasync.so.
#if defined(_WIN32) || defined(__CYGWIN__)
#define ASYNC_API_EXPORT __declspec(dllexport)
#define ASYNC_API_IMPORT __declspec(dllimport)
#else
#define ASYNC_API_EXPORT __attribute__((visibility("default")))
#define ASYNC_API_IMPORT
#endif

#ifdef ASYNC_BUILDING_LIBRARY
#define ASYNC_API ASYNC_API_EXPORT
#else
#define ASYNC_API ASYNC_API_IMPORT
#endif

using AsyncContext = Parser;
using Message = std::string;

ASYNC_API AsyncContext connect(const size_t &bulk_size);
ASYNC_API void receive(const AsyncContext &context, const Message &message);
ASYNC_API void disconnect(const AsyncContext &context);
