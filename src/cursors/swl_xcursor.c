#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <soilleirwl/logger.h>
#include <soilleirwl/cursors/swl_xcursor.h>
#include <string.h>


#define SWL_XCURSOR_DIRECTORIES "/home/caitlyn/.icons:/usr/share/icons"


static swl_cursor_t *fallback_cursor(uint32_t size) {
	swl_warn("Unable to locate cursor falling back to static cursor\n");
	swl_cursor_t *cursor = calloc(1, sizeof(swl_cursor_t));
	cursor->images = calloc(1, sizeof(swl_cursor_image_t));
	uint32_t *data = calloc(4, size * size);

	cursor->images->pixels = data;
	cursor->images->width = size;
	cursor->images->height = size;
	cursor->count = 1;
	for (uint32_t y = 0; y < size; ++y) {
		for (uint32_t x = 0; x < size; ++x) {
			data[y * size] = 0xffffffff;
			data[x] = 0xffffffff;
			data[y * size + y] = 0xffffffff;
		}
	}

	return cursor;
}

static void init_cursor_image_from_ximage(xcursor_image_chunk_t *image, swl_cursor_image_t *cimage) {
	cimage->height = image->height;
	cimage->width = image->width;
	cimage->delay = image->delay;
	cimage->pixels = calloc(4, cimage->height * cimage->width);

	memcpy(cimage->pixels, image->pixels, 4 * cimage->width * cimage->height);
}

static swl_cursor_t *create_cursor_from_file_buffer(void *data, uint32_t pref_size) {
	uint32_t nsize = 0;
	xcursor_header_t *header = data;
	swl_cursor_t *cursor = calloc(1, sizeof(swl_cursor_t));

	if(memcmp(&header->magic, XCURSOR_MAGIC, XCURSOR_MAGIC_LEN) != 0) {
		swl_warn("Opened file but it's not a XCursors file\n");
		return NULL;
	}

	for(uint32_t i = 0; i < header->ntoc; ++i) {
		if(pref_size == header->toc[i].subtype) {
			nsize++;
		}
	}

	if(nsize == 0) return NULL;
	cursor->images = calloc(nsize, sizeof(swl_cursor_image_t));
	nsize = 0;
	for(uint32_t i = 0; i < header->ntoc; i++) {
		if(pref_size == header->toc[i].subtype) {
			init_cursor_image_from_ximage(data + header->toc[i].position, &cursor->images[nsize]);
			nsize++;
		}	
	}

	cursor->count = nsize;
	return cursor;
}

swl_cursor_t *swl_try_xcursor_file(FILE *fp, uint32_t pref_size) {
	long size = 0;
	void *data, *ret;
	xcursor_header_t *header;
	xcursor_image_chunk_t *xcursor_image;

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	data = calloc(1, size);
	/*Load the full file into memory*/
	fread(data, 1, size, fp);
	header = data;

	if(memcmp(&header->magic, XCURSOR_MAGIC, XCURSOR_MAGIC_LEN) != 0) {
		swl_warn("Opened file but it's not a XCursors file\n");
		free(data);
		return NULL;
	}

	swl_cursor_t *cursor = create_cursor_from_file_buffer(data, pref_size);
	free(data);
	return cursor;	
}

swl_cursor_t *swl_open_xcursor(const char *theme, const char *name, uint32_t pref_size) {
	char *search_path = NULL;
	char *env_path = getenv("XCURSOR_PATH");
	char *path = NULL;
	FILE *fp = NULL;
	uint32_t len = 0;
	swl_cursor_t *data = NULL;

	if(env_path) {
		len = snprintf(NULL, 0, "%s:%s", env_path, SWL_XCURSOR_DIRECTORIES);
		search_path = calloc(1, len+1);
		snprintf(search_path, len+1, "%s:%s", env_path, SWL_XCURSOR_DIRECTORIES);
	} else {
		search_path = strdup(SWL_XCURSOR_DIRECTORIES);
	}

	char *token = strtok(search_path, ":");
	while(token) {
		len = snprintf(NULL, 0, "%s/%s/cursors/%s", token, theme, name);

		path = calloc(1, len + 1);
		snprintf(path, len + 1, "%s/%s/cursors/%s", token, theme, name);
		swl_debug("%s\n", path);

		fp = fopen(path, "r");
		if(fp) {
			data = swl_try_xcursor_file(fp, pref_size);
			fclose(fp);
			fp = NULL;
			if(data) return data;
		} else {
			swl_warn("%m\n");
		}
		

		token = strtok(NULL, ":");
	}
	return fallback_cursor(pref_size);
}

swl_cursor_t *swl_open_xcursor_env() {
	const char *name = getenv("CURSOR_NAME");
	const char *theme = getenv("XCURSOR_THEME");
	uint32_t size = 24;
	unsigned long long env_size = 0;


	name = name ? name : "left_ptr";
	theme = theme ? theme : "default";

	swl_open_xcursor(theme, name, size);
}
