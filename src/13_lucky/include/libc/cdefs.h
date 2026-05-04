#pragma once

// Keeps C symbols unmangled when these headers are included from C++ files
// This is primarily used for the new and delete op
#ifdef __cplusplus
#define EXTERN_C_BEGIN extern "C" {
#define EXTERN_C_END }
#else
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#endif
