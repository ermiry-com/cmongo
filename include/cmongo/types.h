#ifndef _CMONGO_TYPES_H_
#define _CMONGO_TYPES_H_

#include <bson/bson.h>

#include "cmongo/config.h"

#define BSON_ARRAY_BUFFER_SIZE		16

#ifdef __cplusplus
extern "C" {
#endif

CMONGO_PUBLIC bson_oid_t *bson_oid_new (void);

CMONGO_PUBLIC void bson_oid_delete (void *bson_oid_t_ptr);

CMONGO_PUBLIC bson_oid_t *bson_oid_create (
	const bson_oid_t *original_oid
);

#ifdef __cplusplus
}
#endif

#endif