#include "stdafx.h"
#include "steam.h"
#include "filesystem.h"
#include "networking.h"
#include "window.h"
#include "window_events.h"
#include "audio.h"
#include "ui.h"
#include "ui_hud.h"
#include "ui_menus.h"
#include "ui_textbox.h"
#include "map.h"
#include "ecs.h"
#include "console.h"
#include "tiled.h"
#include "background.h"
#include "postprocessing.h"
#include "settings.h"
#include "graphics.h"
#include "graphics_globals.h"
#include "shapes.h"
#include "sprites.h"
#include "renderdoc.h"
#include "imgui_backends.h"

int main(int argc, char* argv[])
{
    if (steam::restart_app_if_necessary()) {
        return EXIT_FAILURE;
    }
    steam::initialize(); // Fails silently if Steam is not running.
    filesystem::initialize();
	networking::initialize();
#ifdef _DEBUG_RENDERDOC
    renderdoc::initialize();
#endif
    window::initialize();
#ifdef _DEBUG_IMGUI
    graphics::initialize();
    graphics::initialize_globals();
    imgui_backends::initialize();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    ImGui::GetIO().Fonts->AddFontFromFileTTF("assets/fonts/Consolas.ttf", 18);
    {
        // https://github.com/ocornut/imgui/issues/707#issuecomment-252413954
        ImGuiStyle& style = ImGui::GetStyle();
        style.Colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 0.90f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.09f, 0.15f, 1.00f);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        style.Colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.10f, 0.85f);
        style.Colors[ImGuiCol_Border] = ImVec4(0.70f, 0.70f, 0.70f, 0.65f);
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.00f, 0.00f, 0.01f, 1.00f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.90f, 0.80f, 0.80f, 0.40f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.90f, 0.65f, 0.65f, 0.45f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.83f);
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.40f, 0.40f, 0.80f, 0.20f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.00f, 0.00f, 0.87f);
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.01f, 0.01f, 0.02f, 0.80f);
        style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.25f, 0.30f, 0.60f);
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.55f, 0.53f, 0.55f, 0.51f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.56f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.91f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.90f, 0.90f, 0.83f);
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.70f, 0.70f, 0.70f, 0.62f);
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.30f, 0.30f, 0.30f, 0.84f);
        style.Colors[ImGuiCol_Button] = ImVec4(0.48f, 0.72f, 0.89f, 0.49f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.50f, 0.69f, 0.99f, 0.68f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.80f, 0.50f, 0.50f, 1.00f);
        style.Colors[ImGuiCol_Header] = ImVec4(0.30f, 0.69f, 1.00f, 0.53f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.44f, 0.61f, 0.86f, 1.00f);
        style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.38f, 0.62f, 0.83f, 1.00f);
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.85f);
        style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
        style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
        style.Colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
        style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
    }
#endif
    console::initialize();
    audio::initialize();
    ui::initialize();
    ecs::initialize();

#ifdef _DEBUG_GRAPHICS
    // HACK: We should be using a post-build event to copy the shaders,
    // but then it doesn't run when only debugging and not recompiling,
    // which is annoying when you've changed a shader but not the code,
    // because then the new shader doesn't get copied.
    system("copy /Y ..\\*.vert .\\assets\\shaders\\");
    system("copy /Y ..\\*.frag .\\assets\\shaders\\");
#endif

    for (const filesystem::File& file : filesystem::get_files_in_directory("assets/audio/banks")) {
        if (file.format != filesystem::FileFormat::FmodStudioBank) continue;
        audio::load_bank_from_file(file.path);
    }
    for (const filesystem::File& file : filesystem::get_files_in_directory("assets/fonts")) {
        if (file.format != filesystem::FileFormat::TrueTypeFont) continue;
        ui::load_font_from_file(file.path);
    }
    for (const filesystem::File& file : filesystem::get_files_in_directory("assets/ui")) {
        if (file.format != filesystem::FileFormat::RmlUiDocument) continue;
        ui::load_document_from_file(file.path);
    }
    for (const filesystem::File& file : filesystem::get_files_in_directory("assets/tiled")) {
        if (file.format != filesystem::FileFormat::TiledTileset) continue;
        tiled::load_tileset_from_file(file.path);
    }
    for (const filesystem::File& file : filesystem::get_files_in_directory("assets/tiled")) {
        if (file.format != filesystem::FileFormat::TiledTemplate) continue;
        tiled::load_template_from_file(file.path);
    }
    for (const filesystem::File& file : filesystem::get_files_in_directory("assets/tiled")) {
        if (file.format != filesystem::FileFormat::TiledMap) continue;
        tiled::load_map_from_file(file.path);
    }

    ui::add_event_listeners(); // Must come after loading RML documents.

    settings::load_from_file();
    settings::apply();

    window::set_visible(true);

    // PREPARE FOR GAME LOOP

