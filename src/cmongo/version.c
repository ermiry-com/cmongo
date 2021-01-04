#include <stdio.h>

#include "cmongo/version.h"

// print full erver version information
void cmongo_version_print_full (void) {

	(void) printf ("Cmongo Version: %s\n", CMONGO_VERSION_NAME);
	(void) printf ("Release Date & time: %s - %s\n", CMONGO_VERSION_DATE, CMONGO_VERSION_TIME);
	(void) printf ("Author: %s\n", CMONGO_VERSION_AUTHOR);

}

// print the version id
void cmongo_version_print_version_id (void) {

	(void) printf ("Cmongo Version ID: %s\n", CMONGO_VERSION);

}

// print the version name
void cmongo_version_print_version_name (void) {

	(void) printf ("Cmongo Version: %s\n", CMONGO_VERSION_NAME);

}