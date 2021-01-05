#ifndef _CMONGO_SELECT_H_
#define _CMONGO_SELECT_H_

#include <stddef.h>

#include "cmongo/config.h"

#define CMONGO_SELECT_FIELD_LEN			256

#ifdef __cplusplus
extern "C" {
#endif

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

#define cmongo_select_for_each(select)						\
	for (CMongoSelectField *field = select->start; field; field = field->next)

#define cmongo_select_for_each_backwards(select)			\
	for (CMongoSelectField *field = select->end; field; field = field->prev)

CMONGO_EXPORT CMongoSelect *cmongo_select_new (void);

CMONGO_EXPORT bool cmongo_select_is_empty (
	const CMongoSelect *select
);

CMONGO_EXPORT bool cmongo_select_is_not_empty (
	const CMongoSelect *select
);

// adds a new field to the select list
// returns 0 on success, 1 on error
CMONGO_EXPORT int cmongo_select_insert_field (
	CMongoSelect *select,
	CMongoSelectField *field
);

CMONGO_EXPORT void cmongo_select_delete (
	void *cmongo_select_ptr
);

#ifdef __cplusplus
}
#endif

#endif