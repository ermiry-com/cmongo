#ifndef _CMONGO_VERSION_H_
#define _CMONGO_VERSION_H_

#include "cmongo/config.h"

#define CMONGO_VERSION						"1.0b-9"
#define CMONGO_VERSION_NAME					"Beta 1.0b-9"
#define CMONGO_VERSION_DATE					"12/04/2021"
#define CMONGO_VERSION_TIME					"17:40 CST"
#define CMONGO_VERSION_AUTHOR				"Erick Salas"

#ifdef __cplusplus
extern "C" {
#endif

// print full cmongo version information
CMONGO_EXPORT void cmongo_version_print_full (void);

// print the version id
CMONGO_EXPORT void cmongo_version_print_version_id (void);

// print the version name
CMONGO_EXPORT void cmongo_version_print_version_name (void);

#ifdef __cplusplus
}
#endif

#endif