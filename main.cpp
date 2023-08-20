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

#define TILE_SIZE 16
#define VIEW_WIDTH (TILE_SIZE * 24)
#define VIEW_HEIGHT (TILE_SIZE * 18)
#define WINDOW_WIDTH (VIEW_WIDTH * 3)
#define WINDOW_HEIGHT (VIEW_HEIGHT * 3)

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

    sf::VideoMode video_mode(WINDOW_WIDTH, WINDOW_HEIGHT);
    sf::Uint32 style = sf::Style::Titlebar | sf::Style::Close;
    _window.create(video_mode, "Hello, SFML!", style);

    sf::View view(sf::FloatRect(0, 0, VIEW_WIDTH, VIEW_HEIGHT));
    _window.setView(view);

    // INITIALIZATION

    ImGui::SFML::Init(_window);
    rml::initialize(_window);
    map::initialize(_window);
    console::initialize();

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
    bool debug_draw_physics = false;
    while (_window.isOpen())
    {
        // EVENT HANDLING

        sf::Event event;
        while (_window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(_window, event);
            rml::process_event(event);
            if (event.type == sf::Event::KeyPressed)
            {
                if        (event.key.code == sf::Keyboard::F1) {
				    console::toggle_visible();
                } else if (event.key.code == sf::Keyboard::F2) {
                    debug_draw_physics = !debug_draw_physics;
                }
            }
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
        rml::render(); // Uses OpenGL, so make sure to call resetGLStates() after.
        _window.resetGLStates();
        if (debug_draw_physics)
            map::get_world().DebugDraw();
        ImGui::SFML::Render(_window);
        _window.display();
    }

    // SHUTDOWN

    map::shutdown();
    rml::shutdown();
    ImGui::SFML::Shutdown();

	return 0;
}