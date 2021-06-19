#include <stdlib.h>
#include <string.h>

#include "cmongo/model.h"

CMongoModel *cmongo_model_new (void) {

	CMongoModel *model = (CMongoModel *) malloc (sizeof (CMongoModel));
	if (model) {
		(void) memset (model, 0, sizeof (CMongoModel));
	}

	return model;

}

void cmongo_model_delete (void *model_ptr) {

	if (model_ptr) free (model_ptr);

}

CMongoModel *cmongo_model_create (const char *collname) {

	CMongoModel *model = cmongo_model_new ();
	if (model && collname) {
		(void) strncpy (model->collname, collname, CMONGO_COLLNAME_SIZE - 1);
		model->collname_len = strlen (model->collname);
	}

	return model;

}

void cmongo_model_set_parser (
	CMongoModel *model, const mongo_parser model_parser
) {

	if (model) {
		model->model_parser = model_parser;
	}

}