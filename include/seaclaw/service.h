#ifndef SC_SERVICE_H
#define SC_SERVICE_H

#include "seaclaw/core/error.h"

/* Background service lifecycle. Stub. */
sc_error_t sc_service_start(void);
void sc_service_stop(void);

#endif /* SC_SERVICE_H */
