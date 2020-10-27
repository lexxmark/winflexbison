#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <windows.h>
/**
 * Opens the file whose name is the string pointed to by file and associates
 * a stream with it.
 * Credit to Xavier Roche http://blog.httrack.com/blog/2013/10/05/creating-deletable-and-movable-files-on-windows
 **/
FILE* fopen_unixlike(const char* file, const char* mode) {
    DWORD dwDesiredAccess = 0;
    DWORD dwCreationDisposition = 0;
    const DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
    const DWORD dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
    size_t i = 0;

    HANDLE handle = NULL;
    int fd = 0;

#if defined _WIN32 && ! defined __CYGWIN__
    if (strcmp(file, "/dev/null") == 0)
        file = "NUL";
#endif

    /* Infer flags. */
    for (i = 0; mode[i] != '\0'; i++) {
        switch (mode[i]) {
        case 'r':
            dwDesiredAccess |= GENERIC_READ;
            dwCreationDisposition = OPEN_EXISTING;
            break;
        case 'a':
            dwDesiredAccess |= GENERIC_WRITE;
            dwCreationDisposition = CREATE_NEW;
            break;
        case 'w':
            dwDesiredAccess |= GENERIC_WRITE;
            dwCreationDisposition = CREATE_ALWAYS;
            break;
        case '+':
            dwDesiredAccess |= GENERIC_READ;
            dwDesiredAccess |= GENERIC_WRITE;
            break;
        }
    }

    /* Create the file. */
    handle = CreateFileA(file,
        dwDesiredAccess,
        dwShareMode,
        NULL,
        dwCreationDisposition,
        dwFlagsAndAttributes,
        NULL);
    if (handle == INVALID_HANDLE_VALUE) {
        return NULL;
    }

    /* Associates a C run-time file descriptor with a file HANDLE. */
    fd = _open_osfhandle((intptr_t)handle, _O_BINARY);
    if (fd == -1) {
        CloseHandle(handle);
        return NULL;
    }

    /* Associates a stream with a C run-time file descriptor. */
    return _fdopen(fd, mode);
}

