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

static MongoStatus status = MONGO_STATUS_DISCONNECTED;

MongoStatus mongo_get_status (void) { return status; }

static mongoc_uri_t *uri = NULL;
mongoc_client_t *client = NULL;
static mongoc_database_t *database = NULL;

static String *host = NULL;

void mongo_set_host (const char *h) { if (h) host = str_new (h); }

static String *port = NULL;

void mongo_set_port (const char *p) { if (p) port = str_new (p); }

static String *username = NULL;

void mongo_set_username (const char *u) { if (u) username = str_new (u); }

static String *password = NULL;

void mongo_set_password (const char *pswd) { if (pswd) password = str_new (pswd); }

String *db_name = NULL;

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