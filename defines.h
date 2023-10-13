#pragma once

#define PIXELS_PER_METER 16.f
#define METERS_PER_PIXEL (1.f / PIXELS_PER_METER)

// The view defines the area of the game world that is visible to the player.

#define VIEW_PIXEL_WIDTH 320u
#define VIEW_PIXEL_HEIGHT 180u
#define VIEW_WIDTH (VIEW_PIXEL_WIDTH * METERS_PER_PIXEL)
#define VIEW_HEIGHT (VIEW_PIXEL_HEIGHT * METERS_PER_PIXEL)

// The window size is limited to a multiple of the view size,
// which ensures that pixels are not stretched or squashed.
// This multiple is called the window scale.

#define WINDOW_SCALE_MIN 3u
#define WINDOW_SCALE_MAX 7u