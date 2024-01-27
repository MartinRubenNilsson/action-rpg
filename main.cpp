#include "stdafx.h"
#include <imgui-SFML.h>
#include <imgui.h>
#include "window.h"
#include "audio.h"
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


    std::cout << "Press F1 to toggle console." << std::endl;

    // LOAD ASSETS

    shaders::load_shaders("assets/shaders");
    audio::load_bank_files("assets/audio/banks");
    tiled::load_assets("assets/tiled");
    ui::load_ttf_fonts("assets/fonts");
    ui::load_rml_documents("assets/ui");
    background::load_assets();

    // GAME LOOP

#if defined(_DEBUG) //&& false
    console::write_help_file("assets/scripts/help.txt");
    console::execute(argc, argv);
#else
    background::type = background::Type::MountainDusk;
    ui::set_main_menu_visible(true);
#endif
    sf::Clock clock;
    while (window.isOpen()) {

        // MESSAGE/EVENT LOOP

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::F1)
                    console::toggle_visible();
                else if (event.key.code == sf::Keyboard::F2)
                    ecs::debug_draw_physics = !ecs::debug_draw_physics;
                else if (event.key.code == sf::Keyboard::F3)
                    ecs::debug_draw_pivots = !ecs::debug_draw_pivots;
            }
            ImGui::SFML::ProcessEvent(window, event);
            if (event.type == sf::Event::KeyPressed && ImGui::GetIO().WantCaptureKeyboard)
                continue;
            console::process_event(event);
            ui::process_event(event);
            if (!ui::should_pause_game())
                ecs::process_event(event);
        }

        // HANDLE UI USER REQUESTS

        switch (ui::get_next_action()) {
        case ui::Action::Play:
            background::type = background::Type::None;
            map::open("summer_forest");
			break;
        case ui::Action::GoToMainMenu:
			background::type = background::Type::MountainDusk;
			map::close();
            break;
        case ui::Action::Quit:
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

        window.clear();
        background::render(window);
        ecs::render(window);
        ui::render(); // Uses OpenGL, so make sure to call resetGLStates() after.
        window.resetGLStates();
        ImGui::SFML::Render(window);
        window.display();
    }

    // UNLOAD ASSETS

    tiled::unload_assets(); 
    background::unload_assets();
    shaders::unload_shaders();

    // SHUTDOWN

    ecs::shutdown();
    ui::shutdown();
    audio::shutdown();
    ImGui::SFML::Shutdown();

    return 0;
}