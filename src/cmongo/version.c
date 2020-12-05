#include <stdio.h>

#include "cmongo/version.h"

// print full erver version information
void cmongo_version_print_full (void) {

	printf ("\nCmongo Version: %s\n", CMONGO_VERSION_NAME);
	printf ("Release Date & time: %s - %s\n", CMONGO_VERSION_DATE, CMONGO_VERSION_TIME);
	printf ("Author: %s\n\n", CMONGO_VERSION_AUTHOR);

}

// print the version id
void cmongo_version_print_version_id (void) {

	printf ("\nCmongo Version ID: %s\n", CMONGO_VERSION);

}

// print the version name
void cmongo_version_print_version_name (void) {

	printf ("\nCmongo Version: %s\n", CMONGO_VERSION_NAME);

}