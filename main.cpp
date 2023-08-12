#include "map.h"
#include "html.h"

int main()
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

    html::startup_html(&window);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        //window.clear(sf::Color::White);
        //map::draw_map(window);
        //window.display();

        html::render_html();
    }

    html::cleanup_html();

	return 0;
}