#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <mongoc/mongoc.h>
#include <bson/bson.h>

#include "cmongo/crud.h"
#include "cmongo/select.h"

#ifdef __cplusplus
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

// counts the docs in a collection by a matching query
int64_t mongo_count_docs (
	mongoc_collection_t *collection, bson_t *query
) {

	int64_t retval = 0;

	if (collection && query) {
		bson_error_t error = { 0 };
		retval = mongoc_collection_count_documents (
			collection, query, NULL, NULL, NULL, &error
		);

		if (retval < 0) {
			(void) fprintf (
				stderr, "[MONGO][ERROR]: %s", error.message
			);

			retval = 0;
		}

		bson_destroy (query);
	}

	return retval;

}

// returns true if 1 or more documents matches the query
// returns false if no matches
bool mongo_check (
	mongoc_collection_t *collection, bson_t *query
) {

	bool retval = false;

	if (collection && query) {
		bson_error_t error = { 0 };
		retval = mongoc_collection_count_documents (
			collection, query, NULL, NULL, NULL, &error
		);

		bson_destroy (query);
	}

	return retval;

}

// generates an opts doc that can be used to better work with find methods
// primarily used to query with projection (select) options
bson_t *mongo_find_generate_opts (
	const CMongoSelect *select
) {

	bson_t *opts = bson_new ();

	if (opts) {
		// append projection
		if (select) {
			bson_t projection_doc = { 0 };
			(void) bson_append_document_begin (opts, "projection", -1, &projection_doc);

			(void) bson_append_bool (&projection_doc, "_id", -1, true);

			cmongo_select_for_each (select) {
				(void) bson_append_bool (
					&projection_doc, field->value, field->len, true
				);
			}

			(void) bson_append_document_end (opts, &projection_doc);
		}
	}

	return opts;

}

// use a query to find all matching documents
// select is a dlist of strings used for document projection,
// _id is true by default and should not be incldued
// returns a cursor (should be destroyed) that can be used to traverse the matching documents
// query gets destroyed, select list remains the same
mongoc_cursor_t *mongo_find_all_cursor (
	mongoc_collection_t *collection,
	bson_t *query, const CMongoSelect *select,
	uint64_t *n_docs
) {

	mongoc_cursor_t *cursor = NULL;
	*n_docs = 0;

	if (collection && query) {
		uint64_t count = mongo_count_docs (collection, bson_copy (query));
		if (count > 0) {
			bson_t *opts = mongo_find_generate_opts (select);

			cursor = mongoc_collection_find_with_opts (
				collection, query, opts, NULL
			);

			*n_docs = count;

			if (opts) bson_destroy (opts);
			bson_destroy (query);
		}
	}

	return cursor;

}

// uses a query to find all matching docs with the specified options
// query gets destroyed, options remain the same
mongoc_cursor_t *mongo_find_all_cursor_with_opts (
	mongoc_collection_t *collection,
	bson_t *query, const bson_t *opts
) {

	mongoc_cursor_t *cursor = NULL;

	if (collection && query) {
		cursor = mongoc_collection_find_with_opts (
			collection, query, opts, NULL
		);

		bson_destroy (query);
	}

	return cursor;

}

// use a query to find all matching documents
// an empty query will return all the docs in a collection
const bson_t **mongo_find_all (
	mongoc_collection_t *collection,
	bson_t *query, const CMongoSelect *select,
	uint64_t *n_docs
) {

	const bson_t **retval = NULL;
	*n_docs = 0;

	if (collection && query) {
		uint64_t count = mongo_count_docs (collection, bson_copy (query));
		if (count > 0) {
			retval = (const bson_t **) calloc (count, sizeof (bson_t *));
			for (uint64_t i = 0; i < count; i++) retval[i] = bson_new ();

			bson_t *opts = mongo_find_generate_opts (select);

			mongoc_cursor_t *cursor = mongoc_collection_find_with_opts (
				collection, query, opts, NULL
			);

			uint64_t i = 0;
			const bson_t *doc = NULL;
			while (mongoc_cursor_next (cursor, &doc)) {
				// add the matching doc into our retval array
				bson_copy_to (doc, (bson_t *) retval[i]);
				i++;
			}

			*n_docs = count;

			mongoc_cursor_destroy (cursor);

			if (opts) bson_destroy (opts);

			bson_destroy (query);
		}
	}

	return retval;

}

// correctly destroys an array of docs got from mongo_find_all ()
void mongo_find_all_destroy_docs (
	bson_t **docs, uint64_t count
) {

	if (docs) {
		for (uint64_t i = 0; i < count; i++)
			bson_destroy (docs[i]);

		free (docs);
	}

}

