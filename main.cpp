#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <imgui-SFML.h>
#include "audio.h"
#include "rml.h"
#include "map.h"
#include "game.h"
#include "rml_data_bindings.h"
#include "console.h"

#define VIEW_WIDTH 480
#define VIEW_HEIGHT 320

sf::RenderWindow _window;

sf::RenderWindow& get_window()
{
	return _window;
}

void close_window()
{
    _window.close();
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
    // CREATE WINDOW

    sf::VideoMode video_mode(VIEW_WIDTH * 3, VIEW_HEIGHT * 3);
    sf::Uint32 style = sf::Style::Titlebar | sf::Style::Close;
    _window.create(video_mode, "Hello, SFML!", style);

    sf::View view(sf::FloatRect(0, 0, VIEW_WIDTH, VIEW_HEIGHT));
    _window.setView(view);

    // STARTUP

    ImGui::SFML::Init(_window);
    rml::startup(&_window);
    console::startup();

    // LOAD ASSETS

    audio::load_music_and_sounds();
    rml::load_fonts_and_documents();
    map::load_tilesets();
    map::load_maps();

    // OTHER STUFF

    audio::play_music("quiet_and_falling");

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
        game::update(dt.asSeconds());

        // RENDERING

        _window.clear();
        game::render(_window);
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