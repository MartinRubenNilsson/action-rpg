#include "stdafx.h"
#include "steam.h"
#include "filesystem.h"
#include "window.h"
#include "window_events.h"
#include "audio.h"
#include "ui.h"
#include "ui_menus.h"
#include "ui_textbox.h"
#include "map.h"
#include "ecs.h"
#include "console.h"
#include "tiled.h"
#include "background.h"
#include "postprocessing.h"
#include "settings.h"
#include "graphics.h"
#include "graphics_globals.h"
#include "shapes.h"
#include "sprites.h"
#include "renderdoc.h"
#include "imgui_backends.h"

int main(int argc, char* argv[])
{
    if (steam::restart_app_if_necessary()) {
        return EXIT_FAILURE;
    }
    steam::initialize(); // Fails silently if Steam is not running.
    filesystem::initialize();
#ifdef DEBUG_RENDERDOC
    renderdoc::initialize();
#endif
    window::initialize();
#ifdef DEBUG_IMGUI
    imgui_backends::initialize(window::get_glfw_window());
#endif
    graphics::initialize();
    graphics::initialize_globals();
    console::initialize();
    audio::initialize();
    ui::initialize();
    ecs::initialize();

    for (const filesystem::File& file : filesystem::get_files_in_directory("assets/audio/banks")) {
        if (file.format != filesystem::FileFormat::FmodStudioBank) continue;
        audio::load_bank_from_file(file.path);
    }
    for (const filesystem::File& file : filesystem::get_files_in_directory("assets/fonts")) {
        if (file.format != filesystem::FileFormat::TrueTypeFont) continue;
        ui::load_font_from_file(file.path);
    }
    for (const filesystem::File& file : filesystem::get_files_in_directory("assets/ui")) {
        if (file.format != filesystem::FileFormat::RmlUiDocument) continue;
        ui::load_document_from_file(file.path);
    }
    for (const filesystem::File& file : filesystem::get_files_in_directory("assets/tiled")) {
        if (file.format != filesystem::FileFormat::TiledTileset) continue;
        tiled::load_tileset_from_file(file.path);
    }
    for (const filesystem::File& file : filesystem::get_files_in_directory("assets/tiled")) {
        if (file.format != filesystem::FileFormat::TiledTemplate) continue;
        tiled::load_template_from_file(file.path);
    }
    for (const filesystem::File& file : filesystem::get_files_in_directory("assets/tiled")) {
        if (file.format != filesystem::FileFormat::TiledMap) continue;
        tiled::load_map_from_file(file.path);
    }

    ui::add_event_listeners(); // Must come after loading RML documents.

    settings::load_from_file();
    settings::apply();

    window::set_visible(true);

    // PREPARE FOR GAME LOOP

#ifdef _DEBUG
    console::execute(argc, argv);
#else
    console::execute("execute_script assets/scripts/martin_debug.script");
#endif

    bool debug_stats = false;
    bool debug_textboxes = false;
    bool debug_textures = false;

    // GAME LOOP

    double last_elapsed_time = window::get_elapsed_time();

    while (!window::should_close()) {

        const double elapsed_time = window::get_elapsed_time();
        const float main_delta_time = (float)(elapsed_time - last_elapsed_time);
        last_elapsed_time = elapsed_time;

        steam::run_message_loop();
        window::poll_events();

#ifdef DEBUG_IMGUI
        imgui_backends::new_frame();
#endif

        // PROCESS WINDOW EVENTS
        {
            window::Event ev;
            while (window::get_next_event(ev)) {
                if (ev.type == window::EventType::WindowClose) {
                    window::set_should_close(true);
                }  else if (ev.type == window::EventType::KeyPress) {
#ifdef _DEBUG
                    if (ev.key.code == window::Key::GraveAccent) {
                        console::toggle_visible();
                    } else if (ev.key.code == window::Key::F1) {
                        debug_stats = !debug_stats;
                    } else if (ev.key.code == window::Key::F2) {
                        debug_textboxes = !debug_textboxes;
                    } else if (ev.key.code == window::Key::F3) {
                        ecs::debug_flags ^= ecs::DEBUG_PHYSICS;
                    } else if (ev.key.code == window::Key::F4) {
                        ecs::debug_flags ^= ecs::DEBUG_AI;
                    } else if (ev.key.code == window::Key::F5) {
                        ecs::debug_flags ^= ecs::DEBUG_PLAYER;
                    } else if (ev.key.code == window::Key::F6) {
                        ui::debug = !ui::debug;
                    } else if (ev.key.code == window::Key::F7) {
                        map::debug = !map::debug;
                    } else if (ev.key.code == window::Key::F8) {
                        debug_textures = !debug_textures;
                    }
#endif // _DEBUG
                }

#ifdef DEBUG_IMGUI
                if (ev.type == window::EventType::KeyPress && ImGui::GetIO().WantCaptureKeyboard) {
                    continue;
                }
#endif
                console::process_window_event(ev);
                ui::process_window_event(ev);
                if (!ui::is_menu_or_textbox_visible()) {
                    ecs::process_window_event(ev);
                }
            }
        }

        // PROCESS UI EVENTS
        {
            ui::Event ev;
            while (ui::get_next_event(ev)) {
                switch (ev.type) {
                case ui::EventType::PlayGame: {
					background::set_type(background::Type::None);
					map::open("summer_forest_00");
                } break;
                case ui::EventType::RestartMap: {
                    map::reset();
                } break;
                case ui::EventType::GoToMainMenu: {
                    background::set_type(background::Type::MountainDusk);
					map::close(0.f);
                } break;
                case ui::EventType::QuitApp: {
                    window::set_should_close(true);
                } break;
				}
			}
        }

        // UPDATE

        audio::update();
        console::update(main_delta_time);
        background::update(main_delta_time);
        ui::update(main_delta_time);

        float game_delta_time = main_delta_time;
        if (steam::is_overlay_active()) {
            game_delta_time = 0.f;
        }
        if (ui::is_menu_or_textbox_visible()) {
            game_delta_time = 0.f;
        }
        map::update(game_delta_time);

        float game_world_delta_time = game_delta_time;
        if (map::get_transition_progress() != 0.f) {
			game_world_delta_time = 0.f; // pause game while map is transitioning
        }
        ecs::update(game_world_delta_time);
        postprocessing::update(game_world_delta_time);

        // RENDER

        int window_framebuffer_width = 0;
        int window_framebuffer_height = 0;
        window::get_framebuffer_size(window_framebuffer_width, window_framebuffer_height);
        if (window_framebuffer_width <= 0 || window_framebuffer_height <= 0) {
            // HACK: So we don't get spammed with errors when the window is minimized.
            window_framebuffer_width = 1;
            window_framebuffer_height = 1;
		}

        const float pixel_scale = (float)window_framebuffer_width / WINDOW_MIN_WIDTH;
		
        sprites::clear_drawing_statistics();

        Vector2f camera_min;
        Vector2f camera_max;
        ecs::get_camera_bounds(camera_min, camera_max);


        {
            const Vector2f camera_center = (camera_min + camera_max) / 2.f;
            const Vector2f camera_size = camera_max - camera_min;

            // Rotation components
            const float angle = 0.f;
            const float cosine = std::cos(angle);
            const float sine = std::sin(angle);
            const float tx = -camera_center.x * cosine - camera_center.y * sine + camera_center.x;
            const float ty = camera_center.x * sine - camera_center.y * cosine + camera_center.y;

            // Projection components
            const float a = 2.f / camera_size.x;
            const float b = -2.f / camera_size.y;
            const float c = -a * camera_center.x;
            const float d = -b * camera_center.y;

            // Create the projection matrix
            const float projection_matrix[16] = {
				a * cosine, -b * sine, 0.f, 0.f,
				a * sine, b * cosine, 0.f, 0.f,
				0.f, 0.f, 1.f, 0.f,
				a * tx + c, b * ty + d, 0.f, 1.f
			};

            graphics::FrameUniforms frame_uniforms{};
            memcpy(frame_uniforms.view_proj_matrix, projection_matrix, sizeof(projection_matrix));
            graphics::update_buffer(graphics::frame_uniform_buffer, &frame_uniforms, sizeof(frame_uniforms));
        }

        Handle<graphics::Framebuffer> framebuffer = graphics::get_temporary_framebuffer(
            window_framebuffer_width, window_framebuffer_height);
        graphics::bind_framebuffer(framebuffer);
        graphics::clear_framebuffer(0.f, 0.f, 0.f, 1.f);
        graphics::set_viewport(0, 0, window_framebuffer_width, window_framebuffer_height);
        background::render_sprites(camera_min, camera_max);
        ecs::draw_sprites(camera_min, camera_max);

        switch (ui::get_top_menu()) {
        case ui::MenuType::Pause:
        case ui::MenuType::Settings:
        case ui::MenuType::Credits: {
            postprocessing::set_gaussian_blur_iterations(3);
        } break;
        default: {
            postprocessing::set_gaussian_blur_iterations(0);
        } break;
        }
        postprocessing::set_darkness_intensity(map::is_dark() ? 0.95f : 0.f);
        postprocessing::set_screen_transition_progress(map::get_transition_progress());
        postprocessing::set_pixel_scale(pixel_scale);
        postprocessing::render(framebuffer, camera_min, camera_max);

#ifdef _DEBUG
        ecs::add_debug_shapes_to_render_queue();
        shapes::render("Game World Debug", camera_min, camera_max);
        shapes::update_lifetimes(game_world_delta_time);
#endif
        ui::render();

#ifdef DEBUG_IMGUI
        // CREATE DEBUG STATS WINDOW

        if (debug_stats) {
            constexpr float smoothing_factor = 0.99f;
            static float smoothed_dt = 0.f;
            static float smoothed_fps = 0.f;
            static float dt_buffer[256] = { 0.f };
            static float fps_buffer[256] = { 0.f };
            static int buffer_offset = 0;
            dt_buffer[buffer_offset] = main_delta_time;
            fps_buffer[buffer_offset] = 1.f / main_delta_time;
            buffer_offset = (buffer_offset + 1) % 256;
            smoothed_dt = smoothing_factor * smoothed_dt + (1.f - smoothing_factor) * main_delta_time;
            smoothed_fps = smoothing_factor * smoothed_fps + (1.f - smoothing_factor) / main_delta_time;
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);
            char overlay_text[64];
            sprintf(overlay_text, "%.f us", smoothed_dt * 1'000'000.f);
            ImGui::PlotLines("##dt", dt_buffer, 256, buffer_offset, overlay_text, 0.f, 0.01f, ImVec2(0, 80));
            sprintf(overlay_text, "%.f FPS", smoothed_fps);
            ImGui::PlotLines("##fps", fps_buffer, 256, buffer_offset, overlay_text, 0.f, 600.f, ImVec2(0, 80));
            ImGui::Value("Sprites Drawn", sprites::get_sprites_drawn());
            ImGui::Value("Batches Drawn", sprites::get_batches_drawn());
            ImGui::Value("Largest Batch", sprites::get_largest_batch_sprite_count());
            ImGui::End();
        }

        if (debug_textboxes) {
			ui::show_textbox_debug_window();
		}
        if (debug_textures) {
            graphics::show_texture_debug_window();
        }

#endif // DEBUG_IMGUI

#ifdef DEBUG_IMGUI
        {
            graphics::ScopedDebugGroup debug_group("ImGui");
            imgui_backends::render();
        }
#endif
        {
            graphics::ScopedDebugGroup debug_group("Window");
            graphics::bind_window_framebuffer();
            graphics::bind_shader(graphics::fullscreen_shader);
            graphics::bind_texture(0, graphics::get_framebuffer_texture(framebuffer));
            graphics::draw(graphics::Primitives::TriangleList, 3); // draw a fullscreen-covering triangle
            graphics::release_temporary_framebuffer(framebuffer);
        }

        window::swap_buffers();

#ifdef DEBUG_RENDERDOC
        renderdoc::open_capture_folder_if_capturing();
#endif
    }

    // SHUTDOWN

    ecs::shutdown();
    ui::shutdown();
    audio::shutdown();
    graphics::shutdown();
#ifdef DEBUG_IMGUI
    imgui_backends::shutdown();
#endif
    window::shutdown();
    steam::shutdown();

    return EXIT_SUCCESS;
}