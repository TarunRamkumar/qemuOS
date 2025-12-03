#include "fs.h"
#include "string.h"
#include <stdint.h>

// File metadata structure
typedef struct {
    char name[MAX_FILENAME_LEN];
    char *data;
    int size;
    int capacity;
    int in_use;
    int is_embedded;  // 1 if data points to embedded (read-only) data
} file_t;

// File descriptor entry
typedef struct {
    int file_index;      // Index into files array
    int position;        // Current read/write position
    int flags;           // Open flags (read/write)
    int in_use;          // Whether this FD is in use
} fd_entry_t;

// File system storage
static file_t files[MAX_FILES];
static fd_entry_t fd_table[MAX_OPEN_FDS];
static int next_fd = 3;  // Start at 3 (0,1,2 reserved for stdin, stdout, stderr)

// External program data (for initial files)
extern const char _prog_hello[];
extern const char _prog_echo[];

// Initialize file system
int fs_init(void) {
    // Clear all files
    for (int i = 0; i < MAX_FILES; i++) {
        files[i].in_use = 0;
        files[i].name[0] = 0;
        files[i].data = 0;
        files[i].size = 0;
        files[i].capacity = 0;
        files[i].is_embedded = 0;
    }
    
    // Clear allocation tracking
    for (int i = 0; i < MAX_FILES; i++) {
        pool_allocated[i] = 0;
    }
    
    // Clear file descriptor table
    for (int i = 0; i < MAX_OPEN_FDS; i++) {
        fd_table[i].in_use = 0;
        fd_table[i].file_index = -1;
        fd_table[i].position = 0;
        fd_table[i].flags = 0;
    }
    
    // Create initial files from embedded data
    const char *initial_names[] = {"hello", "echo"};
    const char *initial_data[] = {_prog_hello, _prog_echo};
    
    for (int i = 0; i < 2; i++) {
        int idx = i;
        strncpy(files[idx].name, initial_names[i], MAX_FILENAME_LEN - 1);
        files[idx].name[MAX_FILENAME_LEN - 1] = 0;
        
        int len = strlen(initial_data[i]);
        files[idx].size = len;
        files[idx].capacity = len + 1;
        files[idx].data = (char *)initial_data[i];  // Point to embedded data
        files[idx].is_embedded = 1;  // Mark as read-only embedded data
        files[idx].in_use = 1;
    }
    
    return 0;
}

// Find a file by name
static int find_file(const char *name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].in_use && strcmp(files[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

// Find an empty file slot
static int find_empty_file_slot(void) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (!files[i].in_use) {
            return i;
        }
    }
    return -1;
}

// Find an empty file descriptor slot
static int find_empty_fd_slot(void) {
    for (int i = 0; i < MAX_OPEN_FDS; i++) {
        if (!fd_table[i].in_use) {
            return i;
        }
    }
    return -1;
}

// File data storage pool (one buffer per file slot)
static char file_data_pool[MAX_FILES][MAX_FILE_SIZE];
static int pool_allocated[MAX_FILES];  // Track which slots are allocated

// Allocate memory for file data (returns pointer to a file's buffer)
static char* allocate_file_data(int file_index) {
    if (file_index < 0 || file_index >= MAX_FILES) {
        return 0;
    }
    
    if (pool_allocated[file_index]) {
        return 0;  // Already allocated
    }
    
    pool_allocated[file_index] = 1;
    return file_data_pool[file_index];
}

// Free file data
static void free_file_data(int file_index) {
    if (file_index >= 0 && file_index < MAX_FILES) {
        pool_allocated[file_index] = 0;
    }
}

// Create a new file
int fs_create(const char *name) {
    if (!name || strlen(name) == 0 || strlen(name) >= MAX_FILENAME_LEN) {
        return -1;  // Invalid name
    }
    
    // Check if file already exists
    if (find_file(name) >= 0) {
        return -1;  // File already exists
    }
    
    // Find empty slot
    int idx = find_empty_file_slot();
    if (idx < 0) {
        return -1;  // No space for new file
    }
    
    // Allocate data buffer for this file slot
    char *data = allocate_file_data(idx);
    if (!data) {
        return -1;  // Out of memory
    }
    
    // Initialize file
    strncpy(files[idx].name, name, MAX_FILENAME_LEN - 1);
    files[idx].name[MAX_FILENAME_LEN - 1] = 0;
    files[idx].data = data;
    files[idx].size = 0;
    files[idx].capacity = MAX_FILE_SIZE;
    files[idx].is_embedded = 0;  // Writable file
    files[idx].in_use = 1;
    
    return 0;
}

// Delete a file
int fs_delete(const char *name) {
    int idx = find_file(name);
    if (idx < 0) {
        return -1;  // File not found
    }
    
    // Close all file descriptors pointing to this file
    for (int i = 0; i < MAX_OPEN_FDS; i++) {
        if (fd_table[i].in_use && fd_table[i].file_index == idx) {
            fd_table[i].in_use = 0;
        }
    }
    
    // Mark file as unused
    if (!files[idx].is_embedded) {
        free_file_data(idx);  // Free the buffer if it was allocated
    }
    files[idx].in_use = 0;
    files[idx].name[0] = 0;
    files[idx].size = 0;
    files[idx].is_embedded = 0;
    
    return 0;
}

