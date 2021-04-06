#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <bson/bson.h>

#include "cmongo/types.h"

bson_oid_t *bson_oid_new (void) {

	bson_oid_t *oid = (bson_oid_t *) malloc (sizeof (bson_oid_t));
	if (oid) (void) memset (oid, 0, sizeof (bson_oid_t));
	return oid;

}

void bson_oid_delete (void *bson_oid_t_ptr) {

	if (bson_oid_t_ptr) free (bson_oid_t_ptr);

}

bson_oid_t *bson_oid_create (const bson_oid_t *original_oid) {

	bson_oid_t *oid = (bson_oid_t *) malloc (sizeof (bson_oid_t));
	if (oid) {
		bson_oid_copy (original_oid, oid);
	}

	return oid;

}

void bson_oid_print (const bson_oid_t *oid) {

	char buffer[BSON_OID_BUFFER_SIZE] = { 0 };
	bson_oid_to_string (oid, buffer);
	(void) printf ("%s\n", buffer);

}

void bson_print_as_json (const bson_t *document) {

	size_t json_len = 0;
	char *json = bson_as_relaxed_extended_json (document, &json_len);
	if (json) {
		(void) printf ("%s\n", json);
		free (json);
	}

}