static unsigned int mongo_find_one_internal (
	mongoc_collection_t *collection,
	bson_t *query, const bson_t *opts,
	void *model, const mongo_parser model_parser
) {

	unsigned int retval = 1;

	mongoc_cursor_t *cursor = mongoc_collection_find_with_opts (
		collection, query, opts, NULL
	);

	if (cursor) {
		(void) mongoc_cursor_set_limit (cursor, 1);

		const bson_t *doc = NULL;
		if (mongoc_cursor_next (cursor, &doc)) {
			model_parser (model, doc);
			retval = 0;
		}

		mongoc_cursor_destroy (cursor);
	}

	return retval;

}

// uses a query to find one doc with the specified options
// query gets destroyed, opts are kept the same
// returns 0 on success, 1 on error
unsigned int mongo_find_one_with_opts (
	mongoc_collection_t *collection,
	bson_t *query, const bson_t *opts,
	void *model, const mongo_parser model_parser
) {

	unsigned int retval = 1;

	if (collection && query && model && model_parser) {
		retval = mongo_find_one_internal (
			collection,
			query, opts,
			model, model_parser
		);

		bson_destroy (query);
	}

	return retval;

}

// uses a query to find one doc
// select is a dlist of strings used for document projection,
// _id is true by default and should not be incldued
// query gets destroyed, select structure remains the same
// returns 0 on success, 1 on error
unsigned int mongo_find_one (
	mongoc_collection_t *collection,
	bson_t *query, const CMongoSelect *select,
	void *model, const mongo_parser model_parser
) {

	unsigned int retval = 1;

	if (collection && query && model && model_parser) {
		bson_t *opts = mongo_find_generate_opts (select);

		retval = mongo_find_one_internal (
			collection,
			query, opts,
			model, model_parser
		);

		if (opts) bson_destroy (opts);

		bson_destroy (query);
	}

	return retval;

}

// inserts a document into a collection
// destroys document
// returns 0 on success, 1 on error
int mongo_insert_one (
	mongoc_collection_t *collection, bson_t *doc
) {

	int retval = 1;

	if (collection && doc) {
		bson_error_t error = { 0 };

		retval = mongoc_collection_insert_one (
			collection, doc, NULL, NULL, &error
		) ? 0 : 1;

		bson_destroy (doc);
	}

	return retval;

}

// inserts many documents into a collection
// docs are NOT deleted after the operation
// returns 0 on success, 1 on error
int mongo_insert_many (
	mongoc_collection_t *collection,
	const bson_t **docs, size_t n_docs
) {

	int retval = 1;

	if (collection && docs) {
		bson_error_t error = { 0 };

		retval = mongoc_collection_insert_many (
			collection, docs, n_docs, NULL, NULL, &error
		) ? 0 : 1;
	}

	return retval;

}


// updates a doc by a matching query with the new values
// destroys query and update documents
// returns 0 on success, 1 on error
int mongo_update_one (
	mongoc_collection_t *collection,
	bson_t *query, bson_t *update
) {

	int retval = 1;

	if (collection && query && update) {
		bson_error_t error = { 0 };

		retval = mongoc_collection_update_one (
			collection, query, update, NULL, NULL, &error
		) ? 0 : 1;

		bson_destroy (query);
		bson_destroy (update);
	}

	return retval;

}

// updates all the query matching documents
// destroys the query and the update documents
// returns 0 on success, 1 on error
int mongo_update_many (
	mongoc_collection_t *collection,
	bson_t *query, bson_t *update
) {

	int retval = 0;

	if (collection && query && update) {
		bson_error_t error = { 0 };

		retval = mongoc_collection_update_many (
			collection, query, update, NULL, NULL, &error
		) ? 0 : 1;

		bson_destroy (query);
		bson_destroy (update);
	}

	return retval;

}

// deletes one matching document by a query
// destroys the query document
// returns 0 on success, 1 on error
int mongo_delete_one (
	mongoc_collection_t *collection, bson_t *query
) {

	int retval = 0;

	if (collection && query) {
		bson_error_t error = { 0 };

		retval = mongoc_collection_delete_one (
			collection, query, NULL, NULL, &error
		) ? 0 : 1;

		bson_destroy (query);
	}

	return retval;

}

// deletes all the query matching documents
// destroys the query
// returns 0 on success, 1 on error
int mongo_delete_many (
	mongoc_collection_t *collection, bson_t *query
) {

	int retval = 0;

	if (collection && query) {
		bson_error_t error = { 0 };

		retval = mongoc_collection_delete_many (
			collection, query, NULL, NULL, &error
		) ? 0 : 1;

		bson_destroy (query);
	}

	return retval;

}

#ifdef __cplusplus
#pragma GCC diagnostic pop
#endif