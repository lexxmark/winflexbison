#include <io.h>

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define ssize_t ptrdiff_t

#define _GL_INLINE_HEADER_BEGIN
#define _GL_INLINE_HEADER_END
#define _GL_EXTERN_INLINE inline
#define _GL_ATTRIBUTE_PURE
#define _GL_ATTRIBUTE_CONST
#define _GL_UNUSED
#define _GL_ATTRIBUTE_NODISCARD
#define _GL_ATTRIBUTE_MAYBE_UNUSED
#define HAVE_LONG_LONG_INT 1
# define _GL_ATTRIBUTE_FORMAT(spec) /* empty */
#define HAVE_DECL_STRERROR_R 1
#define HAVE_DECL___ARGV 1
#define _GL_ARG_NONNULL(a)
#define _GL_ATTRIBUTE_FORMAT_PRINTF(a, b)
#define _GL_ATTRIBUTE_DEPRECATED
#define _GL_ATTRIBUTE_FALLTHROUGH
#define GNULIB_XALLOC 1
#define _GL_ATTRIBUTE_MALLOC
#define _GL_ATTRIBUTE_ALLOC_SIZE(a)
#define _GL_ATTRIBUTE_DEALLOC_FREE
#define _GL_ATTRIBUTE_DEALLOC(f, i)
#define _GL_ATTRIBUTE_RETURNS_NONNULL
#define _GL_ATTRIBUTE_COLD
#define _GL_ASYNC_SAFE
#define GNULIB_XALLOC_DIE 1
#define _GL_HAVE__STATIC_ASSERT
#define _Static_assert(a, b)
#define __PGI

#define MIN min
#define _GL_CMP(a, b) (a) == (b)

#define __getopt_argv_const
#define _Noreturn

#ifndef _GL_INLINE
#define _GL_INLINE inline
#endif

#ifndef FILE_SYSTEM_DRIVE_PREFIX_CAN_BE_RELATIVE
#define FILE_SYSTEM_DRIVE_PREFIX_CAN_BE_RELATIVE 1
#endif

#define S_ISDIR(mode) (mode & _S_IFDIR)

extern char* _stpcpy(char *yydest, const char *yysrc);
