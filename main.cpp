#include <Windows.h>
#include "map.h"
#include "rml.h"
#include "game.h"
#include "rml_data_bindings.h"

#define MAP_WIDTH 480
#define MAP_HEIGHT 320

int main()
{
    // CREATE WINDOW

    sf::VideoMode video_mode(MAP_WIDTH * 3, MAP_HEIGHT * 3);
    sf::Uint32 style = sf::Style::Titlebar | sf::Style::Close;
    sf::RenderWindow window(video_mode, "Hello, SFML!", style);

    sf::View view(sf::FloatRect(0, 0, MAP_WIDTH, MAP_HEIGHT));
    window.setView(view);

    // STARTUP

    rml::startup(&window);

    // LOAD ASSETS

    rml::load_assets();
    map::load_assets();

    entt::registry registry;
    map::create_entities(registry, "dungeon.tmx");
    rml::show_document("main_menu.rml");

    rml::test_string = "Hello, RML!";
    rml::dirty_variable("test_string");

    // GAME LOOP

    sf::Clock clock;
    while (window.isOpen())
    {
        // EVENT HANDLING

        sf::Event ev;
        while (window.pollEvent(ev))
        {
            if (ev.type == sf::Event::Closed)
            {
                window.close();
                break;
            }

            rml::process_event(ev);
        }

        // UPDATING

        float dt = clock.restart().asSeconds();
		game::update_player(registry, dt);
        game::update_tiles(registry, dt);
        rml::update();

        // RENDERING

        window.clear();
        for (auto [entity, sprite] : registry.view<sf::Sprite>().each())
            window.draw(sprite);
        window.pushGLStates();
        rml::render(); // Uses OpenGL, so we need to push and pop the OpenGL states before and after.
        window.popGLStates();
        window.display();
    }

    // CLEANUP

    rml::cleanup();

	return 0;
}