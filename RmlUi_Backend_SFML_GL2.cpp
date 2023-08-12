/*
 * This source file is part of RmlUi, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://github.com/mikke89/RmlUi
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 * Copyright (c) 2019-2023 The RmlUi Team, and contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "RmlUi_Backend.h"
#include "RmlUi_Platform_SFML.h"
#include "RmlUi_Renderer_GL2.h"
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/FileInterface.h>
#include <RmlUi/Core/Profiling.h>
#include <RmlUi/Debugger/Debugger.h>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

/**
    Custom render interface example for the SFML/GL2 backend.

    Overloads the OpenGL2 render interface to load textures through SFML's built-in texture loading functionality.
 */
class RenderInterface_GL2_SFML : public RenderInterface_GL2 {
public:
	// -- Inherited from Rml::RenderInterface --

	void RenderGeometry(Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rml::TextureHandle texture,
		const Rml::Vector2f& translation) override
	{
		if (texture)
		{
			sf::Texture::bind((sf::Texture*)texture);
			texture = RenderInterface_GL2::TextureEnableWithoutBinding;
		}

		RenderInterface_GL2::RenderGeometry(vertices, num_vertices, indices, num_indices, texture, translation);
	}

	bool LoadTexture(Rml::TextureHandle& texture_handle, Rml::Vector2i& texture_dimensions, const Rml::String& source) override
	{
		Rml::FileInterface* file_interface = Rml::GetFileInterface();
		Rml::FileHandle file_handle = file_interface->Open(source);
		if (!file_handle)
			return false;

		file_interface->Seek(file_handle, 0, SEEK_END);
		size_t buffer_size = file_interface->Tell(file_handle);
		file_interface->Seek(file_handle, 0, SEEK_SET);

		char* buffer = new char[buffer_size];
		file_interface->Read(buffer, buffer_size, file_handle);
		file_interface->Close(file_handle);

		sf::Texture* texture = new sf::Texture();
		texture->setSmooth(true);

		bool success = texture->loadFromMemory(buffer, buffer_size);

		delete[] buffer;

		if (success)
		{
			texture_handle = (Rml::TextureHandle)texture;
			texture_dimensions = Rml::Vector2i(texture->getSize().x, texture->getSize().y);
		}
		else
		{
			delete texture;
		}

		return success;
	}

	bool GenerateTexture(Rml::TextureHandle& texture_handle, const Rml::byte* source, const Rml::Vector2i& source_dimensions) override
	{
		sf::Texture* texture = new sf::Texture();
		texture->setSmooth(true);

		if (!texture->create(source_dimensions.x, source_dimensions.y))
		{
			delete texture;
			return false;
		}

		texture->update(source, source_dimensions.x, source_dimensions.y, 0, 0);
		texture_handle = (Rml::TextureHandle)texture;

		return true;
	}

	void ReleaseTexture(Rml::TextureHandle texture_handle) override { delete (sf::Texture*)texture_handle; }
};

/**
    Global data used by this backend.

    Lifetime governed by the calls to Backend::Initialize() and Backend::Shutdown().
 */
struct BackendData {
	SystemInterface_SFML system_interface;
	RenderInterface_GL2_SFML render_interface;
	sf::RenderWindow* window = nullptr;
};
static Rml::UniquePtr<BackendData> data;

// Updates the viewport and context dimensions, should be called whenever the window size changes.
static void UpdateWindowDimensions(Rml::Context* context)
{
	if (!data || !data->window)
		return;

	const int width = (int)data->window->getSize().x;
	const int height = (int)data->window->getSize().y;

	if (context)
		context->SetDimensions(Rml::Vector2i(width, height));

	data->render_interface.SetViewport(width, height);
}

void Backend::Initialize(sf::RenderWindow* window)
{
	RMLUI_ASSERT(!data && window);

	data = Rml::MakeUnique<BackendData>();
	data->window = window;
	data->system_interface.SetWindow(data->window); // So that the system interface can set the mouse cursor.

	UpdateWindowDimensions(nullptr);
}

void Backend::Shutdown()
{
	data.reset();
}

Rml::SystemInterface* Backend::GetSystemInterface()
{
	RMLUI_ASSERT(data);
	return &data->system_interface;
}

Rml::RenderInterface* Backend::GetRenderInterface()
{
	RMLUI_ASSERT(data);
	return &data->render_interface;
}

void Backend::ProcessEvent(Rml::Context* context, const sf::Event& ev)
{
	RMLUI_ASSERT(data && context);

	switch (ev.type)
	{
	case sf::Event::Resized:
		UpdateWindowDimensions(context);
		break;
	default:
	{
		sf::Event ev_copy = ev;
		RmlSFML::InputHandler(context, ev_copy);
		break;
	}
	}
}

void Backend::BeginFrame()
{
	RMLUI_ASSERT(data);
	data->render_interface.BeginFrame();
}

void Backend::PresentFrame()
{
	RMLUI_ASSERT(data);
	data->render_interface.EndFrame();
}
