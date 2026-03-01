#include "seaclaw/daemon.h"
#include "seaclaw/core/error.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if !defined(_WIN32) && !defined(__CYGWIN__)
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <errno.h>
#endif

#define SC_DAEMON_PID_DIR ".seaclaw"
#define SC_DAEMON_PID_FILE "seaclaw.pid"
#define SC_MAX_PATH 1024

static int get_pid_path(char *buf, size_t buf_size) {
    const char *home = getenv("HOME");
    if (!home) home = ".";
    return snprintf(buf, buf_size, "%s/%s/%s", home, SC_DAEMON_PID_DIR, SC_DAEMON_PID_FILE);
}

#ifdef SC_IS_TEST
sc_error_t sc_daemon_start(void) {
    char path[SC_MAX_PATH];
    int n = get_pid_path(path, sizeof(path));
    if (n <= 0 || (size_t)n >= sizeof(path)) return SC_ERR_INVALID_ARGUMENT;
    return SC_OK;
}

sc_error_t sc_daemon_stop(void) {
    return SC_OK;
}

bool sc_daemon_status(void) {
    return false;
}
#elif defined(_WIN32) || defined(__CYGWIN__)
sc_error_t sc_daemon_start(void) {
    return SC_ERR_NOT_SUPPORTED;
}

sc_error_t sc_daemon_stop(void) {
    return SC_ERR_NOT_SUPPORTED;
}

bool sc_daemon_status(void) {
    return false;
}
#else
sc_error_t sc_daemon_start(void) {
    char path[SC_MAX_PATH];
    int n = get_pid_path(path, sizeof(path));
    if (n <= 0 || (size_t)n >= sizeof(path)) return SC_ERR_INVALID_ARGUMENT;

    const char *home = getenv("HOME");
    if (!home) home = ".";
    char dir[SC_MAX_PATH];
    n = snprintf(dir, sizeof(dir), "%s/%s", home, SC_DAEMON_PID_DIR);
    if (n <= 0 || (size_t)n >= sizeof(dir)) return SC_ERR_IO;

    if (mkdir(dir, 0755) != 0 && errno != EEXIST) return SC_ERR_IO;

    pid_t pid = fork();
    if (pid < 0) return SC_ERR_IO;
    if (pid > 0) {
        FILE *f = fopen(path, "w");
        if (f) {
            fprintf(f, "%d\n", (int)pid);
            fclose(f);
        }
        return SC_OK;
    }

    setsid();
    chdir("/");
    freopen("/dev/null", "r", stdin);
    freopen("/dev/null", "w", stdout);
    freopen("/dev/null", "w", stderr);

    /* Child continues as daemon - caller would typically exec seaclaw gateway/agent */
    /* For now we just exit; real service would exec the main loop */
    _exit(0);
}

sc_error_t sc_daemon_stop(void) {
    char path[SC_MAX_PATH];
    int n = get_pid_path(path, sizeof(path));
    if (n <= 0 || (size_t)n >= sizeof(path)) return SC_ERR_INVALID_ARGUMENT;

    FILE *f = fopen(path, "r");
    if (!f) return SC_ERR_NOT_FOUND;

    int pid_val = 0;
    if (fscanf(f, "%d", &pid_val) != 1 || pid_val <= 0) {
        fclose(f);
        return SC_ERR_INVALID_ARGUMENT;
    }
    fclose(f);

    if (kill((pid_t)pid_val, SIGTERM) != 0) return SC_ERR_IO;

    remove(path);
    return SC_OK;
}

bool sc_daemon_status(void) {
    char path[SC_MAX_PATH];
    int n = get_pid_path(path, sizeof(path));
    if (n <= 0 || (size_t)n >= sizeof(path)) return false;

    FILE *f = fopen(path, "r");
    if (!f) return false;

    int pid_val = 0;
    int ok = (fscanf(f, "%d", &pid_val) == 1 && pid_val > 0);
    fclose(f);
    if (!ok) return false;

    return kill((pid_t)pid_val, 0) == 0;
}
#endif
