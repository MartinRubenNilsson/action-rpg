#include "stdafx.h"
#include "ui_rmlui_render_interface.h"
#include "graphics.h"
#include "graphics_globals.h"
#include "console.h"

namespace ui
{
	int _viewport_width = 0;
	int _viewport_height = 0;
	int _previous_viewport[4] = { 0 };
	bool _previous_scissor_test_enabled = false;
	int _previous_scissor_box[4] = { 0 };

	Rml::TextureHandle _texture_handle_to_rml(Handle<graphics::Texture> handle)
	{
		// PITFALL: 0 represents an invalid Rml::TextureHandle.
		return *(Rml::TextureHandle*)&handle;
	}

	Handle<graphics::Texture> _texture_handle_from_rml(Rml::TextureHandle handle)
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
		graphics::bind_vertex_buffer(0, graphics::vertex_buffer, sizeof(graphics::Vertex));
		graphics::bind_index_buffer(graphics::index_buffer);
		graphics::pop_debug_group();
	}

	void RmlUiRenderInterface::RenderGeometry(
		Rml::Vertex* vertices, int num_vertices,
		int* indices, int num_indices,
		const Rml::TextureHandle texture,
		const Rml::Vector2f& translation)
	{
		if (texture) {
			graphics::bind_texture(0, _texture_handle_from_rml(texture));
			graphics::set_uniform_1i(graphics::ui_shader, "has_tex", 1);
		} else {
			graphics::set_uniform_1i(graphics::ui_shader, "has_tex", 0);
		}
		graphics::set_uniform_2f(graphics::ui_shader, "translation", translation.x, translation.y);
		graphics::update_buffer(graphics::vertex_buffer, (graphics::Vertex*)vertices, num_vertices * sizeof(graphics::Vertex));
		graphics::update_buffer(graphics::index_buffer, (unsigned int*)indices, num_indices * sizeof(unsigned int));
		graphics::draw_indexed(graphics::Primitives::TriangleList, (unsigned int)num_indices);
	}

	struct CompiledGeometry
	{
		Handle<graphics::Buffer> vertex_buffer;
		Handle<graphics::Buffer> index_buffer;
		unsigned int index_count = 0;
		Handle<graphics::Texture> texture;
	};

	Rml::CompiledGeometryHandle RmlUiRenderInterface::CompileGeometry(Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rml::TextureHandle texture)
	{
		CompiledGeometry* compiled_geometry = new CompiledGeometry();
		compiled_geometry->vertex_buffer = graphics::create_buffer({
			.debug_name = "rmlui vertex buffer",
			.type = graphics::BufferType::Vertex,
			.usage = graphics::Usage::StaticDraw,
			.byte_size = (unsigned int)sizeof(graphics::Vertex) * num_vertices,
			.initial_data = vertices });
		compiled_geometry->index_buffer = graphics::create_buffer({
			.debug_name = "rmlui index buffer",
			.type = graphics::BufferType::Index,
			.usage = graphics::Usage::StaticDraw,
			.byte_size = (unsigned int)sizeof(unsigned int) * num_indices,
			.initial_data = indices });
		compiled_geometry->index_count = (unsigned int)num_indices;
		compiled_geometry->texture = _texture_handle_from_rml(texture);
		return (Rml::CompiledGeometryHandle)compiled_geometry;
	}

	void RmlUiRenderInterface::RenderCompiledGeometry(Rml::CompiledGeometryHandle geometry, const Rml::Vector2f& translation)
	{
		CompiledGeometry* compiled_geometry = (CompiledGeometry*)geometry;
		if (compiled_geometry->texture != Handle<graphics::Texture>()) {
			graphics::bind_texture(0, compiled_geometry->texture);
			graphics::set_uniform_1i(graphics::ui_shader, "has_tex", 1);
		} else {
			graphics::set_uniform_1i(graphics::ui_shader, "has_tex", 0);
		}
		graphics::set_uniform_2f(graphics::ui_shader, "translation", translation.x, translation.y);
		graphics::bind_vertex_buffer(0, compiled_geometry->vertex_buffer, sizeof(graphics::Vertex));
		graphics::bind_index_buffer(compiled_geometry->index_buffer);
		graphics::draw_indexed(graphics::Primitives::TriangleList, compiled_geometry->index_count);
	}

	void RmlUiRenderInterface::ReleaseCompiledGeometry(Rml::CompiledGeometryHandle geometry)
	{
		CompiledGeometry* compiled_geometry = (CompiledGeometry*)geometry;
		graphics::destroy_buffer(compiled_geometry->vertex_buffer);
		graphics::destroy_buffer(compiled_geometry->index_buffer);
		delete compiled_geometry;
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
		const Handle<graphics::Texture> texture = graphics::load_texture(source);
		if (texture == Handle<graphics::Texture>()) return false;
		unsigned int width, height;
		graphics::get_texture_size(texture, width, height);
		texture_handle = _texture_handle_to_rml(texture);
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
		texture_handle = _texture_handle_to_rml(texture);
		return true;
	}

	void RmlUiRenderInterface::ReleaseTexture(Rml::TextureHandle texture_handle)
	{
		graphics::destroy_texture(_texture_handle_from_rml(texture_handle));
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
