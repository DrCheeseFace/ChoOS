#ifndef _KERNEL_MISC_H
#define _KERNEL_MISC_H

#define internal	static
#define global_variable static
#define local_persist	static

#define unused	  __attribute__((__unused__))
#define ignore(i) (void)i

#ifdef __clang__
#define NULLABLE _Nullable
#define NONNULL	 _Nonnull
#else
#define NULLABLE
#define NONNULL
#endif

#if defined(__GNUC__) || defined(__clang__)
#define WARN_UNUSED	  __attribute__((warn_unused_result))
#define NONNULL_ARGS(...) __attribute__((nonnull(__VA_ARGS__)))
#else
#define WARN_UNUSED
#define NONNULL_ARGS(...)
#endif

#endif // _KERNEL_MISC_H
