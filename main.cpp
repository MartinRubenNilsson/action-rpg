#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include "map.h"
#include "rml.h"
#include "game.h"
#include "rml_data_bindings.h"
#include <imgui-SFML.h>
#include <imgui.h>

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

    // LOAD ASSETS

    rml::load_assets();
    map::load_assets();

    entt::registry registry;
    map::create_entities(registry, "dungeon.tmx");

    rml::show_document("main_menu.rml");

    // GAME LOOP

    sf::Clock delta_clock;
    while (window.isOpen())
    {
        // EVENT HANDLING
        {
            sf::Event event;
            while (window.pollEvent(event))
            {
                ImGui::SFML::ProcessEvent(window, event);
                rml::process_event(event);

                if (event.type == sf::Event::Closed)
                    window.close();
            }
        }

        // UPDATING
        {
            sf::Time sf_delta_time = delta_clock.restart();
            float delta_time = sf_delta_time.asSeconds();
            ImGui::SFML::Update(window, sf_delta_time);
            rml::update();
            game::update_player(registry, delta_time);
            game::update_tiles(registry, delta_time);
        }

        // RENDERING

        window.clear();
        for (auto [entity, sprite] : registry.view<sf::Sprite>().each())
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