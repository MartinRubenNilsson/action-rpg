#pragma once
#include <RmlUi/Core/RenderInterface.h>

class RenderInterface_SFML : public Rml::RenderInterface {
public:
	void SetViewport(int viewport_width, int viewport_height);
	void BeginFrame();
	void EndFrame();
	void Clear();

	void RenderGeometry(Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rml::TextureHandle texture, const Rml::Vector2f& translation) override;
	
	void EnableScissorRegion(bool enable) override;
	void SetScissorRegion(int x, int y, int width, int height) override;

	bool LoadTexture(Rml::TextureHandle& texture_handle, Rml::Vector2i& texture_dimensions, const Rml::String& source) override;
	bool GenerateTexture(Rml::TextureHandle& texture_handle, const Rml::byte* source, const Rml::Vector2i& source_dimensions) override;
	void ReleaseTexture(Rml::TextureHandle texture_handle) override;

	void SetTransform(const Rml::Matrix4f* transform) override;

private:
	int _viewport_width = 0;
	int _viewport_height = 0;
	bool _has_transform = false;
};
