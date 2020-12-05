#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdarg.h>

#include <mongoc/mongoc.h>
#include <bson/bson.h>

#include <clibs/types/string.h>

#include <clibs/collections/dlist.h>

#include <clibs/utils/utils.h>
#include <clibs/utils/log.h>

#include "cmongo/mongo.h"

#pragma region types

bson_oid_t *bson_oid_new (void) {

	bson_oid_t *oid = (bson_oid_t *) malloc (sizeof (bson_oid_t));
	if (oid) memset (oid, 0, sizeof (bson_oid_t));
	return oid;

}

void bson_oid_delete (void *bson_oid_t_ptr) { if (bson_oid_t_ptr) free (bson_oid_t_ptr); }

bson_oid_t *bson_oid_create (const bson_oid_t *original_oid) {

	bson_oid_t *oid = (bson_oid_t *) malloc (sizeof (bson_oid_t));
	if (oid) {
		bson_oid_copy (original_oid, oid);
	}

	return oid;

}

#pragma endregion

#pragma region connection

static MongoStatus status = MONGO_STATUS_DISCONNECTED;

MongoStatus mongo_get_status (void) { return status; }

static mongoc_uri_t *uri = NULL;
static mongoc_client_t *client = NULL;
static mongoc_database_t *database = NULL;

static String *host = NULL;

void mongo_set_host (const char *h) { if (h) host = str_new (h); }

static String *port = NULL;

void mongo_set_port (const char *p) { if (p) port = str_new (p); }

static String *username = NULL;

void mongo_set_username (const char *u) { if (u) username = str_new (u); }

static String *password = NULL;

void mongo_set_password (const char *pswd) { if (pswd) password = str_new (pswd); }

static String *db_name = NULL;

void mongo_set_db_name (const char *name) {

	if (name) db_name = str_new (name);

}

static String *app_name = NULL;

void mongo_set_app_name (const char *name) {

	if (name) app_name = str_new (name);

}

static String *uri_string = NULL;

void mongo_set_uri (const char *uri) {

	if (uri) uri_string = str_new (uri);

}

// generates a new uri string with the set values (username, password, host, port & db name)
// that can be used to set as the uri for a new connection
// returns the newly uri string (that should be freed) on success, NULL on error
char *mongo_uri_generate (void) {

	char *retval = NULL;

	if (host && port && db_name) {
		if (username && password) {
			retval = c_string_create (
				"mongodb://%s:%s@%s:%s/%s", 
				username->str, password->str, 
				host->str, port->str,
				db_name->str
			);
		}

		else {
			retval = c_string_create (
				"mongodb://%s:%s/%s", 
				host->str, port->str,
				db_name->str
			);
		}
	}

	return retval;

}

// pings the db to test for a success connection
// Possible connection problems -- failed to authenticate to the db
// returns 0 on success, 1 on error
int mongo_ping_db (void) {

	int retval = 1;

	if (client) {
		if (db_name) {
			bson_t *command = NULL, reply = { 0 };
			bson_error_t error;

			command = BCON_NEW ("ping", BCON_INT32 (1));
			if (mongoc_client_command_simple (
				client, 
				db_name->str, 
				command, 
				NULL, 
				&reply, 
				&error
			)) {
				// success
				char *str = bson_as_json (&reply, NULL);
				if (str) {
					fprintf (stdout, "\n%s\n", str);
					free (str);
				}

				retval = 0;
			}

			else {
				clibs_log_error ("[MONGO] %s", error.message);
			}
		}

		else {
			clibs_log_error ("DB name hasn't been set! Use mongo_set_db_name ()");
		}
	}

	else {
		clibs_log_error ("Not connected to mongo! Call mongo_connect () first");
	}

	return retval;

}

// connect to the mongo db with db name
int mongo_connect (void) {

	int retval = 1;

	if (uri_string) {
		bson_error_t error = { 0 };

		mongoc_init ();     // init mongo internals

		// safely create mongo uri object
		uri = mongoc_uri_new_with_error (uri_string->str, &error);
		if (!uri) {
			fprintf (
				stderr,
				"failed to parse URI: %s\n"
				"error message:       %s\n",
				uri_string->str,
				error.message
			);

			return 1;
		}

		// create a new client instance
		client = mongoc_client_new_from_uri (uri);
		if (client) {
			// register the app name -> for logging info
			mongoc_client_set_appname (client, app_name->str);

			status = MONGO_STATUS_CONNECTED;

			retval = 0;
		}

		else {
			clibs_log_error ("Failed to create a new client instance!\n");
		}
	}

	else {
		clibs_log_error ("Not uri string! Call mongo_set_uri () before attemting a connection");
	}

	return retval;

}

