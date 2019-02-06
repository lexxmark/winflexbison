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
//#define _USE_32BIT_TIME_T
#define HAVE_LONG_LONG_INT 1

inline char* _stpcpy(char *yydest, const char *yysrc)
{
	char *yyd = yydest;
	const char *yys = yysrc;

	while ((*yyd++ = *yys++) != '\0')
		continue;

	return yyd - 1;
}