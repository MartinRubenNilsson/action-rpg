#pragma once
#include <RmlUi/Core/RenderInterface.h>

namespace ui {
	void set_viewport(int viewport_width, int viewport_height);
	void prepare_render_state();
	void restore_render_state();

	struct RmlUiRenderInterface : public Rml::RenderInterface {

		Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) override;
		void RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture) override;
		void ReleaseGeometry(Rml::CompiledGeometryHandle geometry) override;

		Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) override;
		Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions) override;
		void ReleaseTexture(Rml::TextureHandle texture_handle) override;

		void EnableScissorRegion(bool enable) override;
		void SetScissorRegion(Rml::Rectanglei region) override;

		void SetTransform(const Rml::Matrix4f* transform) override;
	};
}
