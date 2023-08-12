#include "map.h"
#include "rml.h"
#include <Windows.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    //if (!map::load_map("assets/maps/test00.tmx"))
	//	return 1;

    // Create the window.
    bool allow_resize = true;
    const sf::Uint32 style = (allow_resize ? sf::Style::Default : (sf::Style::Titlebar | sf::Style::Close));

    sf::ContextSettings context_settings;
    context_settings.stencilBits = 8;
    context_settings.antialiasingLevel = 2;

	sf::RenderWindow window(sf::VideoMode(800, 600), "Hello, SFML!", style, context_settings);
    window.setVerticalSyncEnabled(true);
    window.setActive(true);

    rml::startup_rml(&window);
    rml::load_rml("assets/rml/hello_world.rml");

    while (window.isOpen())
    {
        //sf::Event event;
        //while (window.pollEvent(event))
        //{
        //    if (event.type == sf::Event::Closed)
        //    {
        //        window.close();
        //        goto SHUTDOWN;
        //    }
        //}
        // 
        //map::draw_map(window);

        rml::update_rml();

        window.clear(sf::Color::White);

        // RmlUi uses OpenGL, so we need to push and pop the OpenGL states.
        window.pushGLStates();
        rml::render_rml();
        window.popGLStates();

        window.display();
    }

    SHUTDOWN:
    rml::cleanup_rml();

	return 0;
}