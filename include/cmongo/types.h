#ifndef _CMONGO_TYPES_H_
#define _CMONGO_TYPES_H_

#include <bson/bson.h>

extern bson_oid_t *bson_oid_new (void);

extern void bson_oid_delete (void *bson_oid_t_ptr);

extern bson_oid_t *bson_oid_create (const bson_oid_t *original_oid);

#endif