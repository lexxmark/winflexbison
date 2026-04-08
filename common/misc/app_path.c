#include <windows.h>

#define APP_PATH_LEN  1024
static char app_path[APP_PATH_LEN+1];

const char* get_app_path()
{
	/* First get the file this program was started from: */
	DWORD res = GetModuleFileNameA(NULL, app_path, APP_PATH_LEN);
	if (res == 0 || res == APP_PATH_LEN) 
	  {
		/* Error, or filename was longer than APP_PATH_LEN */
		return NULL;
	  }
	/* Now resolve the symlink, if it was a symlink */
	HANDLE h = CreateFileA(app_path, 0, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h == INVALID_HANDLE_VALUE) 
	  {
		/* Couldn't open it, but we do have the app path already */
		return app_path;
	  }
	res = GetFinalPathNameByHandleA(h, app_path, APP_PATH_LEN + 1, FILE_NAME_NORMALIZED);
	CloseHandle(h);
	if (res == 0 || res >= APP_PATH_LEN)
	  {
		return NULL;
	  }
	/* 
	* Flip backslash to forward slash 
	* because GetFinalPathNameByHandleA returns 'long filenames' with the "\\?\" prefix,
	* e.g. "\\?\C:\path\to\file"
	* and we later use xpath_join to join paths which have a forward slash
	* and fopen can't open files with the prefix and a mixture of '\' and '/'
	* (although it is OK if it doesn't have the prefix!)
	*/
	char* ptr = app_path;
	while(*ptr)
	  {
		if (*ptr == '\\')
			*ptr = '/';
		++ptr;
	  }
	return app_path;
}