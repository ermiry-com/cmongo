#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <cmongo/select.h>

#include "test.h"

static void test_select_create_single (void) {

	CMongoSelect *select = cmongo_select_new ();
	
	test_check_ptr (select);
	test_check_int_eq (select->n_fields, 0, NULL);
	
	cmongo_select_insert_field (select, "name");
	
	test_check_int_eq (select->n_fields, 1, NULL);
	test_check_str_eq (select->fields[0].value, "name", NULL);
	test_check_str_len (select->fields[0].value, strlen ("name"), NULL);

	cmongo_select_delete (select);

}

static void test_select_create_multiple (void) {
	
	CMongoSelect *select = cmongo_select_new ();

	test_check_ptr (select);
	test_check_int_eq (select->n_fields, 0, NULL);
	
	cmongo_select_insert_field (select, "name");
	
	test_check_int_eq (select->n_fields, 1, NULL);
	test_check_str_eq (select->fields[0].value, "name", NULL);
	test_check_str_len (select->fields[0].value, strlen ("name"), NULL);

	cmongo_select_insert_field (select, "email");
	
	test_check_int_eq (select->n_fields, 2, NULL);
	test_check_str_eq (select->fields[1].value, "email", NULL);
	test_check_str_len (select->fields[1].value, strlen ("email"), NULL);

	cmongo_select_insert_field (select, "username");
	
	test_check_int_eq (select->n_fields, 3, NULL);
	test_check_str_eq (select->fields[2].value, "username", NULL);
	test_check_str_len (select->fields[2].value, strlen ("username"), NULL);
	
	cmongo_select_delete (select);

}

int main (int argc, char **argv) {

	(void) printf ("Testing SELECT...\n");

	test_select_create_single ();

	test_select_create_multiple ();

	(void) printf ("\nDone with SELECT tests!\n\n");

	return 0;

}