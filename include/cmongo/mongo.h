#ifndef _CMONGO_MONGO_H_
#define _CMONGO_MONGO_H_

#include <mongoc/mongoc.h>
#include <bson/bson.h>

#include <clibs/types/string.h>

extern mongoc_client_t *client;

extern String *db_name;

typedef enum MongoStatus {

	MONGO_STATUS_DISCONNECTED		= 0,
	MONGO_STATUS_CONNECTED			= 1

} MongoStatus;

extern MongoStatus mongo_get_status (void);

extern void mongo_set_host (const char *h);

extern void mongo_set_port (const char *p);

extern void mongo_set_username (const char *u);

extern void mongo_set_password (const char *pswd);

extern void mongo_set_db_name (const char *name);

extern void mongo_set_app_name (const char *name);

extern void mongo_set_uri (const char *uri);

// generates a new uri string with the set values (username, password, host, port & db name)
// that can be used to set as the uri for a new connection
// returns the newly uri string (that should be freed) on success, NULL on error
extern char *mongo_uri_generate (void);

// pings the db to test for a success connection
// Possible connection problems -- failed to authenticate to the db
// returns 0 on success, 1 on error
extern int mongo_ping_db (void);

// connect to the mongo db with db name
extern int mongo_connect (void);

// disconnects from the db
extern void mongo_disconnect (void);

#endif