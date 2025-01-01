/* dynamic array implementation w/ different typenames */

#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#ifndef _DYNARR_H_
#define _DYNARR_H_

#define DYNARR_INIT_CAP (10)

#define DYNARR(struct_name, type)                                              \
	typedef struct {                                                       \
		type *data;                                                    \
		int size;                                                      \
		int cap;                                                       \
	} struct_name

#define DYNARR_ALLOC(arr_name)                                                 \
	do {                                                                   \
		arr_name = calloc(1, sizeof(*arr_name));                       \
		if (arr_name == NULL) {                                        \
			err(EXIT_FAILURE, "couldn't alloc " #arr_name);        \
		}                                                              \
	} while (0)

#define dynarr_append(arr, item)                                               \
	do {                                                                   \
		if ((arr)->size >= (arr)->cap) {                               \
			(arr)->cap *= 2;                                       \
			if ((arr)->cap == 0)                                   \
				(arr)->cap = DYNARR_INIT_CAP;                  \
			(arr)->data = realloc(                                 \
			    (arr)->data, (arr)->cap * sizeof((arr)->data[0])); \
			if (!(arr)->data) {                                    \
				err(EXIT_FAILURE, "couldn't realloc " #arr);   \
			}                                                      \
		}                                                              \
		(arr)->data[(arr)->size++] = (item);                           \
	} while (0)

#define dynarr_pop(arr, index)                                                 \
	do {                                                                   \
		if (index >= 0 && index < (arr)->size) {                       \
			memcpy(&(arr)->data[index], &(arr)->data[index + 1],   \
			       sizeof((arr)->data[0]) *                        \
			           ((arr)->size - index - 1));                 \
			(arr)->size--;                                         \
			memset(&(arr)->data[(arr)->size], 0,                   \
			       sizeof((arr)->data[0]));                        \
		}                                                              \
	} while (0)

#endif /* !_DYNARR_H_ */
