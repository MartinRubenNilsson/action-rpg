#include "stdafx.h"
#include "graphics.h"
#include "graphics_globals.h"
#include "graphics_vertices.h"
#include "filesystem.h"

namespace graphics {
	eastl::vector<graphics::Vertex> temp_vertices;

	Handle<VertexShader> fullscreen_vert;
	Handle<FragmentShader> fullscreen_frag;
	Handle<FragmentShader> gaussian_blur_hor_frag;
	Handle<FragmentShader> gaussian_blur_ver_frag;
	Handle<FragmentShader> screen_transition_frag;
	Handle<FragmentShader> shockwave_frag;
	Handle<FragmentShader> darkness_frag;
	Handle<VertexShader> sprite_vert;
	Handle<FragmentShader> sprite_frag;
	Handle<VertexShader> grass_vert;
	Handle<VertexShader> shape_vert;
	Handle<FragmentShader> shape_frag;
	Handle<FragmentShader> text_frag;
	Handle<VertexShader> ui_vert;
	Handle<FragmentShader> ui_frag;
	Handle<FragmentShader> player_outfit_frag;

	Handle<Framebuffer> game_framebuffer_0;
	Handle<Framebuffer> game_framebuffer_1;
	Handle<Framebuffer> player_outfit_framebuffer;

	Handle<Buffer> dynamic_vertex_buffer;
	Handle<Buffer> dynamic_index_buffer;
	Handle<Buffer> frame_uniform_buffer;
	Handle<Buffer> ui_uniform_buffer;
	Handle<Buffer> sprite_uniform_buffer;
	Handle<Buffer> player_outfit_uniform_buffer;
	Handle<Buffer> darkness_uniform_buffer;
	Handle<Buffer> screen_transition_uniform_buffer;
	Handle<Buffer> shockwave_uniform_buffer;

	Handle<Texture> error_texture; // 32x32 magenta-black checkerboard
	Handle<Texture> white_texture; // 1x1 white texture

	Handle<Sampler> nearest_sampler;
	Handle<Sampler> linear_sampler;

