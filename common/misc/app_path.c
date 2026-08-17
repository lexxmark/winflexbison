#include <windows.h>
#include <string.h>

#include "app_path.h"

#define APP_PATH_LEN  1024
static char app_path[APP_PATH_LEN+1];
static char app_final_path[APP_PATH_LEN+1];

const char* get_app_path()
{
	DWORD res = GetModuleFileNameA(NULL, app_path, APP_PATH_LEN);

	if (res == 0 || res == APP_PATH_LEN)
		return 0;

	app_path[APP_PATH_LEN] = '\0';
	return app_path;
}

/* GetModuleFileName reports the path the process was launched with, so when
   win_bison is started through a symbolic link -- WinGet installs the package
   under .../WinGet/Packages/ and only symlinks the executables into
   .../WinGet/Links/ -- it names the link, whose directory holds no data/.
   Reopen the image and ask the filesystem for its real name. */
const char* get_app_final_path()
{
	const char* launched;
	HANDLE h;
	DWORD res;
	char* p;

	launched = get_app_path();
	if (!launched)
		return 0;

	/* Access 0 is enough to name the file, and skips the sharing check
	   the loader's own handle on the running image would otherwise lose. */
	h = CreateFileA(launched, 0,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h == INVALID_HANDLE_VALUE)
		return 0;

	res = GetFinalPathNameByHandleA(h, app_final_path, APP_PATH_LEN,
		FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
	CloseHandle(h);

	/* 0 is failure; >= the size we passed means it wanted more room. */
	if (res == 0 || res >= APP_PATH_LEN)
		return 0;

	app_final_path[APP_PATH_LEN] = '\0';

	/* The name comes back in \\?\ form, which fopen() rejects. Strip the
	   prefix rather than escaping it: what is left is an ordinary path
	   every consumer down the line already handles. */
	p = app_final_path;
	if (strncmp(p, "\\\\?\\UNC\\", 8) == 0)
	{
		/* \\?\UNC\server\share -> \\server\share */
		p += 6;
		p[0] = '\\';
	}
	else if (strncmp(p, "\\\\?\\", 4) == 0)
		p += 4;

	return p;
}
