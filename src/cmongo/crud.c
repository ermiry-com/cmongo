#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <bson/bson.h>
#include <mongoc/mongoc.h>

#include "cmongo/crud.h"
#include "cmongo/model.h"
#include "cmongo/mongo.h"
#include "cmongo/select.h"
#include "cmongo/types.h"

#define CMONGO_UNWIND_VALUE_SIZE			64

#ifdef __cplusplus
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

#pragma region count

// counts the docs in a collection by a matching query
static int64_t mongo_count_docs_internal (
	mongoc_collection_t *collection, bson_t *query
) {

	int64_t retval = 0;

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

	return retval;

}

// counts the docs in a collection by a matching query
int64_t mongo_count_docs (
	const CMongoModel *model, bson_t *query
) {

	int64_t retval = 0;

	if (model && query) {
		mongoc_client_t *client = mongoc_client_pool_pop (mongo.pool);
		if (client) {
			mongoc_collection_t *collection = mongoc_client_get_collection (
				client, mongo.db_name, model->collname
			);

			if (collection) {
				retval = mongo_count_docs_internal (
					collection, query
				);

				mongoc_collection_destroy (collection);
			}

			mongoc_client_pool_push (mongo.pool, client);
		}

		bson_destroy (query);
	}

	return retval;

}

#pragma endregion

#pragma region find

