#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdarg.h>

#include <bson/bson.h>
#include <mongoc/mongoc.h>

#include "cmongo/mongo.h"

static Mongo mongo = { 0 };

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
			(void) snprintf (
				mongo.uri, CMONGO_URI_SIZE - 1,
				"mongodb://%s:%s@%s:%u/%s", 
				mongo.username, mongo.password, 
				mongo.host, mongo.port,
				mongo.db_name
			);
		}

		else {
			(void) snprintf (
				mongo.uri, CMONGO_URI_SIZE - 1,
				"mongodb://%s:%s/%s", 
				mongo.host, mongo.port,
				mongo.db_name
			);
		}
	}

	return retval;

}

static mongoc_uri_t *uri = NULL;
mongoc_client_t *client = NULL;
static mongoc_database_t *database = NULL;

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