// Open a file
int fs_open(const char *name, int flags) {
    int idx = find_file(name);
    if (idx < 0) {
        return -1;  // File not found
    }
    
    // Find empty FD slot
    int fd_slot = find_empty_fd_slot();
    if (fd_slot < 0) {
        return -1;  // Too many open files
    }
    
    // Allocate FD number
    int fd = next_fd++;
    if (next_fd >= MAX_OPEN_FDS) {
        next_fd = 3;  // Wrap around (skip 0,1,2)
    }
    
    // Initialize FD entry
    fd_table[fd_slot].in_use = 1;
    fd_table[fd_slot].file_index = idx;
    fd_table[fd_slot].position = 0;
    fd_table[fd_slot].flags = flags;
    
    // Store FD number in the slot (we'll use a simple mapping)
    // For simplicity, we'll use the slot index as the FD
    // In a real OS, this would be more sophisticated
    return fd_slot + 3;  // Return FD >= 3
}

// Close a file descriptor
int fs_close(int fd) {
    if (fd < 3 || fd >= 3 + MAX_OPEN_FDS) {
        return -1;  // Invalid FD
    }
    
    int fd_slot = fd - 3;
    if (!fd_table[fd_slot].in_use) {
        return -1;  // FD not open
    }
    
    fd_table[fd_slot].in_use = 0;
    fd_table[fd_slot].file_index = -1;
    fd_table[fd_slot].position = 0;
    
    return 0;
}

// Read from a file
int fs_read(int fd, char *buf, int len) {
    if (fd < 3 || fd >= 3 + MAX_OPEN_FDS || !buf || len <= 0) {
        return -1;
    }
    
    int fd_slot = fd - 3;
    if (!fd_table[fd_slot].in_use) {
        return -1;  // FD not open
    }
    
    if (!(fd_table[fd_slot].flags & FD_READ)) {
        return -1;  // Not opened for reading
    }
    
    int file_idx = fd_table[fd_slot].file_index;
    if (file_idx < 0 || !files[file_idx].in_use) {
        return -1;
    }
    
    file_t *file = &files[file_idx];
    int pos = fd_table[fd_slot].position;
    int remaining = file->size - pos;
    int to_read = len < remaining ? len : remaining;
    
    if (to_read <= 0) {
        return 0;  // EOF
    }
    
    memcpy(buf, file->data + pos, to_read);
    fd_table[fd_slot].position += to_read;
    
    return to_read;
}

// Write to a file
int fs_write(int fd, const char *buf, int len) {
    if (fd < 3 || fd >= 3 + MAX_OPEN_FDS || !buf || len <= 0) {
        return -1;
    }
    
    int fd_slot = fd - 3;
    if (!fd_table[fd_slot].in_use) {
        return -1;  // FD not open
    }
    
    if (!(fd_table[fd_slot].flags & FD_WRITE)) {
        return -1;  // Not opened for writing
    }
    
    int file_idx = fd_table[fd_slot].file_index;
    if (file_idx < 0 || !files[file_idx].in_use) {
        return -1;
    }
    
    file_t *file = &files[file_idx];
    
    // Don't allow writing to embedded (read-only) files
    if (file->is_embedded) {
        return -1;  // Cannot write to embedded file
    }
    int pos = fd_table[fd_slot].position;
    int remaining = file->capacity - pos;
    int to_write = len < remaining ? len : remaining;
    
    if (to_write <= 0) {
        return -1;  // No space
    }
    
    memcpy(file->data + pos, buf, to_write);
    fd_table[fd_slot].position += to_write;
    
    // Update file size if we wrote past the end
    if (pos + to_write > file->size) {
        file->size = pos + to_write;
    }
    
    return to_write;
}

// Seek in a file
int fs_seek(int fd, int offset) {
    if (fd < 3 || fd >= 3 + MAX_OPEN_FDS) {
        return -1;
    }
    
    int fd_slot = fd - 3;
    if (!fd_table[fd_slot].in_use) {
        return -1;
    }
    
    int file_idx = fd_table[fd_slot].file_index;
    if (file_idx < 0 || !files[file_idx].in_use) {
        return -1;
    }
    
    file_t *file = &files[file_idx];
    int new_pos = offset;
    
    if (new_pos < 0) new_pos = 0;
    if (new_pos > file->size) new_pos = file->size;
    
    fd_table[fd_slot].position = new_pos;
    return new_pos;
}

// List all files
int fs_list_files(char *buf, int maxlen) {
    int pos = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].in_use) {
            int name_len = strlen(files[i].name);
            int size_str_len = 10;  // Enough for size string
            int total_len = name_len + size_str_len + 5;  // "name (size)\n"
            
            if (pos + total_len >= maxlen) break;
            
            // Format: "filename (size)\n"
            memcpy(buf + pos, files[i].name, name_len);
            pos += name_len;
            buf[pos++] = ' ';
            buf[pos++] = '(';
            
            // Convert size to string (simple implementation)
            int size = files[i].size;
            char size_buf[16];
            int size_pos = 0;
            if (size == 0) {
                size_buf[size_pos++] = '0';
            } else {
                char temp[16];
                int temp_pos = 0;
                while (size > 0) {
                    temp[temp_pos++] = '0' + (size % 10);
                    size /= 10;
                }
                for (int j = temp_pos - 1; j >= 0; j--) {
                    size_buf[size_pos++] = temp[j];
                }
            }
            size_buf[size_pos] = 0;
            
            memcpy(buf + pos, size_buf, size_pos);
            pos += size_pos;
            buf[pos++] = ')';
            buf[pos++] = '\n';
        }
    }
    if (pos < maxlen) buf[pos] = 0;
    return pos;
}

// Get file size by name
int fs_get_file_size(const char *name) {
    int idx = find_file(name);
    if (idx < 0) {
        return -1;
    }
    return files[idx].size;
}

// Legacy compatibility: get file content (for backward compatibility)
const char* fs_get_file_content(const char *name, int *len) {
    int idx = find_file(name);
    if (idx < 0) {
        *len = 0;
        return 0;
    }
    *len = files[idx].size;
    return files[idx].data;
}