// returns true if 1 or more documents matches the query
// returns false if no matches
bool mongo_check (
	const CMongoModel *model, bson_t *query
) {

	bool retval = false;

	if (model && query) {
		mongoc_client_t *client = mongoc_client_pool_pop (mongo.pool);
		if (client) {
			mongoc_collection_t *collection = mongoc_client_get_collection (
				client, mongo.db_name, model->collname
			);

			if (collection) {
				bson_error_t error = { 0 };

				retval = mongoc_collection_count_documents (
					collection, query, NULL, NULL, NULL, &error
				);

				mongoc_collection_destroy (collection);
			}

			mongoc_client_pool_push (mongo.pool, client);
		}

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

			for (size_t idx = 0; idx < select->n_fields; idx++) {
				(void) bson_append_bool (
					&projection_doc,
					select->fields[idx].value,
					select->fields[idx].len,
					true
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
	const CMongoModel *model,
	bson_t *query, const CMongoSelect *select,
	uint64_t *n_docs
) {

	mongoc_cursor_t *cursor = NULL;
	*n_docs = 0;

	if (model && query) {
		mongoc_client_t *client = mongoc_client_pool_pop (mongo.pool);
		if (client) {
			mongoc_collection_t *collection = mongoc_client_get_collection (
				client, mongo.db_name, model->collname
			);

			if (collection) {
				uint64_t count = mongo_count_docs_internal (
					collection, bson_copy (query)
				);

				if (count > 0) {
					bson_t *opts = mongo_find_generate_opts (select);

					cursor = mongoc_collection_find_with_opts (
						collection, query, opts, NULL
					);

					*n_docs = count;

					if (opts) bson_destroy (opts);
					bson_destroy (query);
				}

				mongoc_collection_destroy (collection);
			}

			mongoc_client_pool_push (mongo.pool, client);
		}
	}

	return cursor;

}

// uses a query to find all matching docs with the specified options
// query gets destroyed, options remain the same
mongoc_cursor_t *mongo_find_all_cursor_with_opts (
	const CMongoModel *model,
	bson_t *query, const bson_t *opts
) {

	mongoc_cursor_t *cursor = NULL;

	if (model && query) {
		mongoc_client_t *client = mongoc_client_pool_pop (mongo.pool);
		if (client) {
			mongoc_collection_t *collection = mongoc_client_get_collection (
				client, mongo.db_name, model->collname
			);

			if (collection) {
				cursor = mongoc_collection_find_with_opts (
					collection, query, opts, NULL
				);

				mongoc_collection_destroy (collection);
			}

			mongoc_client_pool_push (mongo.pool, client);
		}

		bson_destroy (query);
	}

	return cursor;

}

static char *mongo_find_all_cursor_with_opts_to_json_internal (
	mongoc_cursor_t *cursor,
	const char *array_name, size_t *json_len
) {

	char *json = NULL;

	bson_t *doc = bson_new ();
	if (doc) {
		char buf[BSON_ARRAY_BUFFER_SIZE] = { 0 };
		const char *key = NULL;
		size_t keylen = 0;

		bson_t json_array = BSON_INITIALIZER;
		(void) bson_append_array_begin (doc, array_name, (int) strlen (array_name), &json_array);

		int i = 0;
		const bson_t *object_doc = NULL;
		while (mongoc_cursor_next (cursor, &object_doc)) {
			keylen = bson_uint32_to_string (i, &key, buf, BSON_ARRAY_BUFFER_SIZE);
			(void) bson_append_document (&json_array, key, (int) keylen, object_doc);

			bson_destroy ((bson_t *) object_doc);

			i++;
		}

		(void) bson_append_array_end (doc, &json_array);

		json = bson_as_relaxed_extended_json (doc, json_len);
	}

	return json;

}

// returns a new string in relaxed extended JSON format
// with all matching objects inside an array
// query gets destroyed, opts are kept the same
char *mongo_find_all_cursor_with_opts_to_json (
	const CMongoModel *model,
	bson_t *query, const bson_t *opts,
	const char *array_name, size_t *json_len
) {

	char *json = NULL;

	mongoc_cursor_t *cursor = mongo_find_all_cursor_with_opts (
		model,
		query, opts
	);

	if (cursor) {
		json = mongo_find_all_cursor_with_opts_to_json_internal (
			cursor,
			array_name, json_len
		);

		mongoc_cursor_destroy (cursor);
	}

	return json;

}

static bson_t *mongo_find_all_populate_object_pipeline (
	const char *from, const char *local_field
) {

	char unwind[CMONGO_UNWIND_VALUE_SIZE] = { 0 };
	(void) snprintf (
		unwind, CMONGO_UNWIND_VALUE_SIZE - 1,
		"$%s", local_field
	);

	bson_t *pipeline = BCON_NEW (
		"pipeline",
		"[",
			"{",
				"$lookup", "{",
					"from", BCON_UTF8 (from),
					"localField", BCON_UTF8 (local_field),
					"foreignField", BCON_UTF8 ("_id"),
					"as", BCON_UTF8 (local_field),
				"}",
			"}",

			"{",
				"$unwind", BCON_UTF8 (unwind),
			"}",
		"]"
	);

	return pipeline;

}

// works like mongo_find_all_cursor_with_opts_to_json ()
// but also populates the specified object
unsigned int mongo_find_all_populate_object_to_json (
	const CMongoModel *model,
	const char *from, const char *local_field,
	const char *array_name,
	char **json, size_t *json_len
) {

	unsigned int retval = 1;

	if (model && from && local_field) {
		mongoc_client_t *client = mongoc_client_pool_pop (mongo.pool);
		if (client) {
			mongoc_collection_t *collection = mongoc_client_get_collection (
				client, mongo.db_name, model->collname
			);

			if (collection) {
				bson_t *pipeline = mongo_find_all_populate_object_pipeline (
					from, local_field
				);

				if (pipeline) {
					mongoc_cursor_t *cursor = mongoc_collection_aggregate (
						collection, 0, pipeline, NULL, NULL
					);

					if (cursor) {
						*json = mongo_find_all_cursor_with_opts_to_json_internal (
							cursor, array_name, json_len
						);

						mongoc_cursor_destroy (cursor);

						retval = 0;
					}

					bson_destroy (pipeline);
				}

				mongoc_collection_destroy (collection);
			}

			mongoc_client_pool_push (mongo.pool, client);
		}
	}

	return retval;

}

static inline bson_t *mongo_find_all_populate_array_pipeline (
	const char *from, const char *local_field
) {

	bson_t *pipeline = BCON_NEW (
		"pipeline",
		"[",
			"{",
				"$lookup", "{",
					"from", BCON_UTF8 (from),
					"localField", BCON_UTF8 (local_field),
					"foreignField", BCON_UTF8 ("_id"),
					"as", BCON_UTF8 (local_field),
				"}",
			"}",
		"]"
	);

	return pipeline;

}

// works like mongo_find_all_cursor_with_opts_to_json ()
// but also populates the specified objects array
unsigned int mongo_find_all_populate_array_to_json (
	const CMongoModel *model,
	const char *from, const char *local_field,
	const char *array_name,
	char **json, size_t *json_len
) {

	unsigned int retval = 1;

	if (model && from && local_field) {
		mongoc_client_t *client = mongoc_client_pool_pop (mongo.pool);
		if (client) {
			mongoc_collection_t *collection = mongoc_client_get_collection (
				client, mongo.db_name, model->collname
			);

			if (collection) {
				bson_t *pipeline = mongo_find_all_populate_array_pipeline (
					from, local_field
				);

				if (pipeline) {
					mongoc_cursor_t *cursor = mongoc_collection_aggregate (
						collection, 0, pipeline, NULL, NULL
					);

					if (cursor) {
						*json = mongo_find_all_cursor_with_opts_to_json_internal (
							cursor, array_name, json_len
						);

						mongoc_cursor_destroy (cursor);

						retval = 0;
					}

					bson_destroy (pipeline);
				}

				mongoc_collection_destroy (collection);
			}

			mongoc_client_pool_push (mongo.pool, client);
		}
	}

	return retval;

}

const bson_t **mongo_find_all_internal (
	mongoc_collection_t *collection,
	bson_t *query, const CMongoSelect *select,
	uint64_t *n_docs
) {

	const bson_t **retval = NULL;

	uint64_t count = mongo_count_docs_internal (collection, bson_copy (query));
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
	}

	return retval;

}

// use a query to find all matching documents
// an empty query will return all the docs in a collection
const bson_t **mongo_find_all (
	const CMongoModel *model,
	bson_t *query, const CMongoSelect *select,
	uint64_t *n_docs
) {

	const bson_t **retval = NULL;
	*n_docs = 0;

	if (model && query) {
		mongoc_client_t *client = mongoc_client_pool_pop (mongo.pool);
		if (client) {
			mongoc_collection_t *collection = mongoc_client_get_collection (
				client, mongo.db_name, model->collname
			);

			if (collection) {
				retval = mongo_find_all_internal (
					collection,
					query, select,
					n_docs
				);

				mongoc_collection_destroy (collection);
			}

			mongoc_client_pool_push (mongo.pool, client);
		}

		bson_destroy (query);
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
	void *output, const mongo_parser model_parser
) {

	unsigned int retval = 1;

	mongoc_cursor_t *cursor = mongoc_collection_find_with_opts (
		collection, query, opts, NULL
	);

	if (cursor) {
		(void) mongoc_cursor_set_limit (cursor, 1);

		const bson_t *doc = NULL;
		if (mongoc_cursor_next (cursor, &doc)) {
			model_parser (output, doc);
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
	const CMongoModel *model,
	bson_t *query, const bson_t *opts,
	void *output
) {

	unsigned int retval = 1;

	if (model && query && output) {
		mongoc_client_t *client = mongoc_client_pool_pop (mongo.pool);
		if (client) {
			mongoc_collection_t *collection = mongoc_client_get_collection (
				client, mongo.db_name, model->collname
			);

			if (collection) {
				retval = mongo_find_one_internal (
					collection,
					query, opts,
					output, model->model_parser
				);

				mongoc_collection_destroy (collection);
			}

			mongoc_client_pool_push (mongo.pool, client);
		}

		bson_destroy (query);
	}

	return retval;

}

static unsigned int mongo_find_one_internal_to_json (
	mongoc_collection_t *collection,
	bson_t *query, const bson_t *opts,
	char **json, size_t *json_len
) {

	unsigned int retval = 1;

	mongoc_cursor_t *cursor = mongoc_collection_find_with_opts (
		collection, query, opts, NULL
	);

	if (cursor) {
		(void) mongoc_cursor_set_limit (cursor, 1);

		const bson_t *doc = NULL;
		if (mongoc_cursor_next (cursor, &doc)) {
			*json = bson_as_relaxed_extended_json (doc, json_len);
			retval = 0;
		}

		mongoc_cursor_destroy (cursor);
	}

	return retval;

}

// works like mongo_find_one_with_opts ()
// creates a new string with the result in relaxed extended JSON format
// query gets destroyed, opts are kept the same
// returns 0 on success, 1 on error
unsigned int mongo_find_one_with_opts_to_json (
	const CMongoModel *model,
	bson_t *query, const bson_t *opts,
	char **json, size_t *json_len
) {

	unsigned int retval = 1;

	if (model && query && json && json_len) {
		mongoc_client_t *client = mongoc_client_pool_pop (mongo.pool);
		if (client) {
			mongoc_collection_t *collection = mongoc_client_get_collection (
				client, mongo.db_name, model->collname
			);

			if (collection) {
				retval = mongo_find_one_internal_to_json (
					collection,
					query, opts,
					json, json_len
				);

				mongoc_collection_destroy (collection);
			}

			mongoc_client_pool_push (mongo.pool, client);
		}

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
	const CMongoModel *model,
	bson_t *query, const CMongoSelect *select,
	void *output
) {

	unsigned int retval = 1;

	if (model && query && output) {
		mongoc_client_t *client = mongoc_client_pool_pop (mongo.pool);
		if (client) {
			mongoc_collection_t *collection = mongoc_client_get_collection (
				client, mongo.db_name, model->collname
			);

			if (collection) {
				bson_t *opts = mongo_find_generate_opts (select);

				retval = mongo_find_one_internal (
					collection,
					query, opts,
					output, model->model_parser
				);

				if (opts) bson_destroy (opts);

				mongoc_collection_destroy (collection);
			}

			mongoc_client_pool_push (mongo.pool, client);
		}

		bson_destroy (query);
	}

	return retval;

}

static bson_t *mongo_find_one_populate_object_pipeline (
	const bson_oid_t *oid,
	const char *from, const char *local_field
) {

	char unwind[CMONGO_UNWIND_VALUE_SIZE] = { 0 };
	(void) snprintf (
		unwind, CMONGO_UNWIND_VALUE_SIZE - 1,
		"$%s", local_field
	);

	bson_t *pipeline = BCON_NEW (
		"pipeline",
		"[",
			"{",
				"$match", "{", "store", BCON_OID (oid), "}",
			"}",
			"{",
				"$lookup", "{",
					"from", BCON_UTF8 (from),
					"localField", BCON_UTF8 (local_field),
					"foreignField", BCON_UTF8 ("_id"),
					"as", BCON_UTF8 (local_field),
				"}",
			"}",

			"{",
				"$unwind", BCON_UTF8 (unwind),
			"}",
		"]"
	);

	return pipeline;

}

static unsigned int mongo_find_one_aggregate_internal (
	mongoc_collection_t *collection,
	const bson_t *pipeline,
	void *output, const mongo_parser model_parser
) {

	unsigned int retval = 1;

	mongoc_cursor_t *cursor = mongoc_collection_aggregate (
		collection, 0, pipeline, NULL, NULL
	);

	if (cursor) {
		const bson_t *doc = NULL;
		if (mongoc_cursor_next (cursor, &doc)) {
			model_parser (output, doc);
			retval = 0;
		}

		mongoc_cursor_destroy (cursor);
	}

	return retval;

}

// performs an aggregation in the model's collection
// to match an object by its oid and then lookup & undwind
// the selected field using its _id
// returns 0 on success, 1 on error
unsigned int mongo_find_one_populate_object (
	const CMongoModel *model,
	const bson_oid_t *oid,
	const char *from, const char *local_field,
	void *output
) {

	unsigned int retval = 1;

	if (model && oid && from && local_field && output) {
		mongoc_client_t *client = mongoc_client_pool_pop (mongo.pool);
		if (client) {
			mongoc_collection_t *collection = mongoc_client_get_collection (
				client, mongo.db_name, model->collname
			);

			if (collection) {
				bson_t *pipeline = mongo_find_one_populate_object_pipeline (
					oid, from, local_field
				);

				if (pipeline) {
					retval = mongo_find_one_aggregate_internal (
						collection, pipeline,
						output, model->model_parser
					);
					
					bson_destroy (pipeline);
				}

				mongoc_collection_destroy (collection);
			}

			mongoc_client_pool_push (mongo.pool, client);
		}
	}

	return retval;

}

// works like mongo_find_one_populate_object ()
// but converts the result into a json string
// returns 0 on success, 1 on error
unsigned int mongo_find_one_populate_object_to_json (
	const CMongoModel *model,
	const bson_oid_t *oid,
	const char *from, const char *local_field,
	char **json, size_t *json_len
) {

	unsigned int retval = 1;

	if (model && oid && from && local_field) {
		mongoc_client_t *client = mongoc_client_pool_pop (mongo.pool);
		if (client) {
			mongoc_collection_t *collection = mongoc_client_get_collection (
				client, mongo.db_name, model->collname
			);

			if (collection) {
				bson_t *pipeline = mongo_find_one_populate_object_pipeline (
					oid, from, local_field
				);

				if (pipeline) {
					mongoc_cursor_t *cursor = mongoc_collection_aggregate (
						collection, 0, pipeline, NULL, NULL
					);

					if (cursor) {
						const bson_t *doc = NULL;
						if (mongoc_cursor_next (cursor, &doc)) {
							*json = bson_as_relaxed_extended_json (doc, json_len);
						}

						mongoc_cursor_destroy (cursor);

						retval = 0;
					}

					bson_destroy (pipeline);
				}

				mongoc_collection_destroy (collection);
			}

			mongoc_client_pool_push (mongo.pool, client);
		}
	}

	return retval;

}

static inline bson_t *mongo_find_one_populate_array_pipeline (
	const bson_oid_t *oid,
	const char *from, const char *local_field
) {

	bson_t *pipeline = BCON_NEW (
		"pipeline",
		"[",
			"{",
				"$match", "{", "_id", BCON_OID (oid), "}",
			"}",
			"{",
				"$lookup", "{",
					"from", BCON_UTF8 (from),
					"localField", BCON_UTF8 (local_field),
					"foreignField", BCON_UTF8 ("_id"),
					"as", BCON_UTF8 (local_field),
				"}",
			"}",
		"]"
	);

	return pipeline;

}

// performs an aggregation in the model's collection
// to match an object by its oid and then lookup (populate)
// the selected array by searching by the object's oids
// returns 0 on success, 1 on error
unsigned int mongo_find_one_populate_array (
	const CMongoModel *model,
	const bson_oid_t *oid,
	const char *from, const char *local_field,
	void *output
) {

	unsigned int retval = 1;

	if (model && oid && from && local_field && output) {
		mongoc_client_t *client = mongoc_client_pool_pop (mongo.pool);
		if (client) {
			mongoc_collection_t *collection = mongoc_client_get_collection (
				client, mongo.db_name, model->collname
			);

			if (collection) {
				bson_t *pipeline = mongo_find_one_populate_array_pipeline (
					oid, from, local_field
				);

				if (pipeline) {
					retval = mongo_find_one_aggregate_internal (
						collection, pipeline,
						output, model->model_parser
					);
					
					bson_destroy (pipeline);
				}

				mongoc_collection_destroy (collection);
			}

			mongoc_client_pool_push (mongo.pool, client);
		}
	}

	return retval;

}

// works like mongo_find_one_populate_array ()
// but converts the result into a json string
// returns 0 on success, 1 on error
unsigned int mongo_find_one_populate_array_to_json (
	const CMongoModel *model,
	const bson_oid_t *oid,
	const char *from, const char *local_field,
	char **json, size_t *json_len
) {

	unsigned int retval = 1;

	if (model && oid && from && local_field) {
		mongoc_client_t *client = mongoc_client_pool_pop (mongo.pool);
		if (client) {
			mongoc_collection_t *collection = mongoc_client_get_collection (
				client, mongo.db_name, model->collname
			);

			if (collection) {
				bson_t *pipeline = mongo_find_one_populate_array_pipeline (
					oid, from, local_field
				);

				if (pipeline) {
					mongoc_cursor_t *cursor = mongoc_collection_aggregate (
						collection, 0, pipeline, NULL, NULL
					);

					if (cursor) {
						const bson_t *doc = NULL;
						if (mongoc_cursor_next (cursor, &doc)) {
							*json = bson_as_relaxed_extended_json (doc, json_len);
						}

						mongoc_cursor_destroy (cursor);

						retval = 0;
					}

					bson_destroy (pipeline);
				}

				mongoc_collection_destroy (collection);
			}

			mongoc_client_pool_push (mongo.pool, client);
		}
	}

	return retval;

}

// returns a new string in relaxed extended JSON format
// created with the result of an aggregation that represents
// how a single object's array gets populated
// pipeline gets destroyed, opts are kept the same
// returns 0 on success, 1 on error
unsigned int mongo_find_one_custom_populate_array_to_json (
	const CMongoModel *model,
	bson_t *pipeline,
	char **json, size_t *json_len
) {

	unsigned int retval = 1;

	if (model && pipeline && json_len) {
		mongoc_client_t *client = mongoc_client_pool_pop (mongo.pool);
		if (client) {
			mongoc_collection_t *collection = mongoc_client_get_collection (
				client, mongo.db_name, model->collname
			);

			if (collection) {
				mongoc_cursor_t *cursor = mongoc_collection_aggregate (
					collection,
					MONGOC_QUERY_NONE, pipeline, NULL, NULL
				);

				if (cursor) {
					const bson_t *doc = NULL;
					if (mongoc_cursor_next (cursor, &doc)) {
						*json = bson_as_relaxed_extended_json (doc, json_len);
					}

					mongoc_cursor_destroy (cursor);

					retval = 0;
				}

				mongoc_collection_destroy (collection);
			}

			mongoc_client_pool_push (mongo.pool, client);
		}

		bson_destroy (pipeline);
	}

	return retval;

}

#pragma endregion

#pragma region insert

// inserts a document into a collection
// destroys document
// returns 0 on success, 1 on error
unsigned int mongo_insert_one (
	const CMongoModel *model, bson_t *doc
) {

	unsigned int retval = 1;

	if (model && doc) {
		mongoc_client_t *client = mongoc_client_pool_pop (mongo.pool);
		if (client) {
			mongoc_collection_t *collection = mongoc_client_get_collection (
				client, mongo.db_name, model->collname
			);

			if (collection) {
				bson_error_t error = { 0 };

				retval = mongoc_collection_insert_one (
					collection, doc, NULL, NULL, &error
				) ? 0 : 1;

				mongoc_collection_destroy (collection);
			}

			mongoc_client_pool_push (mongo.pool, client);
		}

		bson_destroy (doc);
	}

	return retval;

}

// inserts many documents into a collection
// docs are NOT deleted after the operation
// returns 0 on success, 1 on error
unsigned int mongo_insert_many (
	const CMongoModel *model,
	const bson_t **docs, size_t n_docs
) {

	unsigned int retval = 1;

	if (model && docs) {
		mongoc_client_t *client = mongoc_client_pool_pop (mongo.pool);
		if (client) {
			mongoc_collection_t *collection = mongoc_client_get_collection (
				client, mongo.db_name, model->collname
			);

			if (collection) {
				bson_error_t error = { 0 };

				retval = mongoc_collection_insert_many (
					collection, docs, n_docs, NULL, NULL, &error
				) ? 0 : 1;

				mongoc_collection_destroy (collection);
			}

			mongoc_client_pool_push (mongo.pool, client);
		}
	}

	return retval;

}

#pragma endregion

#pragma region update

// updates a doc by a matching query with the new values
// destroys query and update documents
// returns 0 on success, 1 on error
unsigned int mongo_update_one (
	const CMongoModel *model,
	bson_t *query, bson_t *update
) {

	unsigned int retval = 1;

	if (model && query && update) {
		mongoc_client_t *client = mongoc_client_pool_pop (mongo.pool);
		if (client) {
			mongoc_collection_t *collection = mongoc_client_get_collection (
				client, mongo.db_name, model->collname
			);

			if (collection) {
				bson_error_t error = { 0 };

				retval = mongoc_collection_update_one (
					collection, query, update, NULL, NULL, &error
				) ? 0 : 1;

				mongoc_collection_destroy (collection);
			}

			mongoc_client_pool_push (mongo.pool, client);
		}

		bson_destroy (query);
		bson_destroy (update);
	}

	return retval;

}

// updates all the query matching documents
// destroys the query and the update documents
// returns 0 on success, 1 on error
unsigned int mongo_update_many (
	const CMongoModel *model,
	bson_t *query, bson_t *update
) {

	unsigned int retval = 0;

	if (model && query && update) {
		mongoc_client_t *client = mongoc_client_pool_pop (mongo.pool);
		if (client) {
			mongoc_collection_t *collection = mongoc_client_get_collection (
				client, mongo.db_name, model->collname
			);

			if (collection) {
				bson_error_t error = { 0 };

				retval = mongoc_collection_update_many (
					collection, query, update, NULL, NULL, &error
				) ? 0 : 1;

				mongoc_collection_destroy (collection);
			}

			mongoc_client_pool_push (mongo.pool, client);
		}

		bson_destroy (query);
		bson_destroy (update);
	}

	return retval;

}

#pragma endregion

#pragma region delete

// deletes one matching document by a query
// destroys the query document
// returns 0 on success, 1 on error
unsigned int mongo_delete_one (
	const CMongoModel *model, bson_t *query
) {

	unsigned int retval = 0;

	if (model && query) {
		mongoc_client_t *client = mongoc_client_pool_pop (mongo.pool);
		if (client) {
			mongoc_collection_t *collection = mongoc_client_get_collection (
				client, mongo.db_name, model->collname
			);

			if (collection) {
				bson_error_t error = { 0 };

				retval = mongoc_collection_delete_one (
					collection, query, NULL, NULL, &error
				) ? 0 : 1;

				mongoc_collection_destroy (collection);
			}

			mongoc_client_pool_push (mongo.pool, client);
		}

		bson_destroy (query);
	}

	return retval;

}

// deletes all the query matching documents
// destroys the query
// returns 0 on success, 1 on error
unsigned int mongo_delete_many (
	const CMongoModel *model, bson_t *query
) {

	unsigned int retval = 0;

	if (model && query) {
		mongoc_client_t *client = mongoc_client_pool_pop (mongo.pool);
		if (client) {
			mongoc_collection_t *collection = mongoc_client_get_collection (
				client, mongo.db_name, model->collname
			);

			if (collection) {
				bson_error_t error = { 0 };

				retval = mongoc_collection_delete_many (
					collection, query, NULL, NULL, &error
				) ? 0 : 1;

				mongoc_collection_destroy (collection);
			}

			mongoc_client_pool_push (mongo.pool, client);
		}

		bson_destroy (query);
	}

	return retval;

}

#pragma endregion

#pragma region aggregation

// performs an aggregation in the model's collection
// the pipeline document gets destroyed
// returns a cursor with the aggregation's result
mongoc_cursor_t *mongo_perform_aggregation_with_opts (
	const CMongoModel *model,
	mongoc_query_flags_t flags,
	const bson_t *opts,
	bson_t *pipeline
) {

	mongoc_cursor_t *cursor = NULL;

	if (model && pipeline) {
		mongoc_client_t *client = mongoc_client_pool_pop (mongo.pool);
		if (client) {
			mongoc_collection_t *collection = mongoc_client_get_collection (
				client, mongo.db_name, model->collname
			);

			if (collection) {
				cursor = mongoc_collection_aggregate (
					collection,
					flags, pipeline, opts, NULL
				);

				mongoc_collection_destroy (collection);
			}

			mongoc_client_pool_push (mongo.pool, client);
		}

		bson_destroy (pipeline);
	}

	return cursor;

}

// works like mongo_perform_aggregation_with_opts ()
// but sets flags to 0 and opts to NULL
mongoc_cursor_t *mongo_perform_aggregation (
	const CMongoModel *model, bson_t *pipeline
) {

	return mongo_perform_aggregation_with_opts (
		model, 0, NULL, pipeline
	);

}

// works like mongo_perform_aggregation ()
// but outputs all the aggregation's result into an object
// useful when aggregation returns just one document
// returns 0 on success, 1 on error
unsigned int mongo_perform_single_aggregation_to_json (
	const CMongoModel *model, bson_t *pipeline,
	char **json, size_t *json_len
) {

	unsigned int retval = 1;

	if (model && pipeline) {
		mongoc_client_t *client = mongoc_client_pool_pop (mongo.pool);
		if (client) {
			mongoc_collection_t *collection = mongoc_client_get_collection (
				client, mongo.db_name, model->collname
			);

			if (collection) {
				mongoc_cursor_t *cursor = mongoc_collection_aggregate (
					collection,
					MONGOC_QUERY_NONE, pipeline, NULL, NULL
				);

				if (cursor) {
					const bson_t *doc = NULL;
					if (mongoc_cursor_next (cursor, &doc)) {
						*json = bson_as_relaxed_extended_json (doc, json_len);
					}

					mongoc_cursor_destroy (cursor);

					retval = 0;
				}

				mongoc_collection_destroy (collection);
			}

			mongoc_client_pool_push (mongo.pool, client);
		}

		bson_destroy (pipeline);
	}

	return retval;

}

// works like mongo_perform_aggregation ()
// but outputs all the aggregation's result into an array
// useful when aggregation returns many matches
// returns 0 on success, 1 on error
unsigned int mongo_perform_aggregation_to_json (
	const CMongoModel *model, bson_t *pipeline,
	const char *array_name,
	char **json, size_t *json_len
) {

	unsigned int retval = 1;

	if (model && pipeline) {
		mongoc_client_t *client = mongoc_client_pool_pop (mongo.pool);
		if (client) {
			mongoc_collection_t *collection = mongoc_client_get_collection (
				client, mongo.db_name, model->collname
			);

			if (collection) {
				mongoc_cursor_t *cursor = mongoc_collection_aggregate (
					collection,
					MONGOC_QUERY_NONE, pipeline, NULL, NULL
				);

				if (cursor) {
					*json = mongo_find_all_cursor_with_opts_to_json_internal (
						cursor, array_name, json_len
					);

					mongoc_cursor_destroy (cursor);

					retval = 0;
				}

				mongoc_collection_destroy (collection);
			}

			mongoc_client_pool_push (mongo.pool, client);
		}

		bson_destroy (pipeline);
	}

	return retval;

}

#pragma endregion

#ifdef __cplusplus
#pragma GCC diagnostic pop
#endif