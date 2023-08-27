#pragma once
#include "RmlUi_Renderer_GL2.h"

class RenderInterface_GL2_SFML : public RenderInterface_GL2
{
public:
	void RenderGeometry(
		Rml::Vertex* vertices, int num_vertices,
		int* indices, int num_indices,
		Rml::TextureHandle texture,
		const Rml::Vector2f& translation) override;

	bool LoadTexture(
		Rml::TextureHandle& texture_handle,
		Rml::Vector2i& texture_dimensions,
		const Rml::String& source) override;

	bool GenerateTexture(
		Rml::TextureHandle& texture_handle,
		const Rml::byte* source,
		const Rml::Vector2i& source_dimensions) override;

	void ReleaseTexture(Rml::TextureHandle texture_handle) override;
};

