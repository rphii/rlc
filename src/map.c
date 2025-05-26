#include "map.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#define LUT_EMPTY               SIZE_MAX

#define map_get_at(l, index)    &l->data[index]
#define map_width_cap(width)    (!!width * (size_t)1ULL << width)

#define map_assert_arg(arg)     assert(arg && "null pointer argument!");
#define map_base(map)           ((map) - offsetof(Map, data))

#define map_must_exist(map)   do { \
        if(!map) { \
            map = map_init(MAP_DEBUG_ARG); \
        } \
    } while(0)

#define map_error(msg, ...)     do { \
        printf("\n" MAP_DEBUG_FMT "map error: " msg "\n" MAP_DEBUG_ARGS, ##__VA_ARGS__); \
        exit(1); \
    } while(0)

typedef struct Map {
    struct {
        size_t size;
        MapHash hash;
        MapCmp cmp;
        MapFree f;
    } key;
    struct {
        MapFree f;
    } val;
    size_t used;
    size_t width;
    MapMeta *data;
} Map;

static inline void *map_init(MAP_DEBUG_DEF) {
    Map *l = malloc(sizeof(Map));
    if(!l) {
        map_error("could not create map");
    }
    memset(l, 0, sizeof(*l));
    return &l->data;
}

void *mapmeta_key(MapMeta *meta) {
    map_assert_arg(meta);
    return meta->key;
}
void *mapmeta_val(MapMeta *meta) {
    map_assert_arg(meta);
    return meta->val;
}

size_t map_len(void *map) {
    if(!map) return 0;
    Map *l = map_base(map);
    return l->used;
}

size_t map_cap(void *map) {
    if(!map) return 0;
    Map *l = map_base(map);
    return map_width_cap(l->width);
}

void map_clear(void *map) {
    if(!map) return;
    Map *l = map_base(map);
    size_t cap = map_width_cap(l->width);
    for(size_t i = 0; i < cap; ++i) {
        l->data[i].key = 0;
        l->data[i].val = 0;
        l->data[i].hash = LUT_EMPTY;
    }
    l->used = 0;
}

void mapmeta_activate(Map *map, MapMeta *item, size_t size_val) {
    map_assert_arg(map);
    map_assert_arg(item);
    map_assert_arg(size_val);
    size_t cap = map_width_cap(map->width);
    size_t i = item - map->data;
    item->val = (void *)((unsigned char *)map->data + (sizeof(MapMeta) * cap) + (i * (map->key.size + size_val)));
    item->key = (void *)((unsigned char *)map->data + (sizeof(MapMeta) * cap) + (i * (map->key.size + size_val)) + size_val);
    ++map->used;
}

void mapmeta_deactivate(Map *map, MapMeta *item) {
    map_assert_arg(map);
    map_assert_arg(item);
    if(item->hash != LUT_EMPTY) --map->used;
    item->hash = LUT_EMPTY;
    item->key = 0;
    item->val = 0;
}

void mapmeta_free(Map *map, MapMeta *item) {
    map_assert_arg(map);
    map_assert_arg(item);
    /* wow, this is cursed XD */
    if(item->key && map->key.f) map->key.f(item->key);
    if(item->val && map->val.f) map->val.f(item->val);
    mapmeta_deactivate(map, item);
}

static MapMeta *_map_get_item(Map *map, void *key, size_t hash, bool intend_to_set) {
    map_assert_arg(map);
    if(!intend_to_set && !map->used) return 0;
    size_t perturb = hash >> 5;
    size_t mask = ~(SIZE_MAX << map->width);
    size_t i = mask & hash;
    MapMeta *item = map_get_at(map, i);
    for(size_t n = 1; n < map->width; ++n) {
        if(intend_to_set && item->hash == LUT_EMPTY) break;
        if(item->hash == hash) {
            void *meta_key = mapmeta_key(item);
            //uintptr_t meta_key_real = *(uintptr_t *)meta_key;
            if(!map->key.cmp(meta_key, key)) {
                return item;
            }
        }
        perturb >>= 5;
        i = mask & (i * 5 + perturb + 1);
        /* get NEXT item */
        item = map_get_at(map, i);
    }
    return item;
}

