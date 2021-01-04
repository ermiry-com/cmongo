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

static void cmongo_select_internal_insert_after (
	CMongoSelect *select,
	CMongoSelectField *element,
	CMongoSelectField *field
) {

	if (element == NULL) {
		if (select->size == 0) select->end = field;
		else select->start->prev = field;
	
		field->next = select->start;
		field->prev = NULL;
		select->start = field;
	}

	else {
		if (element->next == NULL) select->end = field;

		field->next = element->next;
		field->prev = element;
		element->next = field;
	}

	select->size++;

}

static CMongoSelectField *cmongo_select_internal_remove_element (
	CMongoSelect *select,
	CMongoSelectField *element
) {

	CMongoSelectField *old = NULL;

	if (element == NULL) {
		old = select->start;
		select->start = select->start->next;
		if (select->start != NULL) select->start->prev = NULL;
	}

	else {
		old = element;

		CMongoSelectField *prevElement = element->prev;
		CMongoSelectField *nextElement = element->next;

		if (prevElement != NULL && nextElement != NULL) {
			prevElement->next = nextElement;
			nextElement->prev = prevElement;
		}

		else {
			// we are at the start of the select
			if (prevElement == NULL) {
				if (nextElement != NULL) nextElement->prev = NULL;
				select->start = nextElement;
			}

			// we are at the end of the select
			if (nextElement == NULL) {
				if (prevElement != NULL) prevElement->next = NULL;
				select->end = prevElement;
			}
		}
	}

	return old;

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

// adds a new field to the select list
// returns 0 on success, 1 on error
int cmongo_select_insert_field (
	CMongoSelect *select,
	CMongoSelectField *field
) {

	int retval = 1;

	if (select && field) {
		cmongo_select_internal_insert_after (
			select, select->end, field
		);

		retval = 0;
	}

	return retval;

}

void cmongo_select_delete (void *cmongo_select_ptr) {

	if (cmongo_select_ptr) {
		CMongoSelect *select = (CMongoSelect *) cmongo_select_ptr;

		while (select->size > 0) {
			cmongo_select_field_delete (
				cmongo_select_internal_remove_element (
					select, NULL
				)
			);

			select->size--;

			if (select->size == 0) {
				select->start = NULL;
				select->end = NULL;
			}
		}

		free (select);
	}

}

#pragma endregion