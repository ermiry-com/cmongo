#include <stdlib.h>
#include <stdio.h>

#include <mongoc/mongoc.h>

#include "cmongo/mongo.h"

// drops a collection deleting all of its data
// retuns 0 on success, 1 on error
int mongo_collection_drop (mongoc_collection_t *collection) {

	int retval = 1;

	bson_error_t error = { 0 };
	if (mongoc_collection_drop (collection, &error)) {
		retval = 0;
	}

	else {
		(void) fprintf (
			stderr,
			"[MONGO][ERROR]: Failed to drop collection - %s\n",
			error.message
		);
	}

	return retval;

}