#ifndef _CMONGO_COLLECTIONS_H_
#define _CMONGO_COLLECTIONS_H_

#include <mongoc/mongoc.h>

#include "cmongo/mongo.h"

#ifdef __cplusplus
extern "C" {
#endif

// drops a collection deleting all of its data
// retuns 0 on success, 1 on error
CMONGO_EXPORT int mongo_collection_drop (
	mongoc_collection_t *collection
);

#ifdef __cplusplus
}
#endif

#endif