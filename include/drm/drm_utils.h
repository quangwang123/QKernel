/* SPDX-License-Identifier: MIT */
/*
 * Function prototypes for misc. drm utility functions.
 * Specifically this file is for function prototypes for functions which
 * may also be used outside of drm code (e.g. in fbdev drivers).
 *
 * Copyright (C) 2017 Hans de Goede <hdegoede@redhat.com>
 */

#ifndef __DRM_UTILS_H__
#define __DRM_UTILS_H__

#if IS_ENABLED(CONFIG_DRM_PANEL_ORIENTATION_QUIRKS)
int drm_get_panel_orientation_quirk(int width, int height);
#else
static inline int drm_get_panel_orientation_quirk(int width, int height)
{
	return 0;
}
#endif

#endif
