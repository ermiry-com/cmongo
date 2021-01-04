#ifndef _CMONGO_SELECT_H_
#define _CMONGO_SELECT_H_

#include <stddef.h>

#include "cmongo/config.h"

#define CMONGO_SELECT_FIELD_LEN			256

typedef struct CMongoSelectField {

	struct CMongoSelectField *prev;

	char value[CMONGO_SELECT_FIELD_LEN];
	size_t len;

	struct CMongoSelectField *next;

} CMongoSelectField;

CMONGO_EXPORT CMongoSelectField *cmongo_select_field_create (
	const char *value
);

typedef struct CMongoSelect {

	size_t size;

	CMongoSelectField *start;
	CMongoSelectField *end;

} CMongoSelect;

CMONGO_EXPORT CMongoSelect *cmongo_select_new (void);

CMONGO_EXPORT bool cmongo_select_is_empty (
	const CMongoSelect *select
);

CMONGO_EXPORT bool cmongo_select_is_not_empty (
	const CMongoSelect *select
);

#endif