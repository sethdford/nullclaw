#ifndef SC_TOOLS_PDF_H
#define SC_TOOLS_PDF_H

#include "seaclaw/core/allocator.h"
#include "seaclaw/core/error.h"
#include "seaclaw/tool.h"

sc_error_t sc_pdf_create(sc_allocator_t *alloc, sc_tool_t *out);

#endif /* SC_TOOLS_PDF_H */
