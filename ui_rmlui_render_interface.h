#pragma once
#include <RmlUi/Core/RenderInterface.h>

namespace ui
{
	void set_viewport(int viewport_width, int viewport_height);
	void prepare_render_state();
	void restore_render_state();

	struct RmlUiRenderInterface : public Rml::RenderInterface
	{
		void RenderGeometry(Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rml::TextureHandle texture, const Rml::Vector2f& translation) override;
		Rml::CompiledGeometryHandle CompileGeometry(Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rml::TextureHandle texture);
		void RenderCompiledGeometry(Rml::CompiledGeometryHandle geometry, const Vector2f& translation);
		void ReleaseCompiledGeometry(Rml::CompiledGeometryHandle geometry);
		void EnableScissorRegion(bool enable) override;
		void SetScissorRegion(int x, int y, int width, int height) override;
		bool LoadTexture(Rml::TextureHandle& texture_handle, Rml::Vector2i& texture_dimensions, const Rml::String& source) override;
		bool GenerateTexture(Rml::TextureHandle& texture_handle, const Rml::byte* source, const Rml::Vector2i& source_dimensions) override;
		void ReleaseTexture(Rml::TextureHandle texture_handle) override;
		void SetTransform(const Rml::Matrix4f* transform) override;
	};
}