static void *_map_grow2(void *map MAP_DEBUG_DEFS, size_t size_key, size_t size_val, size_t width) {
    Map zero = {0};
    Map *l = map ? map_base(map) : &zero;
    if(map && width <= l->width) return map;
    size_t cap_old = map_width_cap(l->width);
    size_t cap = map_width_cap(width);
    if(cap * 2 < cap) {
        map_error("invalid allocation size: %zu", cap);
    }
    size_t bytes = sizeof(Map) + (sizeof(MapMeta) + size_key + size_val) * cap;
    if((bytes - sizeof(Map)) / (sizeof(MapMeta) + size_key + size_val) != cap) {
        map_error("failed allocation of: %zu elements (%zu bytes)", cap, bytes);
    }
    Map *grown = malloc(bytes);
    if(!grown) {
        map_error("could not allocate map");
    }
    memcpy(grown, l, sizeof(*grown));
    grown->used = 0; /* will be increased with activate */
    grown->width = width;
    grown->key.size = size_key;
    grown->data = (MapMeta *)((unsigned char *)grown + sizeof(Map));
    /* re-add values */
    for(size_t i = 0; i < cap; ++i) {
        MapMeta *item = map_get_at(grown, i);
        item->hash = LUT_EMPTY;
        item->key = 0;
        item->val = 0;
    }
    if(map) {
        for(size_t i = 0; i < cap_old; ++i) {
            MapMeta *src = map_get_at(l, i);
            if(src->hash == LUT_EMPTY) continue;
            size_t hash = src->hash;
            MapMeta *item = _map_get_item(grown, mapmeta_key(src), hash, true);
            item->hash = src->hash;
            mapmeta_activate(grown, item, size_val);
            memcpy(item->val, src->val, size_val);
            memcpy(item->key, src->key, size_key);
        }
        free(l);
    }
    return &grown->data;
}

void _map_config_key(void *map MAP_DEBUG_DEFS, size_t key_size, MapCmp key_cmp, MapHash key_hash, MapFree key_free) {
    map_assert_arg(map);
    if(!key_size) {
        map_error("key size cannot be 0");
    }
    if(!key_cmp) {
        map_error("invalid key comparison function");
    }
    if(!key_hash) {
        map_error("invalid key hashing function");
    }
    void **p = map;
    map_must_exist(*p);
    Map *l = map_base(*p);
    l->key.hash = key_hash;
    l->key.cmp = key_cmp;
    l->key.size = key_size;
    l->key.f = key_free;
}

void _map_config_val(void *map MAP_DEBUG_DEFS, MapFree val_free) {
    map_assert_arg(map);
    void **p = map;
    map_must_exist(*p);
    Map *l = map_base(*p);
    l->val.f = val_free;
}

void *_map_set(void *map MAP_DEBUG_DEFS, size_t size_val, void *key, size_t size_key, void *val) {
    map_assert_arg(map);
    void **p = map;
    map_must_exist(*p);
    Map *l = map_base(*p);
    if(!l->key.size) {
        map_error("cannot have a key size of zero");
    }
    size_t size = size_val + l->key.size + sizeof(MapMeta);
    if(3 * l->used / 2 >= map_width_cap(l->width)) {
        *p = _map_grow2(*p MAP_DEBUG_ARGS, l->key.size, size_val, l->width + 2);
    }
    l = map_base(*p);
    size_t hash = l->key.hash(key) % LUT_EMPTY;
    MapMeta *item = _map_get_item(l, key, hash, true);
    mapmeta_free(l, item);
    mapmeta_activate(l, item, size_val);
    item->hash = hash;
    void *meta_key = mapmeta_key(item);
    memcpy(meta_key, key, l->key.size);
    void *meta_val = mapmeta_val(item);
    memcpy(meta_val, val, size_val);
    return meta_val;
}

void *_map_once(void *map MAP_DEBUG_DEFS, size_t size_val, void *key, size_t size_key, void *val) {
    map_assert_arg(map);
    if(_map_get(map, key)) {
        return 0;
    }
    return _map_set(map MAP_DEBUG_ARGS, size_val, key, size_key, val);
}

MapMeta *_map_it_next(void *map, MapMeta *prev) {
    if(!map) return 0;
    Map *l = map_base(map);
    size_t i = prev ? (prev - l->data) + 1 : 0;
    if(i >= map_width_cap(l->width)) {
        return 0;
    }
    size_t cap = map_width_cap(l->width);
    do {
        if(l->data[i].hash == LUT_EMPTY) continue;
        return &l->data[i];
    } while(++i < cap);
    return 0;
}

void *_map_get(void *map, void *key) {
    if(!map) return 0;
    Map *l = map_base(map);
    MapMeta *got = _map_get_item(l, key, l->key.hash(key), false);
    return got->val;
}

void _map_del(void *map, void *key) {
    if(!map) return;
    Map *l = map_base(map);
    MapMeta *item = _map_get_item(l, key, l->key.hash(key), false);
    if(item) {
        item->hash = LUT_EMPTY;
        --l->used;
    }
}

void _map_free(void *map) {
    map_assert_arg(map);
    void **p = map;
    if(!*p) return;
    Map *l = map_base(*p);
    if(l->key.f || l->val.f) {
        for(size_t i = 0; i < map_width_cap(l->width); ++i) {
            MapMeta *item = &l->data[i];
            mapmeta_free(l, item);
        }
    }
    free(l);
    *p = 0;
}

