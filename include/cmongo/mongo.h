#ifndef _CMONGO_MONGO_H_
#define _CMONGO_MONGO_H_

#include <bson/bson.h>
#include <mongoc/mongoc.h>

#include "cmongo/config.h"

#ifdef __cplusplus
extern "C" {
#endif

extern mongoc_client_t *client;

extern char *db_name;

typedef enum MongoStatus {

	MONGO_STATUS_DISCONNECTED		= 0,
	MONGO_STATUS_CONNECTED			= 1

} MongoStatus;

CMONGO_EXPORT MongoStatus mongo_get_status (void);

CMONGO_EXPORT void mongo_set_host (const char *h);

CMONGO_EXPORT void mongo_set_port (const char *p);

CMONGO_EXPORT void mongo_set_username (const char *u);

CMONGO_EXPORT void mongo_set_password (const char *pswd);

CMONGO_EXPORT void mongo_set_db_name (const char *name);

CMONGO_EXPORT void mongo_set_app_name (const char *name);

CMONGO_EXPORT void mongo_set_uri (const char *uri);

// generates a new uri string with the set values (username, password, host, port & db name)
// that can be used to set as the uri for a new connection
// returns the newly uri string (that should be freed) on success
// returns NULL on error
CMONGO_EXPORT char *mongo_uri_generate (void);

// pings the db to test for a success connection
// returns 0 on success, 1 on error
CMONGO_EXPORT int mongo_ping_db (void);

// connect to the mongo db with db name
CMONGO_EXPORT int mongo_connect (void);

// disconnects from the db
CMONGO_EXPORT void mongo_disconnect (void);

#ifdef __cplusplus
}
#endif

#endif