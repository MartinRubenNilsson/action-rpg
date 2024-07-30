#include "stdafx.h"
#include "ui_rmlui_render_interface.h"
#include "graphics.h"
#include "console.h"

namespace ui
{
	int _viewport_width = 0;
	int _viewport_height = 0;
	int _previous_viewport[4] = { 0 };
	bool _previous_scissor_test_enabled = false;
	int _previous_scissor_box[4] = { 0 };

	Rml::TextureHandle _to_rml_handle(Handle<graphics::Texture> handle)
	{
		// PITFALL: 0 represents an invalid Rml::TextureHandle.
		return *(Rml::TextureHandle*)&handle;
	}

	Handle<graphics::Texture> _from_rml_handle(Rml::TextureHandle handle)
	{
		// PITFALL: 0 represents an invalid Rml::TextureHandle.
		return *(Handle<graphics::Texture>*)&handle;
	}

	void set_viewport(int viewport_width, int viewport_height)
	{
		_viewport_width = viewport_width;
		_viewport_height = viewport_height;
	}

	void prepare_render_state()
	{
		graphics::push_debug_group("UI");
		graphics::get_viewport(
			_previous_viewport[0],
			_previous_viewport[1],
			_previous_viewport[2],
			_previous_viewport[3]);
		_previous_scissor_test_enabled = graphics::get_scissor_test_enabled();
		graphics::get_scissor_box(
			_previous_scissor_box[0],
			_previous_scissor_box[1],
			_previous_scissor_box[2],
			_previous_scissor_box[3]);
		graphics::set_viewport(0, 0, _viewport_width, _viewport_height);
		graphics::bind_shader(graphics::ui_shader);
		graphics::set_uniform_mat4(graphics::ui_shader, "transform", graphics::IDENTITY_MATRIX);
		graphics::set_uniform_2f(graphics::ui_shader, "viewport_size", (float)_viewport_width, (float)_viewport_height);
		graphics::set_uniform_1i(graphics::ui_shader, "tex", 0);
	}

	void restore_render_state()
	{
		graphics::set_viewport(
			_previous_viewport[0],
			_previous_viewport[1],
			_previous_viewport[2],
			_previous_viewport[3]);
		graphics::set_scissor_test_enabled(_previous_scissor_test_enabled);
		graphics::set_scissor_box(
			_previous_scissor_box[0],
			_previous_scissor_box[1],
			_previous_scissor_box[2],
			_previous_scissor_box[3]);
		graphics::pop_debug_group();
	}

	void RmlUiRenderInterface::RenderGeometry(
		Rml::Vertex* vertices, int num_vertices,
		int* indices, int num_indices,
		const Rml::TextureHandle texture,
		const Rml::Vector2f& translation)
	{
		if (texture) {
			graphics::bind_texture(0, _from_rml_handle(texture));
			graphics::set_uniform_1i(graphics::ui_shader, "has_tex", 1);
		} else {
			graphics::set_uniform_1i(graphics::ui_shader, "has_tex", 0);
		}
		graphics::set_uniform_2f(graphics::ui_shader, "translation", translation.x, translation.y);
		graphics::draw_triangles(
			(graphics::Vertex*)vertices, (unsigned int)num_vertices,
			(unsigned int*)indices, (unsigned int)num_indices);
	}

	void RmlUiRenderInterface::EnableScissorRegion(bool enable)
	{
		graphics::set_scissor_test_enabled(enable);
	}

	void RmlUiRenderInterface::SetScissorRegion(int x, int y, int width, int height)
	{
		graphics::set_scissor_box(x, _viewport_height - (y + height), width, height);
	}

	bool RmlUiRenderInterface::LoadTexture(
		Rml::TextureHandle& texture_handle,
		Rml::Vector2i& texture_dimensions,
		const Rml::String& source)
	{
		const Handle<graphics::Texture> texture = graphics::load_texture(source, false);
		if (texture == Handle<graphics::Texture>()) return false;
		unsigned int width, height;
		graphics::get_texture_size(texture, width, height);
		texture_handle = _to_rml_handle(texture);
		texture_dimensions.x = width;
		texture_dimensions.y = height;
		return true;
	}

	bool RmlUiRenderInterface::GenerateTexture(
		Rml::TextureHandle& texture_handle,
		const Rml::byte* source,
		const Rml::Vector2i& source_dimensions)
	{
		const Handle<graphics::Texture> texture = graphics::create_texture({
			.debug_name = "rmlui texture",
			.width = (unsigned int)source_dimensions.x,
			.height = (unsigned int)source_dimensions.y,
			.initial_data = source });
		if (texture == Handle<graphics::Texture>()) return false;
		texture_handle = _to_rml_handle(texture);
		return true;
	}

	void RmlUiRenderInterface::ReleaseTexture(Rml::TextureHandle texture_handle)
	{
		graphics::destroy_texture(_from_rml_handle(texture_handle));
	}

	void RmlUiRenderInterface::SetTransform(const Rml::Matrix4f* transform)
	{
		if (!transform) {
			graphics::set_uniform_mat4(graphics::ui_shader, "transform", graphics::IDENTITY_MATRIX);
		} else if constexpr (std::is_same_v<Rml::Matrix4f, Rml::ColumnMajorMatrix4f>) {
			graphics::set_uniform_mat4(graphics::ui_shader, "transform", transform->data());
		} else if constexpr (std::is_same_v<Rml::Matrix4f, Rml::RowMajorMatrix4f>) {
			graphics::set_uniform_mat4(graphics::ui_shader, "transform", transform->Transpose().data());
		}
	}
}
