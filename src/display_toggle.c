/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 *
 * Custom ZMK behavior: display_toggle
 *
 * Toggles the OLED status display on/off.
 *
 * - Pressing both SPACE keys simultaneously (via a ZMK combo defined in
 *   lily58.keymap) triggers this behavior.
 * - On press: if the display is currently blanked, unblank it; otherwise
 *   blank it. State is tracked internally so consecutive presses always
 *   toggle correctly even when the activity state machine is also blanking
 *   the display automatically.
 * - The 60-second auto-off is handled by ZMK's existing activity state
 *   machine via CONFIG_ZMK_DISPLAY_BLANK_ON_IDLE=y in lily58.conf.
 */

#define DT_DRV_COMPAT lily58pro_behavior_display_toggle

#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/logging/log.h>

#include <drivers/behavior.h>
#include <zmk/behavior.h>

LOG_MODULE_REGISTER(display_toggle, CONFIG_LOG_DEFAULT_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

static const struct device *display_dev;
static bool display_is_on = false;

static int get_display_device(void) {
    if (display_dev != NULL && device_is_ready(display_dev)) {
        return 0;
    }

#if DT_HAS_CHOSEN(zephyr_display)
    display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
#else
    display_dev = NULL;
#endif

    if (display_dev == NULL || !device_is_ready(display_dev)) {
        LOG_WRN("display_toggle: display device not ready");
        return -ENODEV;
    }
    return 0;
}

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    if (get_display_device() != 0) {
        return ZMK_BEHAVIOR_OPAQUE;
    }

    if (display_is_on) {
        display_blanking_on(display_dev);
        display_is_on = false;
        LOG_DBG("display_toggle: blanked");
    } else {
        display_blanking_off(display_dev);
        display_is_on = true;
        LOG_DBG("display_toggle: unblanked");
    }
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                       struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_display_toggle_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};

BEHAVIOR_DT_INST_DEFINE(0, NULL, NULL, NULL, NULL, POST_KERNEL,
                        CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
                        &behavior_display_toggle_driver_api);

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
