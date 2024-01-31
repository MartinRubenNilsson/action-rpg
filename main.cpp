#include "stdafx.h"
#include <imgui-SFML.h>
#include <imgui.h>
#include "window.h"
#include "audio.h"
#include "fonts.h"
#include "shaders.h"
#include "ui.h"
#include "ui_main_menu.h"
#include "map.h"
#include "ecs.h"
#include "console.h"
#include "tiled.h"
#include "background.h"

#pragma comment(lib, "winmm") // SFML requires this
#ifdef _DEBUG
#pragma comment(lib, "sfml-main-d")
#else
#pragma comment(lib, "sfml-main")
#endif

int main(int argc, char* argv[])
{
    sf::RenderWindow window;

    // INITIALIZE

    window::initialize(window);
    ImGui::SFML::Init(window, false);
    ImGui::GetIO().Fonts->AddFontFromFileTTF("assets/fonts/Consolas.ttf", 24);
    ImGui::SFML::UpdateFontTexture();
    audio::initialize();
    ecs::initialize();
    console::initialize();
    ui::initialize(window);

    sf::RenderTexture render_texture;
    render_texture.create(window.getSize().x, window.getSize().y);

    // LOAD ASSETS

    fonts::load_assets("assets/fonts");
    shaders::load_assets("assets/shaders");
    audio::load_bank_files("assets/audio/banks");
    tiled::load_assets("assets/tiled");
    ui::load_ttf_fonts("assets/fonts");
    ui::load_rml_documents("assets/ui");
    background::load_assets();

    // GAME LOOP

#if defined(_DEBUG) //&& false
    console::execute(argc, argv);
#else
    background::type = background::Type::MountainDusk;
    ui::set_main_menu_visible(true);
#endif
    sf::Clock clock;
    while (window.isOpen()) {

        // MESSAGE/EVENT LOOP

        sf::Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed)
                window.close();
            else if (ev.type == sf::Event::Resized)
                render_texture.create(ev.size.width, ev.size.height);
            else if (ev.type == sf::Event::KeyPressed) {
                if (ev.key.code == sf::Keyboard::F1)
                    console::toggle_visible();
                else if (ev.key.code == sf::Keyboard::F2)
                    ecs::debug_flags ^= ecs::DEBUG_PHYSICS;
                else if (ev.key.code == sf::Keyboard::F3)
                    ecs::debug_flags ^= ecs::DEBUG_PIVOTS;
                else if (ev.key.code == sf::Keyboard::F4)
                    ecs::debug_flags ^= ecs::DEBUG_AI;
            }
            ImGui::SFML::ProcessEvent(window, ev);
            if (ev.type == sf::Event::KeyPressed && ImGui::GetIO().WantCaptureKeyboard)
                continue;
            console::process_event(ev);
            ui::process_event(ev);
            if (!ui::should_pause_game())
                ecs::process_event(ev);
        }

        // HANDLE UI REQUESTS

        switch (ui::get_next_request()) {
        case ui::Request::Play:
            background::type = background::Type::None;
            map::open("summer_forest");
			break;
        case ui::Request::GoToMainMenu:
			background::type = background::Type::MountainDusk;
			map::close();
            break;
        case ui::Request::Quit:
			window.close();
			break;
        }

        // UPDATE

        sf::Time dt = clock.restart();
        ImGui::SFML::Update(window, dt);
        console::update(dt.asSeconds()); // Must come after ImGui::SFML::Update but before Imgui::SFML::Render.
        background::update(dt.asSeconds());
        map::update();
        audio::update();
        ui::update(dt.asSeconds());
        if (!ui::should_pause_game())
            ecs::update(dt.asSeconds());

        // RENDER

        render_texture.clear();
        background::render(render_texture);
        ecs::render(render_texture);
        ui::render(); // Uses OpenGL, so make sure to call resetGLStates() after.
        render_texture.resetGLStates();
        render_texture.display();
        window.clear();
        {
            sf::Sprite sprite(render_texture.getTexture());
            window.setView(window.getDefaultView());
            window.draw(sprite);
        }
        ImGui::SFML::Render(window);
        window.display();
    }

    // UNLOAD ASSETS

    tiled::unload_assets(); 
    background::unload_assets();
    shaders::unload_assets();
    fonts::unload_assets();

    // SHUTDOWN

    ecs::shutdown();
    ui::shutdown();
    audio::shutdown();
    ImGui::SFML::Shutdown();

    return 0;
}