	void _initialize_shaders() {
		std::vector<unsigned char> shader_bytecode;
		filesystem::read_binary_file("assets/shaders/fullscreen.vert.spv", shader_bytecode);
		fullscreen_vert = create_vertex_shader({
			.debug_name = "fullscreen vertex shader",
			.bytecode = shader_bytecode
		});
		filesystem::read_binary_file("assets/shaders/fullscreen.frag.spv", shader_bytecode);
		fullscreen_frag = create_fragment_shader({
			.debug_name = "fullscreen fragment shader",
			.bytecode = shader_bytecode
		});
		filesystem::read_binary_file("assets/shaders/gaussian_blur_hor.frag.spv", shader_bytecode);
		gaussian_blur_hor_frag = create_fragment_shader({
			.debug_name = "gaussian blur horizontal fragment shader",
			.bytecode = shader_bytecode
		});
		filesystem::read_binary_file("assets/shaders/gaussian_blur_ver.frag.spv", shader_bytecode);
		gaussian_blur_ver_frag = create_fragment_shader({
			.debug_name = "gaussian blur vertical fragment shader",
			.bytecode = shader_bytecode
		});
		filesystem::read_binary_file("assets/shaders/screen_transition.frag.spv", shader_bytecode);
		screen_transition_frag = create_fragment_shader({
			.debug_name = "screen transition fragment shader",
			.bytecode = shader_bytecode
		});
		filesystem::read_binary_file("assets/shaders/shockwave.frag.spv", shader_bytecode);
		shockwave_frag = create_fragment_shader({
			.debug_name = "shockwave fragment shader",
			.bytecode = shader_bytecode
		});
		filesystem::read_binary_file("assets/shaders/darkness.frag.spv", shader_bytecode);
		darkness_frag = create_fragment_shader({
			.debug_name = "darkness fragment shader",
			.bytecode = shader_bytecode
		});
		filesystem::read_binary_file("assets/shaders/sprite.vert.spv", shader_bytecode);
		sprite_vert = create_vertex_shader({
			.debug_name = "sprite vertex shader",
			.bytecode = shader_bytecode
		});
		filesystem::read_binary_file("assets/shaders/sprite.frag.spv", shader_bytecode);
		sprite_frag = create_fragment_shader({
			.debug_name = "sprite fragment shader",
			.bytecode = shader_bytecode
		});
		filesystem::read_binary_file("assets/shaders/grass.vert.spv", shader_bytecode);
		grass_vert = create_vertex_shader({
			.debug_name = "grass vertex shader",
			.bytecode = shader_bytecode
		});
		filesystem::read_binary_file("assets/shaders/shape.vert.spv", shader_bytecode);
		shape_vert = create_vertex_shader({
			.debug_name = "shape vertex shader",
			.bytecode = shader_bytecode
		});
		filesystem::read_binary_file("assets/shaders/shape.frag.spv", shader_bytecode);
		shape_frag = create_fragment_shader({
			.debug_name = "shape fragment shader",
			.bytecode = shader_bytecode
		});
		filesystem::read_binary_file("assets/shaders/text.frag.spv", shader_bytecode);
		text_frag = create_fragment_shader({
			.debug_name = "text fragment shader",
			.bytecode = shader_bytecode
		});
		filesystem::read_binary_file("assets/shaders/ui.vert.spv", shader_bytecode);
		ui_vert = create_vertex_shader({
			.debug_name = "ui vertex shader",
			.bytecode = shader_bytecode
		});
		filesystem::read_binary_file("assets/shaders/ui.frag.spv", shader_bytecode);
		ui_frag = create_fragment_shader({
			.debug_name = "ui fragment shader",
			.bytecode = shader_bytecode
		});
		filesystem::read_binary_file("assets/shaders/player_outfit.frag.spv", shader_bytecode);
		player_outfit_frag = create_fragment_shader({
			.debug_name = "player outfit fragment shader",
			.bytecode = shader_bytecode
		});
	}

	void _initialize_framebuffers() {
		game_framebuffer_0 = create_framebuffer({
			.debug_name = "game framebuffer 0"
		});
		attach_framebuffer_texture(game_framebuffer_0, create_texture({
			.debug_name = "game framebuffer texture 0",
			.width = GAME_FRAMEBUFFER_WIDTH,
			.height = GAME_FRAMEBUFFER_HEIGHT,
			.format = Format::RGBA8_UNORM
		}));

		game_framebuffer_1 = create_framebuffer({
			.debug_name = "game framebuffer 1"
		});
		attach_framebuffer_texture(game_framebuffer_1, create_texture({
			.debug_name = "game framebuffer texture 1",
			.width = GAME_FRAMEBUFFER_WIDTH,
			.height = GAME_FRAMEBUFFER_HEIGHT,
			.format = Format::RGBA8_UNORM
		}));

		player_outfit_framebuffer = create_framebuffer({
			.debug_name = "player outfit framebuffer"
		});
		attach_framebuffer_texture(player_outfit_framebuffer, create_texture({
			.debug_name = "player outfit texture",
			.width = 1024, // size of player tileset
			.height = 1024, // size of player tileset
			.format = Format::RGBA8_UNORM
		}));
	}

