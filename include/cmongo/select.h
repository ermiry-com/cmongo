#ifndef _CMONGO_SELECT_H_
#define _CMONGO_SELECT_H_

#include <stddef.h>

#include "cmongo/config.h"

#define CMONGO_SELECT_FIELD_LEN			256
#define CMONGO_SELECT_FIELDS_SIZE		32

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CMongoSelectField {

	size_t len;
	char value[CMONGO_SELECT_FIELD_LEN];

} CMongoSelectField;

typedef struct CMongoSelect {

	size_t n_fields;
	CMongoSelectField fields[CMONGO_SELECT_FIELDS_SIZE];

} CMongoSelect;

CMONGO_EXPORT CMongoSelect *cmongo_select_new (void);

// adds a new field to the select list
// returns 0 on success, 1 on error
CMONGO_EXPORT int cmongo_select_insert_field (
	CMongoSelect *select, const char *field
);

CMONGO_EXPORT void cmongo_select_delete (
	void *cmongo_select_ptr
);

#ifdef __cplusplus
}
#endif

#endif