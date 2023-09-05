#include "RmlUi_Renderer_GL2_SFML.h"
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/FileInterface.h>
#include <SFML/Graphics.hpp>

void RenderInterface_GL2_SFML::RenderGeometry(
	Rml::Vertex* vertices, int num_vertices,
	int* indices, int num_indices,
	Rml::TextureHandle texture,
	const Rml::Vector2f& translation)
{
	if (texture)
	{
		sf::Texture::bind((sf::Texture*)texture);
		texture = RenderInterface_GL2::TextureEnableWithoutBinding;
	}

	RenderInterface_GL2::RenderGeometry(
		vertices, num_vertices,
		indices, num_indices,
		texture,
		translation);
}

bool RenderInterface_GL2_SFML::LoadTexture(
	Rml::TextureHandle& texture_handle,
	Rml::Vector2i& texture_dimensions,
	const Rml::String& source)
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
	//texture->setSmooth(true); // CHANGE: commented out

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

bool RenderInterface_GL2_SFML::GenerateTexture(
	Rml::TextureHandle& texture_handle,
	const Rml::byte* source,
	const Rml::Vector2i& source_dimensions)
{
	sf::Texture* texture = new sf::Texture();
	//texture->setSmooth(true); // CHANGE: commented out

	if (!texture->create(source_dimensions.x, source_dimensions.y))
	{
		delete texture;
		return false;
	}

	texture->update(source, source_dimensions.x, source_dimensions.y, 0, 0);
	texture_handle = (Rml::TextureHandle)texture;

	return true;
}

void RenderInterface_GL2_SFML::ReleaseTexture(Rml::TextureHandle texture_handle)
{
	delete (sf::Texture*)texture_handle;
}
