#include "stdafx.h"
#include <imgui-SFML.h>
#include "steam.h"
#include "debug_draw.h"
#include "window.h"
#include "audio.h"
#include "fonts.h"
#include "shaders.h"
#include "ui.h"
#include "ui_menus.h"
#include "map.h"
#include "ecs.h"
#include "console.h"
#include "tiled.h"
#include "background.h"
#include "postprocessing.h"
#include "settings.h"
#include "textures.h"
#include "cursor.h"
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

    fonts::load_assets("assets/fonts");
    shaders::load_assets("assets/shaders");
    audio::load_bank_files("assets/audio/banks");
    tiled::load_assets("assets/tiled");
    ui::load_ttf_fonts("assets/fonts");
    ui::load_rml_documents("assets/ui");
    background::load_assets();

    // INITIALIZATION PASS 2
    {
        // Settings::set() affects window and audio, so these must be initialized first.
        Settings settings{};
        if (settings.load()) settings.set();
        else window::create_or_update();
    }
    ui::add_event_listeners(); // Must come after loading RML documents.
    ImGui::SFML::Init(window, false); // Window must be created first.
    ImGui::GetIO().Fonts->AddFontFromFileTTF("assets/fonts/Consolas.ttf", 24);
    ImGui::SFML::UpdateFontTexture();
    console::initialize(); // Must come after ImGui::SFML::Init.

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

    while (window.isOpen()) {

        steam::run_message_loop();

        // PROCESS WINDOW EVENTS
        {
            sf::Event ev;
            while (window::poll_event(ev)) {
                if (ev.type == sf::Event::Closed) {
                    window.close();
                } if (ev.type == sf::Event::Resized) {
                    textures::clear_render_texture_pool();
                } else if (ev.type == sf::Event::KeyPressed) {
#ifdef _DEBUG
                    if (ev.key.code == sf::Keyboard::Backslash)
                        console::toggle_visible();
                    else if (ev.key.code == sf::Keyboard::F1)
                        debug_stats = !debug_stats;
                    else if (ev.key.code == sf::Keyboard::F2)
                        ecs::debug_flags ^= ecs::DEBUG_PHYSICS;
                    else if (ev.key.code == sf::Keyboard::F3)
                        ecs::debug_flags ^= ecs::DEBUG_TILES;
                    else if (ev.key.code == sf::Keyboard::F4)
                        ecs::debug_flags ^= ecs::DEBUG_AI;
                    else if (ev.key.code == sf::Keyboard::F5)
                        ecs::debug_flags ^= ecs::DEBUG_PLAYER;
                    else if (ev.key.code == sf::Keyboard::F6)
                        ui::debug = !ui::debug;
#endif
                }
                ImGui::SFML::ProcessEvent(window, ev);
                if (ev.type == sf::Event::KeyPressed && ImGui::GetIO().WantCaptureKeyboard)
                    continue;
                console::process_event(ev);
                ui::process_window_event(ev);
                if (!ui::should_pause_game())
                    ecs::process_event(ev);
            }
        }

        // PROCESS UI EVENTS
        {
            ui::Event ev;
            while (ui::poll_event(ev)) {
                switch (ev.type) {
				case ui::Event::PlayGame:
					background::type = background::Type::None;
					map::open("summer_forest");
					break;
				case ui::Event::RestartMap:
					map::reset();
					break;
				case ui::Event::GoToMainMenu:
					background::type = background::Type::MountainDusk;
					map::close();
					break;
				case ui::Event::QuitApp:
					window.close();
					break;
				}
			}
        }

        // UPDATE

        sf::Time dt = clock.restart();
        ImGui::SFML::Update(window, dt);
        console::update(dt.asSeconds()); // Must come after ImGui::SFML::Update.
        background::update(dt.asSeconds());
        map::update();
        audio::update();
        ui::update(dt.asSeconds());
        debug::update(dt.asSeconds());
        if (!ui::should_pause_game() && !steam::is_overlay_active()) {
            ecs::update(dt.asSeconds());
            postprocessing::update(dt.asSeconds());
        }

        if (debug_stats) {
            constexpr float smoothing_factor = 0.99f;
            static float smoothed_dt = 0.f;
            static float smoothed_fps = 0.f;
            static float dt_buffer[256] = { 0.f };
            static float fps_buffer[256] = { 0.f };
            static int buffer_offset = 0;
            float dt_f = dt.asSeconds();
            dt_buffer[buffer_offset] = dt_f;
            fps_buffer[buffer_offset] = 1.f / dt_f;
            buffer_offset = (buffer_offset + 1) % 256;
            smoothed_dt = smoothing_factor * smoothed_dt + (1.f - smoothing_factor) * dt_f;
            smoothed_fps = smoothing_factor * smoothed_fps + (1.f - smoothing_factor) / dt_f;
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

        // RENDER BACKGROUND, ECS, DEBUG DRAW, POSTPROCESS
        {
            std::unique_ptr<sf::RenderTexture> texture =
                textures::take_render_texture_from_pool(window.getSize());
            texture->clear();
            texture->setView(window::get_default_view());
            background::render(*texture);
            ecs::draw(*texture);
            ecs::debug_draw();
            sprites::render(*texture);
            debug::render(*texture);
            texture->display();
            postprocessing::render(texture);
            window.draw(sf::Sprite(texture->getTexture())); // Copy to window.
            textures::give_render_texture_to_pool(std::move(texture));
        }

        // RENDER UI

        ui::render(window);

        // RENDER CURSOR

        bool show_built_in_cursor = ImGui::GetIO().WantCaptureMouse;
        window.setMouseCursorVisible(show_built_in_cursor);
        cursor::set_visible(!show_built_in_cursor);
        cursor::set_position(sf::Vector2f(sf::Mouse::getPosition(window)));
        cursor::set_scale((float)window::get_desc().scale);
        cursor::render(window);

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
    background::unload_assets();
    shaders::unload_assets();
    fonts::unload_assets();
    textures::shutdown();
    steam::shutdown();

    return EXIT_SUCCESS;
}