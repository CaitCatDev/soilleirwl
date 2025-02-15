#pragma once
/*
 * Based upon man page info from here:
 * https://www.x.org/archive/X11R7.7/doc/man/man3/Xcursor.3.xhtml
 */

#include <stdint.h>
#include <sys/types.h>

/*ENV VARIABLES:
 * XCURSOR_THEME (This is standard and is used in most Xcursor related stuff
 * ensuring if a user uses a certain XCURSOR_THEME we will honor it)
 * XCURSOR_SIZE Desired cursor size of XCURSOR_THEME.
 * XCURSOR_PATH (Again we use this because it's common for all Xcursor stuff so users
 * probably already have this set adds a path to be searched)
 *
 * By default we will scan in the following order '$XCURSOR_PATH:~/.icons/:/usr/share/icons/'
 *
 * If No theme is specified we just go to fallback cursor theme of default. If default is not on your system
 * we will fallback to a custom cursor
 * TODO: Improve fallback
 */

#define XCURSOR_MAGIC "Xcur"
#define XCURSOR_MAGIC_LEN 4

#define XCURSOR_TYPE_COMMONENT 0xfffe0001
#define XCURSOR_TYPE_IMAGE 0xfffd0002

#define XCURSOR_SUBTYPE_COPYRIGHT 0x1
#define XCURSOR_SUBTYPE_LICENSE 0x2
#define XCURSOR_SUBTYPE_OTHER 0x3

typedef struct {
	uint32_t type;
	uint32_t subtype;
	uint32_t position;
} xcursor_toc_t;

typedef struct {
	uint32_t size;
	uint32_t type;
	uint32_t subtype;
	uint32_t version;
} xcursor_chunk_t;

typedef struct {
	xcursor_chunk_t header;
	uint32_t length;
	uint8_t string[];
} xcursor_comment_chunk_t;

typedef struct {
	xcursor_chunk_t chunk;
	uint32_t width;
	uint32_t height;
	uint32_t xhot;
	uint32_t yhot;
	uint32_t delay;
	uint32_t pixels[]; /*width*height*4*/
} xcursor_image_chunk_t;

typedef struct {
	uint32_t magic;
	uint32_t bytes;
	uint32_t version;
	uint32_t ntoc;
	xcursor_toc_t toc[]; /*Sizeof toc == ntoc*/	
} xcursor_header_t;

typedef struct {
	uint32_t width, height;
	uint32_t xhot, yhot;
	uint32_t delay;

	uint32_t *pixels;
} swl_cursor_image_t;

/*TODO Animated Cursors*/
typedef struct {
	swl_cursor_image_t *images;
	uint32_t count;
} swl_cursor_t;

swl_cursor_t *swl_open_xcursor(const char *theme, const char *name, uint32_t pref_size);
