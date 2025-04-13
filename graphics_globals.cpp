#include "stdafx.h"
#include "graphics.h"
#include "graphics_globals.h"
#include "graphics_vertices.h"
#include "filesystem.h"

namespace graphics {
	eastl::vector<graphics::Vertex> temp_vertices;

	Handle<Shader> fullscreen_shader;
	Handle<Shader> gaussian_blur_hor_shader;
	Handle<Shader> gaussian_blur_ver_shader;
	Handle<Shader> screen_transition_shader;
	Handle<Shader> shockwave_shader;
	Handle<Shader> grass_shader;
	Handle<Shader> sprite_shader;
	Handle<Shader> shape_shader;
	Handle<Shader> text_shader;
	Handle<Shader> ui_shader;
	Handle<Shader> player_outfit_shader;

	Handle<Framebuffer> game_framebuffer_0;
	Handle<Framebuffer> game_framebuffer_1;
	Handle<Framebuffer> player_outfit_framebuffer;

	Handle<Buffer> dynamic_vertex_buffer;
	Handle<Buffer> dynamic_index_buffer;
	Handle<Buffer> frame_uniform_buffer;
	Handle<Buffer> ui_uniform_buffer;
	Handle<Buffer> sprite_uniform_buffer;
	Handle<Buffer> player_outfit_uniform_buffer;

	Handle<Texture> error_texture; // 32x32 magenta-black checkerboard
	Handle<Texture> white_texture; // 1x1 white texture

	Handle<Sampler> nearest_sampler;
	Handle<Sampler> linear_sampler;

	void _initialize_shaders() {
		std::string fullscreen_vert;
		std::string fullscreen_frag;
		std::string gaussian_blur_hor_frag;
		std::string gaussian_blur_ver_frag;
		std::string screen_transition_frag;
		std::string shockwave_frag;
		std::string sprite_vert;
		std::string sprite_frag;
		std::string grass_vert;
		std::string shape_vert;
		std::string shape_frag;
		std::string text_frag;
		std::string ui_vert;
		std::string ui_frag;
		std::string player_outfit_frag;

		filesystem::read_text_file("assets/shaders/fullscreen.vert", fullscreen_vert);
		filesystem::read_text_file("assets/shaders/fullscreen.frag", fullscreen_frag);
		filesystem::read_text_file("assets/shaders/gaussian_blur_hor.frag", gaussian_blur_hor_frag);
		filesystem::read_text_file("assets/shaders/gaussian_blur_ver.frag", gaussian_blur_ver_frag);
		filesystem::read_text_file("assets/shaders/screen_transition.frag", screen_transition_frag);
		filesystem::read_text_file("assets/shaders/shockwave.frag", shockwave_frag);
		filesystem::read_text_file("assets/shaders/sprite.vert", sprite_vert);
		filesystem::read_text_file("assets/shaders/sprite.frag", sprite_frag);
		filesystem::read_text_file("assets/shaders/grass.vert", grass_vert);
		filesystem::read_text_file("assets/shaders/shape.vert", shape_vert);
		filesystem::read_text_file("assets/shaders/shape.frag", shape_frag);
		filesystem::read_text_file("assets/shaders/text.frag", text_frag);
		filesystem::read_text_file("assets/shaders/ui.vert", ui_vert);
		filesystem::read_text_file("assets/shaders/ui.frag", ui_frag);
		filesystem::read_text_file("assets/shaders/player_outfit.frag", player_outfit_frag);

		fullscreen_shader = create_shader({
			.debug_name = "fullscreen shader",
			.vs_source = fullscreen_vert,
			.fs_source = fullscreen_frag
		});
		gaussian_blur_hor_shader = create_shader({
			.debug_name = "gaussian blur horizontal shader",
			.vs_source = fullscreen_vert,
			.fs_source = gaussian_blur_hor_frag
		});
		gaussian_blur_ver_shader = create_shader({
			.debug_name = "gaussian blur vertical shader",
			.vs_source = fullscreen_vert,
			.fs_source = gaussian_blur_ver_frag
		});
		screen_transition_shader = create_shader({
			.debug_name = "screen transition shader",
			.vs_source = fullscreen_vert,
			.fs_source = screen_transition_frag
		});
		shockwave_shader = create_shader({
			.debug_name = "shockwave shader",
			.vs_source = fullscreen_vert,
			.fs_source = shockwave_frag
		});
		sprite_shader = create_shader({
			.debug_name = "sprite shader",
			.vs_source = sprite_vert,
			.fs_source = sprite_frag
		});
		grass_shader = create_shader({
			.debug_name = "grass shader",
			.vs_source = grass_vert,
			.fs_source = sprite_frag
		});
		shape_shader = create_shader({
			.debug_name = "shape shader",
			.vs_source = shape_vert,
			.fs_source = shape_frag
		});
		text_shader = create_shader({
			.debug_name = "text shader",
			.vs_source = sprite_vert,
			.fs_source = text_frag
		});
		ui_shader = create_shader({
			.debug_name = "ui shader",
			.vs_source = ui_vert,
			.fs_source = ui_frag
		});
		player_outfit_shader = create_shader({
			.debug_name = "player outfit shader",
			.vs_source = fullscreen_vert,
			.fs_source = player_outfit_frag
		});
	}

	void initialize_globals() {
		_initialize_shaders();

		game_framebuffer_0 = create_framebuffer({
			.debug_name = "game framebuffer 0" });
		attach_framebuffer_texture(game_framebuffer_0, create_texture({
			.debug_name = "game framebuffer texture 0",
			.width = GAME_FRAMEBUFFER_WIDTH,
			.height = GAME_FRAMEBUFFER_HEIGHT,
			.format = Format::RGBA8_UNORM
		}));
		game_framebuffer_1 = create_framebuffer({
			.debug_name = "game framebuffer 1" });
		attach_framebuffer_texture(game_framebuffer_1, create_texture({
			.debug_name = "game framebuffer texture 1",
			.width = GAME_FRAMEBUFFER_WIDTH,
			.height = GAME_FRAMEBUFFER_HEIGHT,
			.format = Format::RGBA8_UNORM
		}));
		player_outfit_framebuffer = create_framebuffer({
			.debug_name = "player outfit framebuffer" });
		attach_framebuffer_texture(player_outfit_framebuffer, create_texture({
			.debug_name = "player outfit texture",
			.width = 1024, // size of player tileset
			.height = 1024, // size of player tileset
			.format = Format::RGBA8_UNORM
		}));

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
				.initial_data = error_texture_data });
		}
		{
			unsigned char white_texture_data[4] = { 255, 255, 255, 255 };
			white_texture = create_texture({
				.debug_name = "white texture",
				.width = 1,
				.height = 1,
				.format = Format::RGBA8_UNORM,
				.initial_data = white_texture_data });
		}

		nearest_sampler = create_sampler({
			.debug_name = "nearest sampler",
			.filter = Filter::Nearest
		});
		linear_sampler = create_sampler({
			.debug_name = "linear sampler",
			.filter = Filter::Linear,
			.wrap = Wrap::ClampToEdge
		});

		bind_vertex_buffer(0, dynamic_vertex_buffer, sizeof(Vertex));
		bind_index_buffer(dynamic_index_buffer);
		bind_uniform_buffer(0, frame_uniform_buffer);
		bind_sampler(0, nearest_sampler);
	}
}