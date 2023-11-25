#include "stdafx.h"
#include <imgui-SFML.h>
#include <imgui.h>
#include "window.h"
#include "audio.h"
#include "physics.h"
#include "ui.h"
#include "map.h"
#include "behavior.h"
#include "ecs.h"
#include "console.h"
#include "tables.h"
#include "tiled.h"

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

    audio::load_banks();
    tiled::load_assets();
    ui::load_fonts_and_documents();
    tables::load_tables();

    // OTHER STUFF

#ifdef _DEBUG
    console::write_help_file("assets/scripts/help.txt");
#endif

    // EXECUTE STARTUP COMMANDS

    console::execute(argc, argv);

    // GAME LOOP

    bool pause_game = false;
    bool debug_draw_physics = false;

    sf::Clock clock;
    while (window.isOpen())
    {
        // EVENT HANDLING

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Escape)
                    pause_game = !pause_game;
                if (event.key.code == sf::Keyboard::F1)
                    console::toggle_show();
                if (event.key.code == sf::Keyboard::F2)
                    debug_draw_physics = !debug_draw_physics;
            }

            ImGui::SFML::ProcessEvent(window, event);
            if (event.type == sf::Event::KeyPressed && ImGui::GetIO().WantCaptureKeyboard)
				continue;

            console::process_event(event);
            ui::process_event(event);

            if (!pause_game && !ui::should_pause_game())
                ecs::process_event(event);
        }

        // UPDATE

        sf::Time dt = clock.restart();
        ImGui::SFML::Update(window, dt);
        console::update(dt.asSeconds()); // Must come after ImGui::SFML::Update but before Imgui::SFML::Render.
        map::update();
        audio::update();
        ui::update(dt.asSeconds());
        if (!pause_game && !ui::should_pause_game())
        {
            physics::update(dt.asSeconds());
            ecs::update(dt.asSeconds());
        }

        // RENDER

        window.clear();
        ecs::render(window);
        if (debug_draw_physics)
            physics::debug_draw(window);
        ui::render(); // Uses OpenGL, so make sure to call resetGLStates() after.
        window.resetGLStates();
        ImGui::SFML::Render(window);
        window.display();
    }

    // UNLOAD ASSETS

    tiled::unload_assets(); // Ensures sf::Texture objects are destroyed before main() returns.

    // SHUTDOWN

    ecs::shutdown();
    physics::shutdown();
    ui::shutdown();
    audio::shutdown();
    ImGui::SFML::Shutdown();

	return 0;
}