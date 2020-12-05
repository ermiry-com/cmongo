#ifndef _CMONGO_COLLECTIONS_H_
#define _CMONGO_COLLECTIONS_H_

#include <mongoc/mongoc.h>

// opens handle to a mongo collection in the db
extern mongoc_collection_t *mongo_collection_get (const char *coll_name);

// drops a collection deleting all of its data
// retuns 0 on success, 1 on error
extern int mongo_collection_drop (mongoc_collection_t *collection);

#endif