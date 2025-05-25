#ifndef MAP_SIMPLE_H

#include <stddef.h>
#include <stdbool.h>

/* header stuff {{{ */

typedef size_t (*MapHash)(void *);
typedef int (*MapCmp)(void *, void *);
typedef void (*MapFree)(void *);

typedef struct MapMeta {
    size_t hash;
    void *val;
    void *key;
} MapMeta;

/*}}}*/

/* debug optimization {{{ */

#if defined(NDEBUG)
#define MAP_DEBUG_INFO
#define MAP_DEBUG_ARG
#define MAP_DEBUG_ARGS
#define MAP_DEBUG_DEF
#define MAP_DEBUG_DEFS
#define MAP_DEBUG_FMT
#else
#define MAP_DEBUG_INFO  , __FILE__, __LINE__, __func__
#define MAP_DEBUG_ARG   file, line, func
#define MAP_DEBUG_ARGS  , MAP_DEBUG_ARG
#define MAP_DEBUG_DEF   const char *file, const int line, const char *func
#define MAP_DEBUG_DEFS  , MAP_DEBUG_DEF
#define MAP_DEBUG_FMT   "%s:%u:%s() "
#endif

/*}}}*/

/* available functions + utility {{{ */

size_t map_len(void *map);
size_t map_cap(void *map);
void map_clear(void *map);

#define map_config_key(map, key_type, key_cmp, key_hash, key_free)  _map_config_key(&map MAP_DEBUG_INFO, sizeof(key_type), key_cmp, key_hash, key_free)
#define map_config_val(map, val_free)                               _map_config_val(&map MAP_DEBUG_INFO, val_free)

#define map_set(map, key, val)  (*(typeof(map))(_map_set(&map MAP_DEBUG_INFO, sizeof(*map), (void *)key)) = val)
#define map_get(map, key)       (typeof(map))(_map_get(map, (void *)key))
#define map_del(map, key)       (_map_del(map, (void *)key))
#define map_free(map)           _map_free(&map)

#define map_it_all(map, out_key, out_val)   \
        for(MapMeta *it = _map_it_next(map, 0); \
            it, \
                it ? ((out_key = *(typeof(out_key) *)it->key)) || true : 0, \
                it ? ((out_val = *(typeof(out_val) *)it->val)) || true : 0; \
            it = _map_it_next(map, it) \
        ) \

/*}}}*/

/* actual functions, probably don't use directly {{{ */

void _map_config_key(void *map MAP_DEBUG_DEFS, size_t key_size, MapCmp key_cmp, MapHash key_hash, MapFree key_free);
void _map_config_val(void *map MAP_DEBUG_DEFS, MapFree val_free);

void *_map_set(void *map MAP_DEBUG_DEFS, size_t size_val, void *key);
void *_map_get(void *map, void *key);
MapMeta *_map_it_next(void *map, MapMeta *prev);

void _map_del(void *map, void *key);
void _map_free(void *map);

/*}}}*/


#define MAP_SIMPLE_H
#endif

