#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <imgui-SFML.h>
#include "audio.h"
#include "physics.h"
#include "rml.h"
#include "map.h"
#include "behavior.h"
#include "ecs.h"
#include "rml_data_bindings.h"
#include "console.h"

#define TILE_SIZE 16
#define VIEW_WIDTH (TILE_SIZE * 24)
#define VIEW_HEIGHT (TILE_SIZE * 18)
#define WINDOW_WIDTH (VIEW_WIDTH * 3)
#define WINDOW_HEIGHT (VIEW_HEIGHT * 3)

int WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
    // CREATE WINDOW

    sf::VideoMode video_mode(WINDOW_WIDTH, WINDOW_HEIGHT);
    sf::Uint32 style = sf::Style::Titlebar | sf::Style::Close;
    sf::RenderWindow window(video_mode, "Hello, SFML!", style);

    sf::View view(sf::FloatRect(0, 0, VIEW_WIDTH, VIEW_HEIGHT));
    window.setView(view);

    // INITIALIZATION

    ImGui::SFML::Init(window);
    rml::initialize(window);
    physics::initialize();
    behavior::register_nodes();
    ecs::initialize();
    console::initialize(window);

    // LOAD ASSETS

    audio::load_music_and_sounds();
    rml::load_fonts_and_documents();
    behavior::load_trees();
    map::load_tilesets();
    map::load_maps();

    // OTHER STUFF

    behavior::write_node_models("assets/behaviors/models/models.xml"); // should only be done in debug builds
    audio::play_music("quiet_and_falling");

    // EXECUTE CONSOLE COMMANDS

    console::execute("map open dungeon");

    // GAME LOOP

    sf::Clock clock;
    bool debug_draw_physics = false;
    while (window.isOpen())
    {
        // EVENT HANDLING

        sf::Event event;
        while (window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(window, event);
            rml::process_event(event);

            if      (event.type == sf::Event::Closed)
            {
                window.close();
            }
            else if (event.type == sf::Event::KeyPressed)
            {
                if        (event.key.code == sf::Keyboard::F1) {
				    console::toggle_visible();
                } else if (event.key.code == sf::Keyboard::F2) {
                    debug_draw_physics = !debug_draw_physics;
                }
            }
        }

        // UPDATING

        sf::Time dt = clock.restart();
        ImGui::SFML::Update(window, dt);
        console::update(); // Must come after ImGui::SFML::Update but before Imgui::SFML::Render.
        rml::update();
        physics::update(dt.asSeconds());
        ecs::update(dt.asSeconds());

        // RENDERING

        window.clear();
        ecs::render(window);
        if (debug_draw_physics)
            physics::debug_draw(window);
        rml::render(); // Uses OpenGL, so make sure to call resetGLStates() after.
        window.resetGLStates();
        ImGui::SFML::Render(window);
        window.display();
    }

    // SHUTDOWN

    ecs::shutdown();
    physics::shutdown();
    rml::shutdown();
    ImGui::SFML::Shutdown();

	return 0;
}