#pragma once
#include <RmlUi/Core/RenderInterface.h>

class RenderInterface_GL2 : public Rml::RenderInterface {
public:
	RenderInterface_GL2();

	// The viewport should be updated whenever the window size changes.
	void SetViewport(int viewport_width, int viewport_height);

	// Sets up OpenGL states for taking rendering commands from RmlUi.
	void BeginFrame();
	void EndFrame();

	// Optional, can be used to clear the framebuffer.
	void Clear();

	// -- Inherited from Rml::RenderInterface --

	void RenderGeometry(Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rml::TextureHandle texture,
		const Rml::Vector2f& translation) override;

	void EnableScissorRegion(bool enable) override;
	void SetScissorRegion(int x, int y, int width, int height) override;

	bool LoadTexture(Rml::TextureHandle& texture_handle, Rml::Vector2i& texture_dimensions, const Rml::String& source) override;
	bool GenerateTexture(Rml::TextureHandle& texture_handle, const Rml::byte* source, const Rml::Vector2i& source_dimensions) override;
	void ReleaseTexture(Rml::TextureHandle texture_handle) override;

	void SetTransform(const Rml::Matrix4f* transform) override;

	// Can be passed to RenderGeometry() to enable texture rendering without changing the bound texture.
	static const Rml::TextureHandle TextureEnableWithoutBinding = Rml::TextureHandle(-1);

private:
	int viewport_width = 0;
	int viewport_height = 0;
	bool transform_enabled = false;
};

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