	void _initialize_buffers() {
		dynamic_vertex_buffer = create_buffer({
			.debug_name = "dynamic vertex buffer",
			.size = 8192 * sizeof(Vertex), // 8192 is an initial estimate
			.type = BufferType::VertexBuffer,
			.dynamic = true
		});
		dynamic_index_buffer = create_buffer({
			.debug_name = "dynamic index buffer",
			.size = 8192 * sizeof(unsigned int), // 8192 is an initial estimate
			.type = BufferType::IndexBuffer,
			.dynamic = true
		});
		frame_uniform_buffer = create_buffer({
			.debug_name = "frame uniform buffer",
			.size = sizeof(FrameUniformBlock),
			.type = BufferType::UniformBuffer,
			.dynamic = true
		});
		ui_uniform_buffer = create_buffer({
			.debug_name = "ui uniform buffer",
			.size = sizeof(UiUniformBlock),
			.type = BufferType::UniformBuffer,
			.dynamic = true
		});
		sprite_uniform_buffer = create_buffer({
			.debug_name = "sprite uniform buffer",
			.size = 256 * 256, // estimate we won't need more than 256 custom blocks on screen at same time
			.type = BufferType::UniformBuffer,
			.dynamic = true
		});
		player_outfit_uniform_buffer = create_buffer({
			.debug_name = "player outfit uniform buffer",
			.size = sizeof(PlayerOutfitUniformBlock),
			.type = BufferType::UniformBuffer,
			.dynamic = true
		});
		darkness_uniform_buffer = create_buffer({
			.debug_name = "darkness uniform buffer",
			.size = sizeof(DarknessUniformBlock),
			.type = BufferType::UniformBuffer,
			.dynamic = true
		});
		screen_transition_uniform_buffer = create_buffer({
			.debug_name = "screen transition uniform buffer",
			.size = sizeof(ScreenTransitionUniformBlock),
			.type = BufferType::UniformBuffer,
			.dynamic = true
		});
		shockwave_uniform_buffer = create_buffer({
			.debug_name = "shockwave uniform buffer",
			.size = sizeof(ShockwaveUniformBlock),
			.type = BufferType::UniformBuffer,
			.dynamic = true
		});
	}

	void _initialize_textures() {
		{
			constexpr unsigned int ERROR_TEXTURE_SIZE = 32;
			unsigned char error_texture_data[ERROR_TEXTURE_SIZE * ERROR_TEXTURE_SIZE * 4];
			//Make a magenta-and-black checkerboard pattern
			for (unsigned int i = 0; i < ERROR_TEXTURE_SIZE; ++i) {
				for (unsigned int j = 0; j < ERROR_TEXTURE_SIZE; ++j) {
					unsigned int k = (i * ERROR_TEXTURE_SIZE + j) * 4;
					unsigned char value = (i + j) % 2 == 0 ? 255 : 0;
					error_texture_data[k + 0] = value;
					error_texture_data[k + 1] = 0;
					error_texture_data[k + 2] = value;
					error_texture_data[k + 3] = 255;
				}
			}
			error_texture = create_texture({
				.debug_name = "error texture",
				.width = ERROR_TEXTURE_SIZE,
				.height = ERROR_TEXTURE_SIZE,
				.format = Format::RGBA8_UNORM,
				.initial_data = error_texture_data
			});
		}
		{
			unsigned char white_texture_data[4] = { 255, 255, 255, 255 };
			white_texture = create_texture({
				.debug_name = "white texture",
				.width = 1,
				.height = 1,
				.format = Format::RGBA8_UNORM,
				.initial_data = white_texture_data
			});
		}
	}

	void _initialize_samplers() {
		nearest_sampler = create_sampler({
			.debug_name = "nearest sampler",
			.filter = Filter::Nearest
		});
		linear_sampler = create_sampler({
			.debug_name = "linear sampler",
			.filter = Filter::Linear,
			.wrap = Wrap::ClampToEdge
		});
	}

	void _bind_globals() {
		bind_vertex_buffer(0, dynamic_vertex_buffer, sizeof(Vertex));
		bind_index_buffer(dynamic_index_buffer);
		bind_uniform_buffer(0, frame_uniform_buffer);
		bind_sampler(0, nearest_sampler);
	}

	void initialize_globals() {
		_initialize_shaders();
		_initialize_framebuffers();
		_initialize_buffers();
		_initialize_textures();
		_initialize_samplers();
		_bind_globals();
	}
}