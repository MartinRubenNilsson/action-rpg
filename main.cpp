#include "map.h"

int main()
{
    if (!map::load_map("assets/maps/test00.tmx"))
		return 1;

	sf::RenderWindow window(sf::VideoMode(800, 600), "Hello, SFML!");
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(sf::Color::White);
        map::draw_map_tiles(window);
        window.display();
    }

	return 0;
}