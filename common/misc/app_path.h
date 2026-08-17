#ifndef APP_PATH_H
#define APP_PATH_H

/* Full path of the running executable, as it was launched. When the program
   is started through a symbolic link this is the link, not its target.
   Returns 0 on failure. */
const char* get_app_path(void);

/* Same, but with symbolic links and other reparse points resolved, so it
   always names the real image on disk. Returns 0 on failure (including on
   filesystems that cannot report a final path). */
const char* get_app_final_path(void);

#endif /* APP_PATH_H */
