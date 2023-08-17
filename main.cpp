#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <imgui-SFML.h>
#include <imgui.h>
#include "map.h"
#include "rml.h"
#include "game.h"
#include "rml_data_bindings.h"
#include "console.h"

#define MAP_WIDTH 480
#define MAP_HEIGHT 320

int WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR pCmdLine, int)
{
    // CREATE WINDOW

    sf::VideoMode video_mode(MAP_WIDTH * 3, MAP_HEIGHT * 3);
    sf::Uint32 style = sf::Style::Titlebar | sf::Style::Close;
    sf::RenderWindow window(video_mode, "Hello, SFML!", style);

    sf::View view(sf::FloatRect(0, 0, MAP_WIDTH, MAP_HEIGHT));
    window.setView(view);

    // STARTUP

    ImGui::SFML::Init(window);
    rml::startup(&window);
    console::startup();

    // LOAD ASSETS

    rml::load_assets();
    map::load_assets();

    rml::show_document("main_menu.rml");

    // GAME LOOP

    sf::Clock clock;
    while (window.isOpen())
    {
        // EVENT HANDLING

        sf::Event event;
        while (window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(window, event);
            rml::process_event(event);
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F1)
				console::toggle_visible();
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // UPDATING

        sf::Time delta_time = clock.restart();
        float dt = delta_time.asSeconds();
        ImGui::SFML::Update(window, delta_time);
        console::update(); // Must come after ImGui::SFML::Update but before Imgui::SFML::Render.
        rml::update();
        game::update_player(map::get_registry(), dt);
        game::update_tiles(map::get_registry(), dt);

        // RENDERING

        window.clear();
        for (auto [entity, sprite] : map::get_registry().view<sf::Sprite>().each())
            window.draw(sprite);
        window.pushGLStates();
        rml::render(); // Uses OpenGL, so we need to push and pop the OpenGL states before and after.
        window.popGLStates();
        ImGui::SFML::Render(window);
        window.display();
    }

    // CLEANUP

    rml::cleanup();
    ImGui::SFML::Shutdown();

	return 0;
}