#ifndef _PATHSTUFF_H
#define _PATHSTUFF_H

extern char * convert_Path_to_win32(char *Path, char to_delim);
extern char * w32ify(char *file, int resolve);
extern char * getcwd_fs(char *buf, int len);

#define convert_vpath_to_win32(vpath, delim) convert_Path_to_win32(vpath, delim)

#endif
