#include "stdafx.h"
#include <imgui-SFML.h>
#include <imgui.h>
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
#include "postprocess.h"
#include "settings.h"
#include "textures.h"

#pragma comment(lib, "winmm") // SFML requires this
#ifdef _DEBUG
#pragma comment(lib, "sfml-main-d")
#else
#pragma comment(lib, "sfml-main")
#endif

int main(int argc, char* argv[])
{
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
    sf::RenderTexture render_texture;
    render_texture.create(window.getSize().x, window.getSize().y);
    bool debug_stats = false;

    // GAME LOOP

    while (window.isOpen()) {

        // PROCESS WINDOW EVENTS
        {
            sf::Event ev;
            while (window::poll_event(ev)) {
                if (ev.type == sf::Event::Closed)
                    window.close();
                if (ev.type == sf::Event::Resized)
					render_texture.create(ev.size.width, ev.size.height);
                else if (ev.type == sf::Event::KeyPressed) {
#ifdef _DEBUG
                    if (ev.key.code == sf::Keyboard::Backslash)
                        console::toggle_visible();
                    else if (ev.key.code == sf::Keyboard::F1)
                        debug_stats = !debug_stats;
                    else if (ev.key.code == sf::Keyboard::F2)
                        ecs::debug_flags ^= ecs::DEBUG_PHYSICS;
                    else if (ev.key.code == sf::Keyboard::F3)
                        ecs::debug_flags ^= ecs::DEBUG_PIVOTS;
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
        if (!ui::should_pause_game())
            ecs::update(dt.asSeconds());

        if (debug_stats) {
            static float smoothed_dt = 0.f;
            static float smoothed_fps = 0.f;
            smoothed_dt = 0.9f * smoothed_dt + 0.1f * dt.asSeconds();
            smoothed_fps = 0.9f * smoothed_fps + 0.1f / dt.asSeconds();
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::Text("dt: %.3f ms", smoothed_dt * 1000.f);
            ImGui::Text("FPS: %.1f", smoothed_fps);
            ImGui::End();
        }

        // RENDER TO TEXTURE

        render_texture.clear();
        background::render(render_texture);
        ecs::render(render_texture);
        render_texture.display();

        // RENDER TO WINDOW

        window.clear();

#if 0
        // SHOCKWAVE POSTPROCESSING EFFECT
        {
            static sf::Vector2f center(0.5f, 0.5f);
            static float force = 0.0f;
            static float size = 0.15f;
            static float thickness = 0.05f;
            ImGui::Begin("Shockwave");
            ImGui::SliderFloat("Center X", &center.x, 0.0f, (float)window.getSize().x);
            ImGui::SliderFloat("Center Y", &center.y, 0.0f, (float)window.getSize().y);
            ImGui::SliderFloat("Force", &force, 0.0f, 1.0f);
            ImGui::SliderFloat("Size", &size, 0.0f, 1.0f);
            ImGui::SliderFloat("Thickness", &thickness, 0.0f, 1.0f);
            ImGui::End();
            postprocess::shockwave(window, render_texture.getTexture(), center, force, size, thickness);
        }
#else
        postprocess::copy(window, render_texture.getTexture());
#endif

        ui::render(window);
        ImGui::SFML::Render(window);
        window.display();
    }

    // SHUTDOWN

    ecs::shutdown();
    ui::shutdown();
    audio::shutdown();
    ImGui::SFML::Shutdown();

    // UNLOAD ASSETS

    tiled::unload_assets(); 
    background::unload_assets();
    shaders::unload_assets();
    fonts::unload_assets();
    textures::unload_assets();

    return 0;
}