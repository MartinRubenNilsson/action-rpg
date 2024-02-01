#include "stdafx.h"
#include <imgui-SFML.h>
#include <imgui.h>
#include "window.h"
#include "audio.h"
#include "fonts.h"
#include "shaders.h"
#include "ui.h"
#include "ui_menus.h"
#include "map.h"
#include "ecs.h"
#include "console.h"
#include "tiled.h"
#include "background.h"
#include "postprocess.h"

#pragma comment(lib, "winmm") // SFML requires this
#ifdef _DEBUG
#pragma comment(lib, "sfml-main-d")
#else
#pragma comment(lib, "sfml-main")
#endif

int main(int argc, char* argv[])
{
    sf::RenderWindow window;

    // INITIALIZE

    window::initialize(window);
    ImGui::SFML::Init(window, false);
    ImGui::GetIO().Fonts->AddFontFromFileTTF("assets/fonts/Consolas.ttf", 24);
    ImGui::SFML::UpdateFontTexture();
    audio::initialize();
    ecs::initialize();
    console::initialize();
    ui::initialize(window);

    sf::RenderTexture render_texture;
    render_texture.create(window.getSize().x, window.getSize().y);

    // LOAD ASSETS

    fonts::load_assets("assets/fonts");
    shaders::load_assets("assets/shaders");
    audio::load_bank_files("assets/audio/banks");
    tiled::load_assets("assets/tiled");
    ui::load_ttf_fonts("assets/fonts");
    ui::load_rml_documents("assets/ui");
    background::load_assets();

    // GAME LOOP

#ifdef _DEBUG
    console::execute(argc, argv);
#else
    background::type = background::Type::MountainDusk;
    ui::push_menu(ui::MenuType::Main);
#endif

    sf::Clock clock;
    float smoothed_dt = 0.f;
    float smoothed_fps = 0.f;
    bool show_stats = false;
    while (window.isOpen()) {

        // MESSAGE/EVENT LOOP

        sf::Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed)
                window.close();
            else if (ev.type == sf::Event::Resized)
                render_texture.create(ev.size.width, ev.size.height);
            else if (ev.type == sf::Event::KeyPressed) {
                if (ev.key.code == sf::Keyboard::Backslash)
                    console::toggle_visible();
                else if (ev.key.code == sf::Keyboard::F1)
                    ; // reserved
                else if (ev.key.code == sf::Keyboard::F2)
                    ecs::debug_flags ^= ecs::DEBUG_PHYSICS;
                else if (ev.key.code == sf::Keyboard::F3)
                    ecs::debug_flags ^= ecs::DEBUG_PIVOTS;
                else if (ev.key.code == sf::Keyboard::F4)
                    ecs::debug_flags ^= ecs::DEBUG_AI;
                else if (ev.key.code == sf::Keyboard::F5)
                    ecs::debug_flags ^= ecs::DEBUG_PLAYER;
                else if (ev.key.code == sf::Keyboard::F6)
                    show_stats = !show_stats;
            }
            ImGui::SFML::ProcessEvent(window, ev);
            if (ev.type == sf::Event::KeyPressed && ImGui::GetIO().WantCaptureKeyboard)
                continue;
            console::process_event(ev);
            ui::process_event(ev);
            if (!ui::should_pause_game())
                ecs::process_event(ev);
        }

        // HANDLE UI REQUESTS

        switch (ui::poll_event()) {
        case ui::Event::PlayGame:
            background::type = background::Type::None;
            map::open("summer_forest");
            break;
        case ui::Event::GoBackToMainMenu:
            background::type = background::Type::MountainDusk;
            map::close();
            break;
        case ui::Event::QuitApp:
            window.close();
            break;
        }

        // UPDATE

        sf::Time dt = clock.restart();
        smoothed_dt = 0.9f * smoothed_dt + 0.1f * dt.asSeconds();
        smoothed_fps = 0.9f * smoothed_fps + 0.1f / dt.asSeconds();
        ImGui::SFML::Update(window, dt);
        console::update(dt.asSeconds()); // Must come after ImGui::SFML::Update but before Imgui::SFML::Render.
        background::update(dt.asSeconds());
        map::update();
        audio::update();
        ui::update(dt.asSeconds());
        if (!ui::should_pause_game())
            ecs::update(dt.asSeconds());

        // RENDER

        render_texture.clear();
        background::render(render_texture);
        ecs::render(render_texture);
        ui::render(); // Uses OpenGL, so make sure to call resetGLStates() after.
        render_texture.resetGLStates();
        render_texture.display();
        window.clear();
        window.setView(window.getDefaultView());

#if 0
        ImGui::Begin("Shockwave shader");
        static float shockwave_time = 0.0f;
        static sf::Vector2f shockwave_center(0.5f, 0.5f);
        ImGui::SliderFloat("Time", &shockwave_time, 0.0f, 5.0f);
        ImGui::SliderFloat("Center X", &shockwave_center.x, 0.0f, (float)window.getSize().x);
        ImGui::SliderFloat("Center Y", &shockwave_center.y, 0.0f, (float)window.getSize().y);
        ImGui::End();
        postprocess::shockwave(window, render_texture, shockwave_time, shockwave_center);
#else
        postprocess::copy(window, render_texture);
#endif

#ifdef _DEBUG
        if (show_stats) {
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::Text("dt: %.3f ms", smoothed_dt * 1000.f);
            ImGui::Text("FPS: %.1f", smoothed_fps);
            ImGui::End();
        }
#endif

        ImGui::SFML::Render(window);
        window.display();
    }

    // UNLOAD ASSETS

    tiled::unload_assets(); 
    background::unload_assets();
    shaders::unload_assets();
    fonts::unload_assets();

    // SHUTDOWN

    ecs::shutdown();
    ui::shutdown();
    audio::shutdown();
    ImGui::SFML::Shutdown();

    return 0;
}