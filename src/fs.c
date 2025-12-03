#include "fs.h"
#include <string.h>

extern const char _prog_hello[];
extern const char _prog_echo[];

typedef struct {
    const char *name;
    const char *data;
} file_entry_t;

const file_entry_t files[] = {
    {"hello", _prog_hello},
    {"echo", _prog_echo},
};

int fs_list_files(char *buf, int maxlen) {
    int pos = 0;
    for (int i = 0; i < 2; i++) {
        int l = strlen(files[i].name);
        if (pos + l + 2 >= maxlen) break;
        memcpy(buf + pos, files[i].name, l);
        pos += l;
        buf[pos++] = '\n';
    }
    if (pos < maxlen) buf[pos] = 0;
    return pos;
}

const char* fs_get_file_content(const char *name, int *len) {
    for (int i = 0; i < 2; i++) {
        if (strcmp(name, files[i].name) == 0) {
            *len = strlen(files[i].data);
            return files[i].data;
        }
    }
    *len = 0;
    return 0;
}
