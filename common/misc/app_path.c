#include <windows.h>

#define APP_PATH_LEN  1024
static char app_path[APP_PATH_LEN+1];

const char* get_app_path()
{
	DWORD res = GetModuleFileName(NULL, app_path, APP_PATH_LEN);

	if (res == 0 || res == APP_PATH_LEN)
		return 0;

	app_path[APP_PATH_LEN] = '\0';
	return app_path;
}