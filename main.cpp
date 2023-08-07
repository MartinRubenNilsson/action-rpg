#include <iostream>

int main()
{
    sf::Font font;
    if (!font.loadFromFile("assets/fonts/orange_juice.ttf"))
        return 1;

    sf::Text text;
    text.setFont(font);
    text.setString("Hello, SFML!");
    text.setCharacterSize(120);

	sf::RenderWindow window(sf::VideoMode(800, 600), "Hello, SFML!");
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        window.clear(sf::Color::Black);
        window.draw(text);
        window.display();
    }


	return 0;
}