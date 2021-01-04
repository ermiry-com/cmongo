#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "cmongo/select.h"

#pragma region internal

static CMongoSelectField *cmongo_select_field_new (void) {

	CMongoSelectField *field = (CMongoSelectField *) malloc (sizeof (CMongoSelectField));
	if (field) {
		field->prev = NULL;

		(void) memset (field->value, 0, CMONGO_SELECT_FIELD_LEN);
		field->len = 0;

		field->next = NULL;
	}

	return field;

}

static void cmongo_select_field_delete (CMongoSelectField *field) {

	if (field) free (field);

}

#pragma endregion

#pragma region public

CMongoSelectField *cmongo_select_field_create (
	const char *value
) {

	CMongoSelectField *field = cmongo_select_field_new ();
	if (field && value) {
		(void) strncpy (field->value, value, CMONGO_SELECT_FIELD_LEN - 1);
		field->len = strlen (field->value);
	}

	return field;

}

CMongoSelect *cmongo_select_new (void) {

	CMongoSelect *select = (CMongoSelect *) malloc (sizeof (CMongoSelect));
	if (select) {
		select->size = 0;

		select->start = NULL;
		select->end = NULL;
	}

	return select;

}

bool cmongo_select_is_empty (const CMongoSelect *select) {

	return select ? (select->size == 0) : false;

}

bool cmongo_select_is_not_empty (const CMongoSelect *select) {

	return select ? (select->size > 0) : false;

}

#pragma endregion