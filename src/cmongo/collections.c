#include <stdlib.h>

#include <mongoc/mongoc.h>

#include <clibs/utils/log.h>

#include "cmongo/mongo.h"

// opens handle to a mongo collection in the db
mongoc_collection_t *mongo_collection_get (const char *coll_name) {

	return mongoc_client_get_collection (client, db_name->str, coll_name);

}

// drops a collection deleting all of its data
// retuns 0 on success, 1 on error
int mongo_collection_drop (mongoc_collection_t *collection) {

	int retval = 1;

	bson_error_t error;
	if (mongoc_collection_drop (collection, &error)) {
		retval = 0;
	}

	else {
		clibs_log_error ("Failed to drop collection - %s", error.message);
	}

	return retval;

}