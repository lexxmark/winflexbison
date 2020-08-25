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
#define _Noreturn

#ifndef _GL_INLINE
#define _GL_INLINE inline
#endif

extern char* _stpcpy(char *yydest, const char *yysrc);
