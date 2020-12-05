#ifndef _CMONGO_VERSION_H_
#define _CMONGO_VERSION_H_

#define CMONGO_VERSION						"1.0"
#define CMONGO_VERSION_NAME					"Release 1.0"
#define CMONGO_VERSION_DATE					"04/12/2020"
#define CMONGO_VERSION_TIME					"18:21 CST"
#define CMONGO_VERSION_AUTHOR				"Erick Salas"

// print full cmongo version information
extern void cmongo_version_print_full (void);

// print the version id
extern void cmongo_version_print_version_id (void);

// print the version name
extern void cmongo_version_print_version_name (void);

#endif