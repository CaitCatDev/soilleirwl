#pragma once

#include <wayland-server-core.h>

typedef struct _swl_zxdg_output_manager swl_zxdg_output_manager_t;

swl_zxdg_output_manager_t *swl_zxdg_output_manager_create(struct wl_display *display);
