#include "map.h"
#include "rml.h"
#include <Windows.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	sf::RenderWindow window(sf::VideoMode(800, 600), "Hello, SFML!");
    window.setVerticalSyncEnabled(true);

    rml::startup(&window);
    rml::load_document("assets/rml/hello_world.rml");

    map::load_map("assets/maps/test00.tmx");

    while (window.isOpen())
    {
        // EVENT HANDLING

        sf::Event ev;
        while (window.pollEvent(ev))
        {
            if (ev.type == sf::Event::Closed)
            {
                window.close();
                goto SHUTDOWN;
            }

            rml::process_event(ev);
        }

        // UPDATING

        rml::update();

        // RENDERING

        window.clear();
        map::draw_map(window);

        // RmlUi uses OpenGL, so we need to push and pop the OpenGL states.
        window.pushGLStates();
        rml::render();
        window.popGLStates();

        window.display();
    }

    SHUTDOWN:
    rml::cleanup();

	return 0;
}