// disconnects from the db
void mongo_disconnect (void) {

	mongoc_database_destroy (database);
	mongoc_uri_destroy (uri);
	mongoc_client_destroy (client);

	str_delete (host);
	str_delete (port);
	str_delete (username);
	str_delete (password);
	
	str_delete (app_name);
	str_delete (uri_string);
	str_delete (db_name);

	mongoc_cleanup ();

	status = MONGO_STATUS_DISCONNECTED;

}

#pragma endregion

#pragma region collections

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

#pragma endregion

#pragma region CRUD

// counts the docs in a collection by a matching query
int64_t mongo_count_docs (mongoc_collection_t *collection, bson_t *query) {

	int64_t retval = 0;

	if (collection && query) {
		bson_error_t error = { 0 };
		retval = mongoc_collection_count_documents (collection, query, NULL, NULL, NULL, &error);
		if (retval < 0) {
			clibs_log_error ("%s", error.message);

			retval = 0;
		}

		bson_destroy (query);
	}

	return retval;

}

// returns true if 1 or more documents matches the query, false if no matches
bool mongo_check (mongoc_collection_t *collection, bson_t *query) {

	bool retval = false;

	if (collection && query) {
		bson_error_t error = { 0 };
		switch (mongoc_collection_count_documents (collection, query, NULL, NULL, NULL, &error)) {
			case -1: clibs_log_error ("%s", error.message); break;
			case 0: break;
			default: retval = true; break;
		}

		bson_destroy (query);
	}

	return retval;

}

// generates an opts doc that can be used to better work with find methods
// primarily used to query with projection (select) options
bson_t *mongo_find_generate_opts (const DoubleList *select) {

	bson_t *opts = bson_new ();

	if (opts) {
		// append projection
		if (select) {
			bson_t projection_doc = { 0 };
			bson_append_document_begin (opts, "projection", -1, &projection_doc);

			bson_append_bool (&projection_doc, "_id", -1, true);

			String *field = NULL;
			for (ListElement *le = dlist_start (select); le; le = le->next) {
				field = (String *) le->data;
				bson_append_bool (&projection_doc, field->str, field->len, true);
			}
			
			bson_append_document_end (opts, &projection_doc);
		}
	}

	return opts;

}

