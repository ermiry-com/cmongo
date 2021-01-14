#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "cmongo/select.h"

CMongoSelect *cmongo_select_new (void) {

	CMongoSelect *select = (CMongoSelect *) malloc (sizeof (CMongoSelect));
	if (select) {
		(void) memset (select, 0, sizeof (CMongoSelect));
	}

	return select;

}

// adds a new field to the select list
// returns 0 on success, 1 on error
int cmongo_select_insert_field (
	CMongoSelect *select, const char *field
) {

	int retval = 1;

	if (select && field) {
		if (select->n_fields < CMONGO_SELECT_FIELDS_SIZE) {
			(void) strncpy (
				select->fields[select->n_fields].value,
				field,
				CMONGO_SELECT_FIELD_LEN - 1
			);

			select->fields[select->n_fields].len = strlen (
				select->fields[select->n_fields].value
			);

			select->n_fields += 1;
			
			retval = 0;
		}
	}

	return retval;

}

void cmongo_select_delete (void *cmongo_select_ptr) {

	if (cmongo_select_ptr) {
		free (cmongo_select_ptr);
	}

}

#pragma endregion