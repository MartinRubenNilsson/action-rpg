#include <Windows.h>
#include "map.h"
#include "rml.h"
#include "game.h"

#define MAP_WIDTH 480
#define MAP_HEIGHT 320

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    // CREATE WINDOW

    sf::VideoMode video_mode(MAP_WIDTH * 3, MAP_HEIGHT * 3);
    sf::Uint32 style = sf::Style::Titlebar | sf::Style::Close;
    sf::RenderWindow window(video_mode, "Hello, SFML!", style);

    sf::View view(sf::FloatRect(0, 0, MAP_WIDTH, MAP_HEIGHT));
    window.setView(view);

    rml::startup(&window);
    //rml::load_document("assets/rml/main_menu.rml");

    map::load_assets();

    entt::registry registry;
    map::load(registry, "dungeon");

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
        rml::update();

        // RENDERING

        window.clear();

        for (auto [entity, sprite] : registry.view<sf::Sprite>().each())
            window.draw(sprite);

        // RmlUi uses OpenGL, so we need to push and pop the OpenGL states.
        window.pushGLStates();
        rml::render();
        window.popGLStates();

        window.display();
    }

    rml::cleanup();

	return 0;
}