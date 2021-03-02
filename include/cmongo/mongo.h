#ifndef _CMONGO_MONGO_H_
#define _CMONGO_MONGO_H_

#include <bson/bson.h>
#include <mongoc/mongoc.h>

#include "cmongo/config.h"

#define CMONGO_DB_NAME_SIZE				128
#define CMONGO_HOST_SIZE				128

#define CMONGO_USERNAME_SIZE			128
#define CMONGO_PASSWORD_SIZE			128

#define CMONGO_APP_NAME_SIZE			128

#define CMONGO_URI_SIZE					1024

#ifdef __cplusplus
extern "C" {
#endif

typedef enum MongoStatus {

	MONGO_STATUS_DISCONNECTED		= 0,
	MONGO_STATUS_CONNECTED			= 1

} MongoStatus;

typedef struct Mongo {

	MongoStatus status;

	size_t db_name_len;
	char db_name[CMONGO_DB_NAME_SIZE];

	size_t host_len;
	char host[CMONGO_HOST_SIZE];
	unsigned int port;

	size_t username_len;
	char username[CMONGO_USERNAME_SIZE];
	size_t password_len;
	char password[CMONGO_PASSWORD_SIZE];

	size_t app_name_len;
	char app_name[CMONGO_APP_NAME_SIZE];

	size_t uri_len;
	char uri[CMONGO_URI_SIZE];

} Mongo;

CMONGO_EXPORT void mongo_set_db_name (const char *db_name);

CMONGO_EXPORT void mongo_set_host (const char *host);

CMONGO_EXPORT void mongo_set_port (const unsigned int port);

CMONGO_EXPORT void mongo_set_username (const char *username);

CMONGO_EXPORT void mongo_set_password (const char *pswd);

CMONGO_EXPORT void mongo_set_app_name (const char *app_name);

CMONGO_EXPORT void mongo_set_uri (const char *uri);

// generates a new uri string with the set values
// (username, password, host, port & db name)
// that can be used to set as the uri for a new connection
// returns 0 on success 1 on error
CMONGO_EXPORT unsigned int mongo_uri_generate (void);

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