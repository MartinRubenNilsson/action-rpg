#pragma once

// fwd.h - forward declarations

typedef void* (*GLADloadproc)(const char* name);

namespace window {
	struct Event;
}

namespace audio {
	struct Event;
}

namespace graphics {
	struct VertexShader;
	struct FragmentShader;
	struct Buffer;
	struct Texture;
	struct Sampler;
	struct Framebuffer;
}

namespace fonts {
	struct Font;
}

namespace tiled {
	struct Map;
	struct Tileset;
	struct Object;
}
