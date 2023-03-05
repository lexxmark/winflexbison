#include "pid_tempname.h"
#include <stdio.h>
#include <process.h>

char temp_prefix[40];

// This adds the unique process id to the built in _tempnam to prevent race condition
// This race condition is more prevalent in Ninja as it parallelizes build and instantiates multiple win_flex/win_bison
// flex_temp_out_main fails frequently as its used for a longer time (compared to temp_file_names),
// hence more possibility to overlap
const char *pid_tempname (const char *prefix)
{
    // Here is the race condition:
    //
    //           flex A
    //              |                                flex B
    //              ↓                                   |
    //   _tempnam returns ~file_X                       ↓
    //              |                        _tempnam returns ~file_X
    //              ↓                                   |
    //      freopen ~file_X w+                          ↓
    //              |                           freopen ~file_X w+
    //              ↓                                   |
    //             ...                                  ↓
    //              |                                  ...
    //              ↓                                   |
    //       _unlink ~file_X                           ...       <- Fails to print
    //                                                  ↓
    //                                           _unlink ~file_X <- Fails to delete
    //
    // It is also possible that ~file_X was deleted before the lagging win_flex is done, which can cause other errors,
    // hence changing to non-fatal error upon missing ~file_X on _unlink will not work all the time
    // This problem can be prevented with an even more unique temporary file name by adding the current process ID
    // Synchronization is not necessary either as there is no reason for the independent processes to wait...
    // The reason why this is an issue is because Windows filesystem is reflected immediately,
    // unlike inode in Linux that only properly deletes files when their link count is down to zero
    sprintf(temp_prefix, "%s%d_", prefix, _getpid());
    return _tempnam(NULL, temp_prefix);
}