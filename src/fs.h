#ifndef FS_H
#define FS_H

int fs_list_files(char *buf, int maxlen);
const char* fs_get_file_content(const char *name, int *len);

#endif
