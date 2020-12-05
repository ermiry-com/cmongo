#include <stdlib.h>
#include <string.h>

#include <bson/bson.h>

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