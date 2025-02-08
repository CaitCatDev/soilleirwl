#include <stdint.h>
#include <stdlib.h>
#include <wayland-server-core.h>
#include <wayland-server-protocol.h>

#include <soilleirwl/logger.h>
#include <soilleirwl/interfaces/swl_output.h>
#include <soilleirwl/interfaces/swl_zxdg_output.h>

#include <private/xdg-output-unstable-v1-server.h>
#include <wayland-server.h>
#include <wayland-util.h>

#define SWL_ZXDG_OUTPUT_MANAGER_VERSION 3

typedef struct _swl_zxdg_output {
	struct wl_resource *wl_output;
	struct wl_resource *xdg_output;

	struct wl_list link;
} swl_zxdg_output_t;

struct _swl_zxdg_output_manager {
	struct wl_global *global;

	struct wl_list list;
};

static void swl_zxdg_output_get_logical_size(uint32_t scale, uint32_t transform,
		int32_t w, int32_t h, int32_t *lw, int32_t *lh) {
	if(!lh || !lw) {
		swl_warn("Bad input h and w set to NULL\n");
		return;
	}

	switch (transform) {
		/*Monitor is mounted vertically*/
		case WL_OUTPUT_TRANSFORM_270:
		case WL_OUTPUT_TRANSFORM_90:
		case WL_OUTPUT_TRANSFORM_FLIPPED_90:
		case WL_OUTPUT_TRANSFORM_FLIPPED_270:
			*lw = h;
			*lh = w;
			break;
		/*Monitor is horizontal or unknown(assume horizontal)*/
		default:
			*lh = h;
			*lw = w;
	}

	/* Scale the monitor to global compositor space coords
	 * I think that just means dividing i.e. with a scale
	 * of two we treat it as tho it's a half the resolution
	 * in terms of buffer
	 */
	*lh /= scale;
	*lw /= scale;
}

/*TODO we need to support resending description, log pos and size if they should change*/
static void swl_zxdg_output_destroy(struct wl_client *client, struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static void swl_zxdg_output_resource_destroy(struct wl_resource *resource) {
	free(wl_resource_get_user_data(resource));
}

static struct zxdg_output_v1_interface swl_zxdg_output_v1_impl = {
	.destroy = swl_zxdg_output_destroy,
};

static void swl_zxdg_output_manager_destroy(struct wl_client *client, struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static void swl_zxdg_output_manager_resource_destroy(struct wl_resource *resource) {

}

static void swl_zxdg_output_manager_get_output(struct wl_client *client, struct wl_resource *resource, uint32_t id, struct wl_resource *wl_output) {
	swl_zxdg_output_manager_t *manager = wl_resource_get_user_data(resource);
	swl_output_t *output = wl_resource_get_user_data(wl_output);
	int32_t lw, lh;
	int version = wl_resource_get_version(resource);

	struct wl_resource *output_res = wl_resource_create(client, &zxdg_output_v1_interface, version, id);
	wl_resource_set_implementation(resource, &swl_zxdg_output_v1_impl, NULL, swl_zxdg_output_resource_destroy);

	/*XD FIX that name*/
	swl_zxdg_output_get_logical_size(output->scale, output->tranform, output->mode.width,
			output->mode.height, &lw, &lh);
	zxdg_output_v1_send_logical_size(output_res, lw, lh);
	zxdg_output_v1_send_logical_position(output_res, output->x, output->y);
	/*These are deprecated but wayland protocols says I must still "support" this event
	 * which I assume means I have to send it to the client and worst case if a client doesn't
	 * use this event it'll just do nothing
	 */
	if(version >= ZXDG_OUTPUT_V1_DESCRIPTION_SINCE_VERSION) {
		zxdg_output_v1_send_description(output_res, output->description);
	}
	if(version >= ZXDG_OUTPUT_V1_NAME_SINCE_VERSION) {
		zxdg_output_v1_send_name(output_res, output->name);
	}
	if(version < 3) {
		zxdg_output_v1_send_done(output_res);
	} else {
		wl_output_send_done(wl_output);
	}
}


static struct zxdg_output_manager_v1_interface swl_zxdg_output_manager_impl = {
	.destroy = swl_zxdg_output_manager_destroy,
	.get_xdg_output = swl_zxdg_output_manager_get_output,
};

static void swl_zxdg_output_manager_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id) {
	struct wl_resource *resource = wl_resource_create(client, &zxdg_output_manager_v1_interface, version, id);
	wl_resource_set_implementation(resource, &swl_zxdg_output_manager_impl, data, swl_zxdg_output_resource_destroy);
}

swl_zxdg_output_manager_t *swl_zxdg_output_manager_create(struct wl_display *display) {
	swl_zxdg_output_manager_t *manager = calloc(1, sizeof(swl_zxdg_output_manager_t));

	manager->global = wl_global_create(display, &zxdg_output_manager_v1_interface, SWL_ZXDG_OUTPUT_MANAGER_VERSION, manager, swl_zxdg_output_manager_bind);

	return manager;
}
