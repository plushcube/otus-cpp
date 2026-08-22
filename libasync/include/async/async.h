#pragma once

#include <cstddef>

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

using ContextID = size_t;

ASYNC_API ContextID connect(const size_t &block_size);
ASYNC_API void receive(const ContextID &cid, const char *buffer, const size_t &size);
ASYNC_API void disconnect(const ContextID &cid);
