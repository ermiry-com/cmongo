#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdarg.h>

#include <bson/bson.h>
#include <mongoc/mongoc.h>

#include "cmongo/mongo.h"

static MongoStatus status = MONGO_STATUS_DISCONNECTED;

MongoStatus mongo_get_status (void) { return status; }

static mongoc_uri_t *uri = NULL;
mongoc_client_t *client = NULL;
static mongoc_database_t *database = NULL;

static char *host = NULL;

void mongo_set_host (const char *h) {
	
	if (h) host = strdup (h);
	
}

static char *port = NULL;

void mongo_set_port (const char *p) {
	
	if (p) port = strdup (p);
	
}

static char *username = NULL;

void mongo_set_username (const char *u) {
	
	if (u) username = strdup (u);
	
}

static char *password = NULL;

void mongo_set_password (const char *pswd) {
	
	if (pswd) password = strdup (pswd);
	
}

char *db_name = NULL;

void mongo_set_db_name (const char *name) {

	if (name) db_name = strdup (name);

}

static char *app_name = NULL;

void mongo_set_app_name (const char *name) {

	if (name) app_name = strdup (name);

}

static char *uri_string = NULL;

void mongo_set_uri (const char *uri) {

	if (uri) uri_string = strdup (uri);

}

// generates a new uri string with the set values (username, password, host, port & db name)
// that can be used to set as the uri for a new connection
// returns the newly uri string (that should be freed) on success, NULL on error
char *mongo_uri_generate (void) {

	char *retval = NULL;

	if (host && port && db_name) {
		char buffer[512] = { 0 };
		if (username && password) {
			(void) snprintf (
				buffer, 512,
				"mongodb://%s:%s@%s:%s/%s", 
				username, password, 
				host, port,
				db_name
			);
		}

		else {
			(void) snprintf (
				buffer, 512,
				"mongodb://%s:%s/%s", 
				host, port,
				db_name
			);
		}

		retval = strdup (buffer);
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
			bson_error_t error = { 0 };

			command = BCON_NEW ("ping", BCON_INT32 (1));
			if (mongoc_client_command_simple (
				client, 
				db_name, 
				command, 
				NULL, 
				&reply, 
				&error
			)) {
				// success
				char *str = bson_as_json (&reply, NULL);
				if (str) {
					(void) fprintf (stdout, "\n%s\n", str);
					free (str);
				}

				retval = 0;
			}

			else {
				(void) fprintf (
					stderr, "[MONGO][ERROR]: %s\n", error.message
				);
			}
		}

		else {
			(void) fprintf (
				stderr,
				"[MONGO][ERROR]: DB name hasn't been set! "
				"Use mongo_set_db_name ()\n"
			);
		}
	}

	else {
		(void) fprintf (
			stderr,
			"[MONGO][ERROR]: Not connected to mongo! "
			"Call mongo_connect () first\n"
		);
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
		uri = mongoc_uri_new_with_error (uri_string, &error);
		if (uri) {
			// create a new client instance
			client = mongoc_client_new_from_uri (uri);
			if (client) {
				// register the app name -> for logging info
				mongoc_client_set_appname (client, app_name);

				status = MONGO_STATUS_CONNECTED;

				retval = 0;
			}

			else {
				(void) fprintf (
					stderr, "Failed to create a new client instance!\n"
				);
			}
		}

		else {
			(void) fprintf (
				stderr,
				"failed to parse URI: %s\n"
				"error message:       %s\n",
				uri_string,
				error.message
			);
		}
	}

	else {
		(void) fprintf (
			stderr, "Not uri string! "
			"Call mongo_set_uri () before attemting a connection\n"
		);
	}

	return retval;

}

// disconnects from the db
void mongo_disconnect (void) {

	mongoc_database_destroy (database);
	mongoc_uri_destroy (uri);
	mongoc_client_destroy (client);

	if (host) free (host);
	if (port) free (port);
	if (username) free (username);
	if (password) free (password);
	
	if (app_name) free (app_name);
	if (uri_string) free (uri_string);
	if (db_name) free (db_name);

	mongoc_cleanup ();

	status = MONGO_STATUS_DISCONNECTED;

}