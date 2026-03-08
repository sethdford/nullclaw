#ifndef SC_DAG_H
#define SC_DAG_H

#include "seaclaw/core/allocator.h"
#include "seaclaw/core/error.h"
#include <stdbool.h>
#include <stddef.h>

#define SC_DAG_MAX_NODES 32
#define SC_DAG_MAX_DEPS   8

typedef enum sc_dag_node_status {
    SC_DAG_PENDING,
    SC_DAG_READY,
    SC_DAG_RUNNING,
    SC_DAG_DONE,
    SC_DAG_FAILED,
} sc_dag_node_status_t;

typedef struct sc_dag_node {
    char *id;
    char *tool_name;
    char *args_json;
    char *deps[SC_DAG_MAX_DEPS];
    size_t dep_count;
    sc_dag_node_status_t status;
    char *result;
    size_t result_len;
} sc_dag_node_t;

typedef struct sc_dag {
    sc_allocator_t alloc;
    sc_dag_node_t nodes[SC_DAG_MAX_NODES];
    size_t node_count;
} sc_dag_t;

sc_error_t sc_dag_init(sc_dag_t *dag, sc_allocator_t alloc);
sc_error_t sc_dag_add_node(sc_dag_t *dag, const char *id, const char *tool_name,
                           const char *args_json, const char **deps, size_t dep_count);
sc_error_t sc_dag_validate(const sc_dag_t *dag);
sc_error_t sc_dag_parse_json(sc_dag_t *dag, sc_allocator_t *alloc, const char *json,
                             size_t json_len);
bool sc_dag_is_complete(const sc_dag_t *dag);
sc_dag_node_t *sc_dag_find_node(sc_dag_t *dag, const char *id, size_t id_len);
void sc_dag_deinit(sc_dag_t *dag);

#endif
