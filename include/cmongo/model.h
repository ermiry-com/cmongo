#ifndef _CMONGO_MODELS_H_
#define _CMONGO_MODELS_H_

#include <stddef.h>

#include <bson/bson.h>

#include "cmongo/config.h"

#define CMONGO_COLLNAME_SIZE		128

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*mongo_parser)(void *model, const bson_t *doc);

typedef struct CMongoModel {

	size_t collname_len;
	char collname[CMONGO_COLLNAME_SIZE];

	mongo_parser model_parser;

} CMongoModel;

CMONGO_PUBLIC CMongoModel *cmongo_model_new (void);

CMONGO_PUBLIC void cmongo_model_delete (void *model_ptr);

CMONGO_EXPORT CMongoModel *cmongo_model_create (
	const char *collname
);

CMONGO_EXPORT void cmongo_model_set_parser (
	CMongoModel *model, const mongo_parser model_parser
);

#ifdef __cplusplus
}
#endif

#endif