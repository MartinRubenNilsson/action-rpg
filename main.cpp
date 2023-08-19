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

sf::RenderWindow _window;

void close_window()
{
    _window.close();
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
    // CREATE WINDOW

    sf::VideoMode video_mode(MAP_WIDTH * 3, MAP_HEIGHT * 3);
    sf::Uint32 style = sf::Style::Titlebar | sf::Style::Close;
    _window.create(video_mode, "Hello, SFML!", style);

    sf::View view(sf::FloatRect(0, 0, MAP_WIDTH, MAP_HEIGHT));
    _window.setView(view);

    // STARTUP

    ImGui::SFML::Init(_window);
    rml::startup(&_window);
    console::startup();

    // LOAD ASSETS

    rml::load_all();
    map::load_all();

    // EXECUTE CONSOLE COMMANDS

    console::execute("map open dungeon");

    // GAME LOOP

    sf::Clock clock;
    while (_window.isOpen())
    {
        // EVENT HANDLING

        sf::Event event;
        while (_window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(_window, event);
            rml::process_event(event);
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F1)
				console::toggle_visible();
            if (event.type == sf::Event::Closed)
                _window.close();
        }

        // UPDATING

        sf::Time dt = clock.restart();
        ImGui::SFML::Update(_window, dt);
        console::update(); // Must come after ImGui::SFML::Update but before Imgui::SFML::Render.
        rml::update();
        game::update(map::get_registry(), dt.asSeconds());

        // RENDERING

        _window.clear();
        game::render(map::get_registry(), _window);
        _window.pushGLStates();
        rml::render(); // Uses OpenGL, so we need to push and pop the OpenGL states before and after.
        _window.popGLStates();
        ImGui::SFML::Render(_window);
        _window.display();
    }

    // CLEANUP

    rml::cleanup();
    ImGui::SFML::Shutdown();

	return 0;
}