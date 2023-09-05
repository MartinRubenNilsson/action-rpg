#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <imgui-SFML.h>
#include <imgui.h>
#include "window.h"
#include "audio.h"
#include "physics.h"
#include "ui.h"
#include "map.h"
#include "behavior.h"
#include "ecs.h"
#include "console.h"
#include "data.h"

#include "ui_textbox.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR command_line, int)
{
    // CREATE WINDOW

    sf::RenderWindow& window = window::create();

    // INITIALIZE SUBSYSTEMS

    ImGui::SFML::Init(window);
    physics::initialize();
    behavior::register_nodes();
    ecs::initialize();
    console::initialize();
    ui::initialize(window);

    // LOAD ASSETS

    audio::load_music_and_sounds();
    ui::load_fonts_and_documents();
    behavior::load_trees();
    map::load_tilesets();
    map::load_maps();
    data::load_texts();

    // OTHER STUFF

#ifdef _DEBUG
    console::write_help_file("assets/scripts/help.txt");
    behavior::write_node_models_file("assets/behaviors/models/models.xml");
#endif

    ui::set_textbox_sprite("iron-helmet");
    ui::set_textbox_text(
        "Hello, world! My name is <span style='color: red;'>Martin</span> "
        "and I'm the best programmer ever!<br/>"
        "<span style='color: #ffd700;'>...and good-lookin', too!</span>");

    // EXECUTE STARTUP COMMANDS

    if (strlen(command_line) > 0)
        console::execute(command_line);

    // GAME LOOP

    sf::Clock clock;
    bool debug_draw_physics = false;
    while (window.isOpen())
    {
        // EVENT HANDLING

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::F1)
                    console::toggle_visible();
                if (event.key.code == sf::Keyboard::F2)
                    debug_draw_physics = !debug_draw_physics;
            }

            ImGui::SFML::ProcessEvent(window, event);
            if (event.type == sf::Event::KeyPressed && ImGui::GetIO().WantCaptureKeyboard)
				continue;

            console::process_event(event);
            ui::process_event(event);
        }

        // UPDATING

        sf::Time dt = clock.restart();
        ImGui::SFML::Update(window, dt);
        console::update(); // Must come after ImGui::SFML::Update but before Imgui::SFML::Render.
        map::update();
        physics::update(dt.asSeconds());
        ecs::update(dt.asSeconds());
        ui::update();

        // RENDERING

        window.clear();
        ecs::render(window);
        if (debug_draw_physics)
            physics::debug_draw(window);
        ui::render(); // Uses OpenGL, so make sure to call resetGLStates() after.
        window.resetGLStates();
        ImGui::SFML::Render(window);
        window.display();
    }

    // SHUTDOWN

    ecs::shutdown();
    physics::shutdown();
    ui::shutdown();
    ImGui::SFML::Shutdown();

	return 0;
}