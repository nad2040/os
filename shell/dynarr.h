// dynamic array implementation w/ type separation
// I believe type separation is useful

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#ifndef DYNARR_H
#	define DYNARR_H

#	define DYNARR_INIT_CAP (10)

#	define DYNARR(struct_name, type) \
		typedef struct {          \
			type *data;       \
			int size;         \
			int cap;          \
		} struct_name

#	define DYNARR_INIT(struct_name, arr_name)                          \
		struct_name *arr_name = malloc(sizeof(struct_name));        \
		if (!arr_name) {                                            \
			fprintf(stderr,                                     \
				"couldn't init " #struct_name " " #arr_name \
				" %s\n",                                    \
				strerror(errno));                           \
			exit(1);                                            \
		}                                                           \
		*arr_name = (struct_name) { 0 }

#	define DYNARR_FREE(arr) \
		free(arr->data); \
		free(arr);

#	define dynarr_append(arr, item)                                  \
		do {                                                      \
			if ((arr)->size >= (arr)->cap) {                  \
				(arr)->cap *= 2;                          \
				if ((arr)->cap == 0)                      \
					(arr)->cap = DYNARR_INIT_CAP;     \
				(arr)->data = realloc(                    \
				    (arr)->data,                          \
				    (arr)->cap * sizeof((arr)->data[0])); \
				if (!(arr)->data) {                       \
					fprintf(stderr,                   \
						"couldn't realloc " #arr  \
						" %s\n",                  \
						strerror(errno));         \
					exit(1);                          \
				}                                         \
			}                                                 \
			(arr)->data[(arr)->size++] = (item);              \
		} while (0)

#	define dynarr_pop(arr, index)                                   \
		do {                                                     \
			if (index >= 0 && index < (arr)->size) {         \
				memcpy(&(arr)->data[index],              \
				       &(arr)->data[index + 1],          \
				       sizeof((arr)->data[0])            \
					   * ((arr)->size - index - 1)); \
				(arr)->size--;                           \
				memset(&(arr)->data[(arr)->size], 0,     \
				       sizeof((arr)->data[0]));          \
			}                                                \
		} while (0)

#endif // DYNARR_H impl
