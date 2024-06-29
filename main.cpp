#include "stdafx.h"
#include "steam.h"
#include "window.h"
#include "window_events.h"
#include "audio.h"
#include "ui.h"
#include "ui_menus.h"
#include "map.h"
#include "ecs.h"
#include "console.h"
#include "tiled.h"
#include "background.h"
#include "postprocessing.h"
#include "settings.h"
#include "cursor.h"
#include "graphics.h"
#include "shapes.h"
#include "sprites.h"
#include "clock.h"
#include "imgui_backends.h"

int main(int argc, char* argv[]) {
    if (steam::restart_app_if_necessary()) {
        return EXIT_FAILURE;
    }
    steam::initialize(); // Fails silently if Steam is not running.
    window::initialize();
#ifdef BUILD_IMGUI
    imgui_backends::initialize(window::get_glfw_window());
#endif
    graphics::initialize();
    console::initialize();
    audio::initialize();
    ui::initialize();
    ecs::initialize();

#if 0
    {
        // Settings::set() affects window and audio, so these must be initialized first.
        Settings settings{};
        if (settings.load()) settings.set();
        else window::set_state(window::State());
    }
#endif

    audio::load_bank_files("assets/audio/banks");
    tiled::load_assets("assets/tiled");
    ui::load_ttf_fonts("assets/fonts");
    ui::load_rml_documents("assets/ui");
    ui::add_event_listeners(); // Must come after loading RML documents.

    // PREPARE FOR GAME LOOP

#ifdef _DEBUG
    console::execute(argc, argv);
#else
    background::type = background::Type::MountainDusk;
    ui::push_menu(ui::MenuType::Main);
#endif

    Clock clock;
    bool debug_stats = false;

    // GAME LOOP

    while (!window::should_close()) {

        steam::run_message_loop();
        window::poll_events();

#ifdef BUILD_IMGUI
        imgui_backends::new_frame();
#endif

        // PROCESS WINDOW EVENTS
        {
            window::Event ev;
            while (window::get_next_event(ev)) {
                if (ev.type == window::EventType::WindowClose) {
                    window::set_should_close(true);
                //} if (ev.type == sf::Event::Resized) {
                //    //textures::clear_render_texture_pool();
                }  else if (ev.type == window::EventType::KeyPress) {
#ifdef _DEBUG
                    if (ev.key.code == window::Key::Backslash) {
                        console::toggle_visible();
                    } else if (ev.key.code == window::Key::F1) {
                        debug_stats = !debug_stats;
                    } else if (ev.key.code == window::Key::F2) {
                        ecs::debug_flags ^= ecs::DEBUG_ENTITIES;
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
                    }
#endif // _DEBUG
                }

#ifdef BUILD_IMGUI
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

        const float main_dt = (float)clock.restart();
        console::update(main_dt);
        background::update(main_dt);
        ui::update(main_dt);

        float game_dt = main_dt;
        if (steam::is_overlay_active()) {
            game_dt = 0.f;
        }
        if (ui::is_menu_or_textbox_visible()) {
            game_dt = 0.f;
        }
        map::update(game_dt);

        float game_world_dt = game_dt;
        if (map::get_transition_progress() != 0.f) {
			game_world_dt = 0.f; // pause game while map is transitioning
        }
        ecs::update(game_world_dt);
        shapes::update_lifetimes(game_world_dt); // why is this here??

        postprocessing::update(game_world_dt);
		postprocessing::set_darkness_intensity(map::is_dark() ? 0.95f : 0.f);
        postprocessing::set_screen_transition_progress(map::get_transition_progress());
        //postprocessing::set_pixel_scale((float)window::get_state().scale);

        switch (ui::get_top_menu()) {
        case ui::MenuType::Pause:
        case ui::MenuType::Settings:
        case ui::MenuType::Credits:
            postprocessing::set_gaussian_blur_iterations(3);
            break;
        default:
            postprocessing::set_gaussian_blur_iterations(0);
			break;
        }

        sprites::reset_rendering_statistics();

        // RENDER

        int render_width = 0;
        int render_height = 0;
        window::get_framebuffer_size(render_width, render_height);

        int render_target_id = graphics::acquire_pooled_render_target(render_width, render_height);
        graphics::bind_render_target(render_target_id);
        graphics::clear_render_target(0.f, 0.f, 0.f, 1.f);
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

            graphics::set_modelview_matrix_to_identity();
            graphics::set_projection_matrix(projection_matrix);
            graphics::set_texture_matrix_to_identity();
        }

        graphics::set_viewport(0, 0, render_width, render_height);

        background::render_sprites(camera_min, camera_max);
        ecs::render_sprites(camera_min, camera_max);
        ecs::add_debug_shapes_to_render_queue();
        postprocessing::render(render_target_id, camera_min, camera_max);
        shapes::render(camera_min, camera_max);
        
        //ui::render(); //BROKEN rn

        bool show_built_in_cursor = false;
#ifdef BUILD_IMGUI
        ImGui::GetIO().WantCaptureMouse;
#endif
        //window.setMouseCursorVisible(show_built_in_cursor);
        cursor::set_visible(!show_built_in_cursor);
        //cursor::set_position(Vector2f(sf::Mouse::getPosition(window)));
        //cursor::set_scale((float)window::get_state().scale);
        cursor::render_sprite();

        // RENDER TO WINDOW

        graphics::bind_render_target(graphics::window_render_target_id);
        graphics::clear_render_target(0.f, 0.f, 0.f, 1.f);
        graphics::bind_shader(graphics::fullscreen_shader_id);
        graphics::set_shader_uniform_1i(graphics::fullscreen_shader_id, "tex", 0);
        graphics::bind_texture(0, graphics::get_render_target_texture(render_target_id));
        graphics::draw_triangle_strip(4);
        graphics::release_pooled_render_target(render_target_id);

        // RENDER DEBUG STATS

        if (debug_stats) {
            constexpr float smoothing_factor = 0.99f;
            static float smoothed_dt = 0.f;
            static float smoothed_fps = 0.f;
            static float dt_buffer[256] = { 0.f };
            static float fps_buffer[256] = { 0.f };
            static int buffer_offset = 0;
            dt_buffer[buffer_offset] = main_dt;
            fps_buffer[buffer_offset] = 1.f / main_dt;
            buffer_offset = (buffer_offset + 1) % 256;
            smoothed_dt = smoothing_factor * smoothed_dt + (1.f - smoothing_factor) * main_dt;
            smoothed_fps = smoothing_factor * smoothed_fps + (1.f - smoothing_factor) / main_dt;
#ifdef BUILD_IMGUI
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);
            char overlay_text[64];
            sprintf(overlay_text, "%.f us", smoothed_dt * 1'000'000.f);
            ImGui::PlotLines("##dt", dt_buffer, 256, buffer_offset, overlay_text, 0.f, 0.01f, ImVec2(0, 80));
            sprintf(overlay_text, "%.f FPS", smoothed_fps);
            ImGui::PlotLines("##fps", fps_buffer, 256, buffer_offset, overlay_text, 0.f, 400.f, ImVec2(0, 80));
            ImGui::Value("Sprites Drawn", sprites::get_sprites_drawn());
            ImGui::Value("Batches Drawn", sprites::get_batches_drawn());
            ImGui::Value("Largest Batch", sprites::get_sprites_in_largest_batch());
            ImGui::Checkbox("Enable Batching", &sprites::enable_batching);
            ImGui::End();
#endif // BUILD_IMGUI
        }

#ifdef BUILD_IMGUI
        imgui_backends::render();
#endif

        window::swap_buffers();
    }

    // SHUTDOWN

    ecs::shutdown();
    ui::shutdown();
    audio::shutdown();
    graphics::shutdown();
#ifdef BUILD_IMGUI
    imgui_backends::shutdown();
#endif
    window::shutdown();
    steam::shutdown();

    return EXIT_SUCCESS;
}