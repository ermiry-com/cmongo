#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/stat.h>

#include <cmongo/version.h>

// opens and reads a file into a buffer
// sets file size to the amount of bytes read
static char *file_read (const char *filename, size_t *file_size) {

	char *file_contents = NULL;

	if (filename) {
		struct stat filestatus = { 0 };
		if (!stat (filename, &filestatus)) {
			FILE *fp = fopen (filename, "rt");
			if (fp) {
				*file_size = filestatus.st_size;
				file_contents = (char *) malloc (filestatus.st_size);

				// read the entire file into the buffer
				if (fread (file_contents, filestatus.st_size, 1, fp) != 1) {
					(void) fprintf (
						stderr, "Failed to read file (%s) contents!", filename
					);

					free (file_contents);
				}

				(void) fclose (fp);
			}
		}
		

		else {
			(void) fprintf (stderr, "Unable to open file %s.", filename);
		}
	}

	return file_contents;

}

int main (int argc, char **argv) {

	// get version from file
	size_t version_len = 0;
	char *version_from_file = file_read ("version.txt", &version_len);
	
	if (version_from_file) {
		if (!strcmp (CMONGO_VERSION, version_from_file)) {
			return 0;
		}
	}

	return 1;

}