#include "stdafx.h"
#include <imgui-SFML.h>
#include <imgui.h>
#include "window.h"
#include "audio.h"
#include "physics.h"
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
    physics::initialize();
    ecs::initialize();
    console::initialize();
    ui::initialize(window);

    // LOAD ASSETS

    audio::load_assets("assets/audio/banks");
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
    bool debug_draw_physics = false;
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
                    debug_draw_physics = !debug_draw_physics;
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

        switch (ui::get_user_request()) {
        case ui::UserRequest::Play:
            background::type = background::Type::None;
            map::open("forest_summer");
			break;
        case ui::UserRequest::GoToMainMenu:
			background::type = background::Type::MountainDusk;
			map::close();
            break;
        case ui::UserRequest::Quit:
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
        if (!ui::should_pause_game()) {
            physics::update(dt.asSeconds());
            ecs::update(dt.asSeconds());
        }

        // RENDER

        window.clear();
        background::render(window);
        ecs::render(window);
        if (debug_draw_physics)
            physics::debug_draw(window);
        ui::render(); // Uses OpenGL, so make sure to call resetGLStates() after.
        window.resetGLStates();
        ImGui::SFML::Render(window);
        window.display();
    }

    // UNLOAD ASSETS

    // Ensures sf::Texture objects are destroyed before main() returns.
    tiled::unload_assets(); 
    background::unload_assets();

    // SHUTDOWN

    ecs::shutdown();
    physics::shutdown();
    ui::shutdown();
    audio::shutdown();
    ImGui::SFML::Shutdown();

    return 0;
}