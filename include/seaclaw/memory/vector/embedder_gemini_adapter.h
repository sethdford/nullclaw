#ifndef SC_MEMORY_VECTOR_EMBEDDER_GEMINI_ADAPTER_H
#define SC_MEMORY_VECTOR_EMBEDDER_GEMINI_ADAPTER_H

#include "seaclaw/core/allocator.h"
#include "seaclaw/memory/vector.h"
#include "seaclaw/memory/vector/embeddings.h"

/* Create an sc_embedder_t that wraps a Gemini embedding provider.
 * The adapter forwards embed/embed_batch/dimensions/deinit to the provider.
 * embed_batch is implemented by looping over embed (Gemini has no native batch). */
sc_embedder_t sc_embedder_gemini_adapter_create(sc_allocator_t *alloc,
                                                 sc_embedding_provider_t provider);

#endif /* SC_MEMORY_VECTOR_EMBEDDER_GEMINI_ADAPTER_H */
