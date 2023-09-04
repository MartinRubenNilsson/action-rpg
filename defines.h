#pragma once

// The view defines the area of the game world that is visible to the player.
// Its size is measured in pixels, and in this game it is always 320x180.

#define WINDOW_VIEW_WIDTH 320u
#define WINDOW_VIEW_HEIGHT 180u

// The window size is limited to a multiple of the view size,
// which ensures that pixels are not stretched or squashed.
// This multiple is called the window scale.

#define WINDOW_SCALE_MIN 3u
#define WINDOW_SCALE_MAX 7u