// use a query to find all matching documents
// select is a dlist of strings used for document projection, _id is true by default and should not be incldued
// returns a cursor (should be destroyed) that can be used to traverse the matching documents
// query gets destroyed, select list remains the same
mongoc_cursor_t *mongo_find_all_cursor (
	mongoc_collection_t *collection, 
	bson_t *query, const DoubleList *select,
	uint64_t *n_docs
) {

	mongoc_cursor_t *cursor = NULL;
	*n_docs = 0;

	if (collection && query) {
		uint64_t count = mongo_count_docs (collection, bson_copy (query));
		if (count > 0) {
			bson_t *opts = mongo_find_generate_opts (select);

			cursor = mongoc_collection_find_with_opts (collection, query, opts, NULL);

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
		cursor = mongoc_collection_find_with_opts (collection, query, opts, NULL);

		bson_destroy (query);
	}

	return cursor;

}

// use a query to find all matching documents
// an empty query will return all the docs in a collection
const bson_t **mongo_find_all (
	mongoc_collection_t *collection, 
	bson_t *query, const DoubleList *select,
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

			mongoc_cursor_t *cursor = mongoc_collection_find_with_opts (collection, query, opts, NULL);

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
void mongo_find_all_destroy_docs (bson_t **docs, uint64_t count) {

	if (docs) {
		for (uint64_t i = 0; i < count; i++) bson_destroy (docs[i]);

        free (docs);
	}

}

static void mongo_find_one_internal (
	mongoc_collection_t *collection, bson_t *query, const bson_t *opts, const bson_t **doc
) {

	mongoc_cursor_t *cursor = mongoc_collection_find_with_opts (collection, query, opts, NULL);
	mongoc_cursor_set_limit (cursor, 1);

	mongoc_cursor_next (cursor, doc);

	mongoc_cursor_destroy (cursor);

}

// uses a query to find one doc with the specified options
// query gets destroyed, opts are kept the same
const bson_t *mongo_find_one_with_opts (
	mongoc_collection_t *collection, bson_t *query, const bson_t *opts
) {

	const bson_t *doc = NULL;

	if (collection && query) {
		mongo_find_one_internal (collection, query, opts, &doc);

		bson_destroy (query);
	}

	return doc;

}

// uses a query to find one doc
// select is a dlist of strings used for document projection, _id is true by default and should not be incldued
// query gets destroyed, select list remains the same
const bson_t *mongo_find_one (
	mongoc_collection_t *collection, bson_t *query, const DoubleList *select
) {

	const bson_t *doc = NULL;

	if (collection && query) {
		bson_t *opts = mongo_find_generate_opts (select);

		mongo_find_one_internal (collection, query, opts, &doc);

		if (opts) bson_destroy (opts);

		bson_destroy (query);
	}

	return doc;

}

// inserts a document into a collection
// destroys document
// returns 0 on success, 1 on error
int mongo_insert_one (mongoc_collection_t *collection, bson_t *doc) {

	int retval = 1;

	if (collection && doc) {
		bson_error_t error = { 0 };

		if (mongoc_collection_insert_one (collection, doc, NULL, NULL, &error)) {
			retval = 0;		// success
		}

		else {
			clibs_log_error ("Insert failed: %s", error.message);
		}

		bson_destroy (doc);
	}

	return retval;

}

// inserts many documents into a collection
// returns 0 on success, 1 on error
int mongo_insert_many (mongoc_collection_t *collection, const bson_t **docs, size_t n_docs) {

	int retval = 1;

	if (collection && docs) {
		bson_error_t error = { 0 };

		if (mongoc_collection_insert_many (collection, docs, n_docs, NULL, NULL, &error)) {
			retval = 0;		// success
		}

		else {
			clibs_log_error ("Insert failed: %s", error.message);
		}

		// for (size_t i = 0; i < n_docs; i++) {
		// 	bson_destroy (docs[i]);
		// }

		// free (docs);
	}

	return retval;

}


// updates a doc by a matching query with the new values
// destroys query and update documents
// returns 0 on success, 1 on error
int mongo_update_one (mongoc_collection_t *collection, bson_t *query, bson_t *update) {

	int retval = 1;

	if (collection && query && update) {
		bson_error_t error = { 0 };

		if (mongoc_collection_update_one (collection, query, update, NULL, NULL, &error)) {
			retval = 0;		// success
		}

		else {
			clibs_log_error ("Update failed: %s", error.message);
		}

		bson_destroy (query);
		bson_destroy (update);
	}

	return retval;

}

// updates all the query matching documents
// destroys the query and the update documents
// returns 0 on success, 1 on error
int mongo_update_many (mongoc_collection_t *collection, bson_t *query, bson_t *update) {

	int retval = 0;

	if (collection && query && update) {
		bson_error_t error = { 0 };

		if (mongoc_collection_update_many (collection, query, update, NULL, NULL, &error)) {
			retval = 0;		// success
		}

		else {
			clibs_log_error ("Update failed: %s", error.message);
		}

		bson_destroy (query);
		bson_destroy (update);
	}

	return retval;

}

// deletes one matching document by a query
// destroys the query document
// returns 0 on success, 1 on error
int mongo_delete_one (mongoc_collection_t *collection, bson_t *query) {

	int retval = 0;

	if (collection && query) {
		bson_error_t error = { 0 };

		if (mongoc_collection_delete_one (collection, query, NULL, NULL, &error)) {
			retval = 0;		// success
		}

		else {
			clibs_log_error ("Delete failed: %s", error.message);
		}

		bson_destroy (query);
	}

	return retval;

}

// deletes all the query matching documents
// destroys the query
// returns 0 on success, 1 on error
int mongo_delete_many (mongoc_collection_t *collection, bson_t *query) {

	int retval = 0;

	if (collection && query) {
		bson_error_t error = { 0 };

		if (mongoc_collection_delete_many (collection, query, NULL, NULL, &error)) {
			retval = 0;		// success
		}

		else {
			clibs_log_error ("Delete failed: %s", error.message);
		}

		bson_destroy (query);
	}

	return retval;

}

#pragma endregion