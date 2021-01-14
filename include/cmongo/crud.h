#ifndef _CMONGO_CRUD_H_
#define _CMONGO_CRUD_H_

#include <mongoc/mongoc.h>
#include <bson/bson.h>

#include "cmongo/select.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*mongo_parser)(void *model, const bson_t *doc);

// counts the docs in a collection by a matching query
extern int64_t mongo_count_docs (
	mongoc_collection_t *collection, bson_t *query
);

// returns true if 1 or more documents matches the query, false if no matches
extern bool mongo_check (
	mongoc_collection_t *collection, bson_t *query
);

// generates an opts doc that can be used to better work with find methods
// primarily used to query with projection (select) options
extern bson_t *mongo_find_generate_opts (
	const CMongoSelect *select
);

// use a query to find all matching documents
// select is a dlist of strings used for document projection,
// _id is true by default and should not be incldued
// returns a cursor (should be destroyed) that can be used to traverse the matching documents
// query gets destroyed, select list remains the same
extern mongoc_cursor_t *mongo_find_all_cursor (
	mongoc_collection_t *collection, 
	bson_t *query, const CMongoSelect *select,
	uint64_t *n_docs
);

// uses a query to find all matching docs with the specified options
// query gets destroyed, options remain the same
extern mongoc_cursor_t *mongo_find_all_cursor_with_opts (
	mongoc_collection_t *collection, 
	bson_t *query, const bson_t *opts
);

// use a query to find all matching documents
// an empty query will return all the docs in a collection
extern const bson_t **mongo_find_all (
	mongoc_collection_t *collection, 
	bson_t *query, const CMongoSelect *select,
	uint64_t *n_docs
);

// correctly destroys an array of docs got from mongo_find_all ()
extern void mongo_find_all_destroy_docs (
	bson_t **docs, uint64_t count
);

// uses a query to find one doc with the specified options
// query gets destroyed, opts are kept the same
// returns 0 on success, 1 on error
extern unsigned int mongo_find_one_with_opts (
	mongoc_collection_t *collection,
	bson_t *query, const bson_t *opts,
	void *model, const mongo_parser model_parser
);

// uses a query to find one doc
// select is a dlist of strings used for document projection,
// _id is true by default and should not be incldued
// query gets destroyed, select structure remains the same
// returns 0 on success, 1 on error
extern unsigned int mongo_find_one (
	mongoc_collection_t *collection,
	bson_t *query, const CMongoSelect *select,
	void *model, const mongo_parser model_parser
);

// inserts a document into a collection
// destroys document
// returns 0 on success, 1 on error
extern int mongo_insert_one (
	mongoc_collection_t *collection, bson_t *doc
);

// inserts many documents into a collection
// docs are NOT deleted after the operation
// returns 0 on success, 1 on error
extern int mongo_insert_many (
	mongoc_collection_t *collection,
	const bson_t **docs, size_t n_docs
);

// updates a doc by a matching query with the new values
// destroys query and update documents
// returns 0 on success, 1 on error
extern int mongo_update_one (
	mongoc_collection_t *collection,
	bson_t *query, bson_t *update
);

// updates all the query matching documents
// destroys the query and the update documents
// returns 0 on success, 1 on error
extern int mongo_update_many (
	mongoc_collection_t *collection,
	bson_t *query, bson_t *update
);

// deletes one matching document by a query
// destroys the query document
// returns 0 on success, 1 on error
extern int mongo_delete_one (
	mongoc_collection_t *collection, bson_t *query
);

// deletes all the query matching documents
// destroys the query
// returns 0 on success, 1 on error
extern int mongo_delete_many (
	mongoc_collection_t *collection, bson_t *query
);

#ifdef __cplusplus
}
#endif

#endif