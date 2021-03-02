#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdarg.h>

#include <bson/bson.h>
#include <mongoc/mongoc.h>

#include "cmongo/mongo.h"

Mongo mongo = { 0 };

MongoStatus mongo_get_status (void) { return mongo.status; }

void mongo_set_db_name (const char *db_name) {

	if (db_name) {
		(void) strncpy (mongo.db_name, db_name, CMONGO_DB_NAME_SIZE - 1);
		mongo.db_name_len = strlen (mongo.db_name);
	}

}

void mongo_set_host (const char *host) {

	if (host) {
		(void) strncpy (mongo.host, host, CMONGO_HOST_SIZE - 1);
		mongo.host_len = strlen (mongo.host);
	}

}

void mongo_set_port (const unsigned int port) {

	mongo.port = port;

}

void mongo_set_username (const char *username) {

	if (username) {
		(void) strncpy (mongo.username, username, CMONGO_USERNAME_SIZE - 1);
		mongo.username_len = strlen (mongo.username);
	}

}

void mongo_set_password (const char *pswd) {

	if (pswd) {
		(void) strncpy (mongo.password, pswd, CMONGO_PASSWORD_SIZE - 1);
		mongo.password_len = strlen (mongo.password);
	}

}

void mongo_set_app_name (const char *app_name) {

	if (app_name) {
		(void) strncpy (mongo.app_name, app_name, CMONGO_APP_NAME_SIZE - 1);
		mongo.app_name_len = strlen (mongo.app_name);
	}

}

void mongo_set_uri (const char *uri) {

	if (uri) {
		(void) strncpy (mongo.uri, uri, CMONGO_URI_SIZE - 1);
		mongo.uri_len = strlen (mongo.uri);
	}

}

// generates a new uri string with the set values
// (username, password, host, port & db name)
// that can be used to set as the uri for a new connection
// returns 0 on success 1 on error
unsigned int mongo_uri_generate (void) {

	unsigned int retval = 1;

	if (
		mongo.host_len
		&& mongo.port
		&& mongo.db_name_len
	) {
		if (mongo.username_len && mongo.password_len) {
			mongo.uri_len = snprintf (
				mongo.uri, CMONGO_URI_SIZE - 1,
				"mongodb://%s:%s@%s:%u/%s",
				mongo.username, mongo.password,
				mongo.host, mongo.port,
				mongo.db_name
			);

			retval = 0;
		}

		else {
			mongo.uri_len = snprintf (
				mongo.uri, CMONGO_URI_SIZE - 1,
				"mongodb://%s:%u/%s",
				mongo.host, mongo.port,
				mongo.db_name
			);

			retval = 0;
		}
	}

	return retval;

}

static unsigned int mongo_connect_internal (
	const mongoc_uri_t *uri
) {

	unsigned int retval = 1;

	mongo.pool = mongoc_client_pool_new (uri);
	if (mongo.pool) {
		mongoc_client_pool_set_error_api (mongo.pool, 2);

		if (mongo.app_name_len) {
			mongoc_client_pool_set_appname (
				mongo.pool, mongo.app_name
			);
		}

		mongo.status = MONGO_STATUS_CONNECTED;

		retval = 0;
	}

	return retval;

}

// connect to the mongo db with db name
unsigned int mongo_connect (void) {

	unsigned int retval = 1;

	if (mongo.uri_len) {
		bson_error_t error = { 0 };

		mongoc_init (); // init mongo internals

		// safely create mongo uri object
		mongoc_uri_t *uri = mongoc_uri_new_with_error (
			mongo.uri, &error
		);

		if (uri) {
			retval = mongo_connect_internal (uri);

			mongoc_uri_destroy (uri);
		}

		else {
			(void) fprintf (
				stderr,
				"Failed to parse Mongo URI: %s",
				mongo.uri
			);

			(void) fprintf (
				stderr, "Error: %s\n\n", error.message
			);
		}
	}

	else {
		(void) fprintf (
			stderr,
			"Not uri string! "
			"Call mongo_set_uri () before attempting a connection\n"
		);
	}

	return retval;

}

// pings the db to test for a success connection
// returns 0 on success, 1 on error
unsigned int mongo_ping_db (void) {

	unsigned int retval = 1;

	if (mongo.status == MONGO_STATUS_CONNECTED) {
		bson_t *command = NULL, reply = { 0 };
		bson_error_t error = { 0 };

		// get a client from the pool
		mongoc_client_t *client = mongoc_client_pool_pop (mongo.pool);

		command = BCON_NEW ("ping", BCON_INT32 (1));

		if (mongoc_client_command_simple (
			client,
			mongo.db_name,
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

	return retval;

}

// disconnects from the db
void mongo_disconnect (void) {

	mongoc_client_pool_destroy (mongo.pool);
	mongo.pool = NULL;

	mongoc_cleanup ();

	mongo.status = MONGO_STATUS_DISCONNECTED;

}