#ifndef SC_SECURITY_VAULT_H
#define SC_SECURITY_VAULT_H

#include "seaclaw/core/allocator.h"
#include "seaclaw/core/error.h"
#include <stddef.h>

typedef struct sc_vault sc_vault_t;

/* Create a vault that stores secrets encrypted at rest.
 * vault_path: path to JSON file (default ~/.seaclaw/vault.json if NULL). */
sc_vault_t *sc_vault_create(sc_allocator_t *alloc, const char *vault_path);

/* Store a secret (encrypted with XOR using key from SEACLAW_VAULT_KEY, or base64 if no key). */
sc_error_t sc_vault_set(sc_vault_t *vault, const char *key, const char *value);

/* Retrieve a secret (decrypted). out_size includes null terminator. */
sc_error_t sc_vault_get(sc_vault_t *vault, const char *key, char *out, size_t out_size);

/* Delete a secret. */
sc_error_t sc_vault_delete(sc_vault_t *vault, const char *key);

/* List all secret keys (not values). Caller allocates keys array; count set on return. */
sc_error_t sc_vault_list_keys(sc_vault_t *vault, char **keys, size_t max_keys, size_t *count);

/* Destroy vault (zeroes sensitive memory). */
void sc_vault_destroy(sc_vault_t *vault);

/* Convenience: get API key for provider. Checks: vault <provider>_api_key, env <PROVIDER>_API_KEY,
 * config api_key. config_api_key can be NULL. out must be freed by caller. */
sc_error_t sc_vault_get_api_key(sc_vault_t *vault, sc_allocator_t *alloc, const char *provider_name,
                                const char *config_api_key, char **out);

#endif /* SC_SECURITY_VAULT_H */
