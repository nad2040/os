#include "util.h"

#include <ctype.h>
#include <err.h>
#include <stdlib.h>
#include <string.h>

/*
 * strip_copy makes heap-allocated copy of string with the leading
 * and trailing whitespace gone.
 */
char *
strip_copy(const char *string)
{
	char *stripped;
	const char *begin = string;
	const char *end = string + strlen(string) - 1;
	while (isspace((unsigned char)*begin)) {
		begin++;
	}
	while (isspace((unsigned char)*end)) {
		end--;
	}
	if ((stripped = strndup(begin, end + 1 - begin)) == NULL) {
		err(EXIT_FAILURE, "failed to make stripped copy");
	}
	return stripped;
}

/*
 * free_and_copy frees string pointed to by dst and dups string src
 * *dst must be initialized to NULL.
 */
void
free_and_copy(char **dst, char *src)
{
	free(*dst);
	if ((*dst = strdup(src)) == NULL) {
		err(EXIT_FAILURE, "failed to make copy");
	}
}
