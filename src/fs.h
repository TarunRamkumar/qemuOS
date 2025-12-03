#ifndef FS_H
#define FS_H

#include <stdint.h>

// File system constants
#define MAX_FILES 16
#define MAX_FILENAME_LEN 32
#define MAX_FILE_SIZE 4096
#define MAX_OPEN_FDS 16

// File descriptor flags
#define FD_READ 0x1
#define FD_WRITE 0x2

// File system operations
int fs_init(void);
int fs_create(const char *name);
int fs_delete(const char *name);
int fs_open(const char *name, int flags);
int fs_close(int fd);
int fs_read(int fd, char *buf, int len);
int fs_write(int fd, const char *buf, int len);
int fs_list_files(char *buf, int maxlen);
int fs_get_file_size(const char *name);
int fs_seek(int fd, int offset);

// Legacy compatibility functions
const char* fs_get_file_content(const char *name, int *len);

#endif
