#include "stdafx.h"
#include <imgui-SFML.h>
#include "steam.h"
#include "window.h"
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

#pragma comment(lib, "winmm") // SFML requires this
#ifdef _DEBUG
#pragma comment(lib, "sfml-main-d")
#else
#pragma comment(lib, "sfml-main")
#endif

int main(int argc, char* argv[])
{
    if (steam::restart_app_if_necessary())
        return EXIT_FAILURE;
    steam::initialize(); // Fails silently if Steam is not running.

    // INITIALIZATION PASS 1

    sf::RenderWindow window;
    window::initialize(window); // Does not call sf::RenderWindow::create().
    audio::initialize();
    ui::initialize();
    ecs::initialize();

    // LOAD ASSETS

    audio::load_bank_files("assets/audio/banks");
    tiled::load_assets("assets/tiled");
    ui::load_ttf_fonts("assets/fonts");
    ui::load_rml_documents("assets/ui");

    // INITIALIZATION PASS 2
    {
        // Settings::set() affects window and audio, so these must be initialized first.
        Settings settings{};
        if (settings.load()) settings.set();
        else window::set_state(window::State());
    }
    ui::add_event_listeners(); // Must come after loading RML documents.
    ImGui::SFML::Init(window, false); // Window must be created first.
    ImGui::GetIO().Fonts->AddFontFromFileTTF("assets/fonts/Consolas.ttf", 18);
    ImGui::SFML::UpdateFontTexture();
    console::initialize(); // Must come after ImGui::SFML::Init.
    graphics::initialize();

    // PREPARE FOR GAME LOOP

#ifdef _DEBUG
    console::execute(argc, argv);
#else
    background::type = background::Type::MountainDusk;
    ui::push_menu(ui::MenuType::Main);
#endif

    sf::Clock clock;
    bool debug_stats = false;

    // GAME LOOP

    window.resetGLStates();
    while (window.isOpen()) {

        steam::run_message_loop();

        // PROCESS WINDOW EVENTS
        {
            sf::Event ev;
            while (window::poll_event(ev)) {
                if (ev.type == sf::Event::Closed) {
                    window.close();
                } if (ev.type == sf::Event::Resized) {
                    //textures::clear_render_texture_pool();
                } else if (ev.type == sf::Event::KeyPressed) {
#ifdef _DEBUG
                    if (ev.key.code == sf::Keyboard::Backslash)
                        console::toggle_visible();
                    else if (ev.key.code == sf::Keyboard::F1)
                        debug_stats = !debug_stats;
                    else if (ev.key.code == sf::Keyboard::F2)
                        ecs::debug_flags ^= ecs::DEBUG_ENTITIES;
                    else if (ev.key.code == sf::Keyboard::F3)
                        ecs::debug_flags ^= ecs::DEBUG_PHYSICS;
                    else if (ev.key.code == sf::Keyboard::F4)
                        ecs::debug_flags ^= ecs::DEBUG_AI;
                    else if (ev.key.code == sf::Keyboard::F5)
                        ecs::debug_flags ^= ecs::DEBUG_PLAYER;
                    else if (ev.key.code == sf::Keyboard::F6)
                        ui::debug = !ui::debug;
                    else if (ev.key.code == sf::Keyboard::F7)
                        map::debug = !map::debug;
#endif
                }
                ImGui::SFML::ProcessEvent(window, ev);
                if (ev.type == sf::Event::KeyPressed && ImGui::GetIO().WantCaptureKeyboard)
                    continue;
                console::process_event(ev);
                ui::process_window_event(ev);
                if (!ui::is_menu_or_textbox_visible())
                    ecs::process_event(ev);
            }
        }

        // PROCESS UI EVENTS
        {
            ui::Event ev;
            while (ui::poll_event(ev)) {
                switch (ev.type) {
				case ui::Event::PlayGame:
					background::set_type(background::Type::None);
					map::open("summer_forest_00");
					break;
				case ui::Event::RestartMap:
					map::reset();
					break;
				case ui::Event::GoToMainMenu:
                    background::set_type(background::Type::MountainDusk);
					map::close(0.f);
					break;
				case ui::Event::QuitApp:
					window.close();
					break;
				}
			}
        }

        // UPDATE

        sf::Time dt = clock.restart();
        ImGui::SFML::Update(window, dt); // Always call this first!

        audio::update();

        float main_dt = dt.asSeconds();
        console::update(main_dt); // Must come after ImGui::SFML::Update.
        background::update(main_dt);
        ui::update(main_dt);

        float game_dt = main_dt;
        if (steam::is_overlay_active())
            game_dt = 0.f;
		if (ui::is_menu_or_textbox_visible())
            game_dt = 0.f;
        map::update(game_dt);

        float game_world_dt = game_dt;
		if (map::get_transition_progress() != 0.f)
			game_world_dt = 0.f;
        ecs::update(game_world_dt);
        shapes::update_lifetimes(game_world_dt);

        // UPDATE POSTPROCESSING

        postprocessing::update(game_world_dt);
		postprocessing::set_darkness_intensity(map::is_dark() ? 0.95f : 0.f);
        postprocessing::set_screen_transition_progress(map::get_transition_progress());
        postprocessing::set_pixel_scale((float)window::get_state().scale);

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

        // RENDER BACKGROUND, ECS, DEBUG DRAW, POSTPROCESS

        const sf::Vector2u window_size = window.getSize();

        int render_target_id = graphics::acquire_pooled_render_target(window_size.x, window_size.y);
        graphics::bind_render_target(render_target_id);
        graphics::clear_render_target(0.f, 0.f, 0.f, 1.f);
        sf::Vector2f camera_min;
        sf::Vector2f camera_max;
        ecs::get_camera_bounds(camera_min, camera_max);
        sf::View view{ (camera_min + camera_max) / 2.f, camera_max - camera_min };
        graphics::set_modelview_matrix_to_identity();
        graphics::set_projection_matrix(view.getTransform().getMatrix());
        graphics::set_texture_matrix_to_identity();
        graphics::set_viewport(0, 0, window_size.x, window_size.y);

        background::render_sprites(camera_min, camera_max);
        ecs::render_sprites(camera_min, camera_max);
        ecs::add_debug_shapes_to_render_queue();
        postprocessing::render(render_target_id, camera_min, camera_max);
        shapes::render(camera_min, camera_max);

        // RENDER UI

        window.resetGLStates();
        ui::render();
        window.resetGLStates();

        // RENDER CURSOR

        bool show_built_in_cursor = ImGui::GetIO().WantCaptureMouse;
        window.setMouseCursorVisible(show_built_in_cursor);
        cursor::set_visible(!show_built_in_cursor);
        cursor::set_position(sf::Vector2f(sf::Mouse::getPosition(window)));
        cursor::set_scale((float)window::get_state().scale);
        cursor::render_sprite();

        // RENDER TO WINDOW

        window.setActive();
        window.clear(sf::Color::Black);
        window.resetGLStates();

        graphics::bind_shader(graphics::fullscreen_shader_id);
        graphics::set_shader_uniform_1i(graphics::fullscreen_shader_id, "tex", 0);
        graphics::bind_texture(0, graphics::get_render_target_texture(render_target_id));
        graphics::draw_triangle_strip(4);
        graphics::release_pooled_render_target(render_target_id);
        window.resetGLStates();

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
        }

        // RENDER IMGUI

        ImGui::SFML::Render(window);

        // DISPLAY

        window.display();
    }

    // SHUTDOWN

    ecs::shutdown();
    ui::shutdown();
    audio::shutdown();
    ImGui::SFML::Shutdown();
    tiled::unload_assets();
    graphics::shutdown();
    steam::shutdown();

    return EXIT_SUCCESS;
}