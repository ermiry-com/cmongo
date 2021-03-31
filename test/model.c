#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <cmongo/model.h>

#include "test.h"

static const char *collname = { "users" };

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

static void test_model_parser (
	void *model, const bson_t *doc
) {

	// nothing to be done here

}

#pragma GCC diagnostic pop

static void test_model_new_single (void) {

	CMongoModel *model = cmongo_model_new ();

	test_check_unsigned_eq (model->collname_len, 0, NULL);
	test_check_null_ptr (model->model_parser);

	cmongo_model_delete (model);

}

static void test_model_create_single (void) {

	CMongoModel *model = cmongo_model_create (collname);

	test_check_unsigned_eq (model->collname_len, strlen (collname), NULL);
	test_check_str_eq (model->collname, collname, NULL);
	test_check_null_ptr (model->model_parser);

	cmongo_model_delete (model);

}

static void test_model_set_parser (void) {

	CMongoModel *model = cmongo_model_create (collname);

	test_check_unsigned_eq (model->collname_len, strlen (collname), NULL);
	test_check_str_eq (model->collname, collname, NULL);
	test_check_null_ptr (model->model_parser);

	cmongo_model_set_parser (model, test_model_parser);
	test_check_ptr (model->model_parser);

	cmongo_model_delete (model);

}

int main (int argc, char **argv) {

	(void) printf ("Testing MODEL...\n");

	test_model_new_single ();

	test_model_create_single ();

	test_model_set_parser ();

	(void) printf ("\nDone with MODEL tests!\n\n");

	return 0;

}