#ifndef SC_DAEMON_H
#define SC_DAEMON_H

#include "core/error.h"
#include <stdbool.h>

/**
 * Start the daemon (fork/daemonize), write PID to ~/.seaclaw/seaclaw.pid.
 * On non-Unix: returns SC_ERR_NOT_SUPPORTED.
 * In SC_IS_TEST mode: skip actual fork, just validate args.
 */
sc_error_t sc_daemon_start(void);

/**
 * Stop the daemon: read PID file, send SIGTERM.
 */
sc_error_t sc_daemon_stop(void);

/**
 * Check if the daemon PID is still running.
 */
bool sc_daemon_status(void);

#endif /* SC_DAEMON_H */