#ifdef _DEBUG
    console::execute(argc, argv);
#else
    console::execute("execute_script assets/scripts/martin_debug.script");
#endif

    bool debug_stats = false;
    bool debug_textboxes = false;
    bool debug_textures = false;

    // GAME LOOP

    float app_time = (float)window::get_elapsed_time();
	float game_time = 0.0;

    while (!window::should_close()) {

        const float new_app_time = (float)window::get_elapsed_time();
		const float app_delta_time = new_app_time - app_time;
        app_time = new_app_time;

        steam::run_message_loop();
        window::poll_events();

#ifdef _DEBUG_IMGUI
        imgui_backends::new_frame();
#endif

        // PROCESS WINDOW EVENTS
        {
            window::Event ev{};
            while (window::pop_event(ev)) {
                if (ev.type == window::EventType::WindowClose) {
                    window::set_should_close(true);
                }  else if (ev.type == window::EventType::KeyPress) {
#ifdef _DEBUG
                    if (ev.key.code == window::Key::GraveAccent) {
                        console::toggle_visible();
                    } else if (ev.key.code == window::Key::F1) {
                        debug_stats = !debug_stats;
                    } else if (ev.key.code == window::Key::F2) {
                        debug_textboxes = !debug_textboxes;
                    } else if (ev.key.code == window::Key::F3) {
                        ecs::debug_flags ^= ecs::DEBUG_PHYSICS;
                    } else if (ev.key.code == window::Key::F4) {
                        ecs::debug_flags ^= ecs::DEBUG_AI;
                    } else if (ev.key.code == window::Key::F5) {
                        ecs::debug_flags ^= ecs::DEBUG_PLAYER;
                    } else if (ev.key.code == window::Key::F6) {
                        ui::debug = !ui::debug;
                    } else if (ev.key.code == window::Key::F7) {
                        map::debug = !map::debug;
                    } else if (ev.key.code == window::Key::F8) {
                        debug_textures = !debug_textures;
                    }
#endif // _DEBUG
                }

#ifdef _DEBUG_IMGUI
                if (ev.type == window::EventType::KeyPress && ImGui::GetIO().WantCaptureKeyboard) {
                    continue;
                }
#endif
                console::process_window_event(ev);
                ui::process_window_event(ev);
                if (!ui::is_menu_or_textbox_visible()) {
                    ecs::process_window_event(ev);
                }
            }
        }

        // PROCESS UI EVENTS
        {
            ui::Event ev;
            while (ui::get_next_event(ev)) {
                switch (ev.type) {
                case ui::EventType::PlayGame: {
					background::set_type(background::Type::None);
					map::open("summer_forest_00");
                } break;
                case ui::EventType::RestartMap: {
                    map::reset();
                } break;
                case ui::EventType::GoToMainMenu: {
                    background::set_type(background::Type::MountainDusk);
					map::close(0.f);
                } break;
                case ui::EventType::QuitApp: {
                    window::set_should_close(true);
                } break;
				}
			}
        }

        // UPDATE

        audio::update();
        console::update(app_delta_time);
        background::update(app_delta_time);
        ui::update(app_delta_time);
        map::update(app_delta_time);

        float game_delta_time = app_delta_time;
        if (steam::is_overlay_active()) {
            game_delta_time = 0.0;
        }
        if (ui::is_menu_or_textbox_visible()) {
            game_delta_time = 0.0;
        }
        if (map::get_transition_progress() != 0.f) {
			game_delta_time = 0.0; // pause game while map is transitioning
        }

		game_time += game_delta_time;

        ecs::update(game_delta_time);
        postprocessing::update(game_delta_time);

        // RENDER

        const float pixel_scale = 1.f;
		
        sprites::clear_drawing_statistics();

        Vector2f camera_min;
        Vector2f camera_max;
        ecs::get_camera_bounds(camera_min, camera_max);

        {
            const Vector2f camera_center = (camera_min + camera_max) / 2.f;
            const Vector2f camera_size = camera_max - camera_min;

            // Rotation components
            const float angle = 0.f;
            const float cosine = std::cos(angle);
            const float sine = std::sin(angle);
            const float tx = -camera_center.x * cosine - camera_center.y * sine + camera_center.x;
            const float ty = camera_center.x * sine - camera_center.y * cosine + camera_center.y;

            // Projection components
            const float a = 2.f / camera_size.x;
            const float b = -2.f / camera_size.y;
            const float c = -a * camera_center.x;
            const float d = -b * camera_center.y;

            // Create the projection matrix
            const float projection_matrix[16] = {
				a * cosine, -b * sine, 0.f, 0.f,
				a * sine, b * cosine, 0.f, 0.f,
				0.f, 0.f, 1.f, 0.f,
				a * tx + c, b * ty + d, 0.f, 1.f
			};

            graphics::FrameUniformBlock frame_ub{};
            memcpy(frame_ub.view_proj_matrix, projection_matrix, sizeof(projection_matrix));
			frame_ub.app_time = app_time;
			frame_ub.game_time = game_time;
            graphics::update_buffer(graphics::frame_uniform_buffer, &frame_ub, sizeof(frame_ub));
        }

		constexpr float CLEAR_COLOR[4] = { 0.f, 0.f, 0.f, 1.f };
        graphics::clear_framebuffer(graphics::game_framebuffer_0, CLEAR_COLOR);
        graphics::bind_framebuffer(graphics::game_framebuffer_0);
        graphics::set_viewport({ .width = GAME_FRAMEBUFFER_WIDTH, .height = GAME_FRAMEBUFFER_HEIGHT });

        background::draw_sprites(camera_min, camera_max);
        ecs::draw_sprites(camera_min, camera_max);

        switch (ui::get_top_menu()) {
        case ui::MenuType::Pause:
        case ui::MenuType::Settings:
        case ui::MenuType::Credits: {
            postprocessing::set_gaussian_blur_iterations(2);
        } break;
        default: {
            postprocessing::set_gaussian_blur_iterations(0);
        } break;
        }

        postprocessing::set_darkness_intensity(map::is_dark() ? 0.95f : 0.f);
        postprocessing::set_screen_transition_progress(map::get_transition_progress());
        postprocessing::render(camera_min, camera_max);

		if (!window::get_minimized()) {
            graphics::ScopedDebugGroup debug_group("Upscale to window");
            graphics::bind_default_framebuffer();
            graphics::bind_shader(graphics::fullscreen_shader);
            graphics::bind_texture(0, graphics::get_framebuffer_texture(graphics::game_framebuffer_0));
            int window_framebuffer_width = 0;
            int window_framebuffer_height = 0;
            window::get_framebuffer_size(window_framebuffer_width, window_framebuffer_height);
            graphics::set_viewport({ .width = (float)window_framebuffer_width, .height = (float)window_framebuffer_height });
            graphics::draw(graphics::Primitives::TriangleList, 3); // draw a fullscreen-covering triangle
        }

#ifdef _DEBUG
        ecs::add_debug_shapes_to_render_queue();
        shapes::draw("ECS debug", camera_min, camera_max);
        shapes::update_lifetimes(game_delta_time);
#endif
        ui::render();

#ifdef _DEBUG_IMGUI
        // CREATE DEBUG STATS WINDOW

        if (debug_stats) {
            constexpr float smoothing_factor = 0.99f;
            static float smoothed_dt = 0.f;
            static float smoothed_fps = 0.f;
            static float dt_buffer[256] = { 0.f };
            static float fps_buffer[256] = { 0.f };
            static int buffer_offset = 0;
            dt_buffer[buffer_offset] = app_delta_time;
            fps_buffer[buffer_offset] = 1.f / app_delta_time;
            buffer_offset = (buffer_offset + 1) % 256;
            smoothed_dt = smoothing_factor * smoothed_dt + (1.f - smoothing_factor) * app_delta_time;
            smoothed_fps = smoothing_factor * smoothed_fps + (1.f - smoothing_factor) / app_delta_time;
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);
            char overlay_text[64];
            sprintf(overlay_text, "%.f us", smoothed_dt * 1'000'000.f);
            ImGui::PlotLines("##dt", dt_buffer, 256, buffer_offset, overlay_text, 0.f, 0.01f, ImVec2(0, 80));
            sprintf(overlay_text, "%.f FPS", smoothed_fps);
            ImGui::PlotLines("##fps", fps_buffer, 256, buffer_offset, overlay_text, 0.f, 600.f, ImVec2(0, 80));
            ImGui::Value("Sprites Drawn", sprites::get_sprites_drawn());
            ImGui::Value("Batches Drawn", sprites::get_batches_drawn());
            ImGui::Value("Largest Batch", sprites::get_largest_batch_sprite_count());
            ImGui::End();
        }

        if (debug_textboxes) {
			ui::show_textbox_debug_window();
		}
        if (debug_textures) {
            graphics::show_texture_debug_window();
        }

#endif // DEBUG_IMGUI

#ifdef _DEBUG_IMGUI
        {
            graphics::ScopedDebugGroup debug_group("ImGui");
            imgui_backends::render();
        }
#endif

        window::swap_buffers();

#ifdef _DEBUG_RENDERDOC
        renderdoc::open_capture_folder_if_capturing();
#endif
    }

    // SHUTDOWN

    ecs::shutdown();
    ui::shutdown();
    audio::shutdown();
    graphics::shutdown();
#ifdef _DEBUG_IMGUI
    imgui_backends::shutdown();
#endif
    window::shutdown();
	networking::shutdown();
    steam::shutdown();

    return EXIT_SUCCESS;
}