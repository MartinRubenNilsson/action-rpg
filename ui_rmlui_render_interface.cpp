#include "stdafx.h"
#include "ui_rmlui_render_interface.h"
#include <glad/glad.h> // TODO: remove dependency on glad, only use graphics.h
#include "graphics.h"

namespace ui
{
	int _viewport_width = 0;
	int _viewport_height = 0;
	int _previous_viewport[4] = { 0 };
	int _previous_scissor_box[4] = { 0 };

	void set_viewport(int viewport_width, int viewport_height)
	{
		_viewport_width = viewport_width;
		_viewport_height = viewport_height;
	}

	void prepare_render_state()
	{
		graphics::get_viewport(
			_previous_viewport[0],
			_previous_viewport[1],
			_previous_viewport[2],
			_previous_viewport[3]);
		graphics::get_scissor_box(
			_previous_scissor_box[0],
			_previous_scissor_box[1],
			_previous_scissor_box[2],
			_previous_scissor_box[3]);
		Rml::Matrix4f projection = Rml::Matrix4f::ProjectOrtho(
			0, (float)_viewport_width, (float)_viewport_height, 0, -10000, 10000);
		graphics::set_projection_matrix(projection.data());
		graphics::set_texture_matrix_to_identity();
		graphics::set_modelview_matrix_to_identity(); //IMPORTANT: must come last!
		graphics::set_viewport(0, 0, _viewport_width, _viewport_height);
		graphics::bind_shader(graphics::default_shader_id);
		graphics::set_shader_uniform_1i(graphics::default_shader_id, "tex", 0);
	}

	void restore_render_state()
	{
		graphics::set_viewport(
			_previous_viewport[0],
			_previous_viewport[1],
			_previous_viewport[2],
			_previous_viewport[3]);
		graphics::set_scissor_box(
			_previous_scissor_box[0],
			_previous_scissor_box[1],
			_previous_scissor_box[2],
			_previous_scissor_box[3]);
	}

	void RmlUiRenderInterface::RenderGeometry(Rml::Vertex* vertices, int /*num_vertices*/, int* indices, int num_indices, const Rml::TextureHandle texture, const Rml::Vector2f& translation)
	{
		glPushMatrix();
		glTranslatef(translation.x, translation.y, 0);
		glVertexPointer(2, GL_FLOAT, sizeof(Rml::Vertex), &vertices[0].position);
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Rml::Vertex), &vertices[0].colour);
		if (texture) {
			glEnable(GL_TEXTURE_2D);
			graphics::bind_texture(0, (int)texture - 1);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2, GL_FLOAT, sizeof(Rml::Vertex), &vertices[0].tex_coord);
		} else {
			glDisable(GL_TEXTURE_2D);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, indices);
		glPopMatrix();
	}

	void RmlUiRenderInterface::EnableScissorRegion(bool enable)
	{
		graphics::set_scissor_test_enabled(enable);
	}

	void RmlUiRenderInterface::SetScissorRegion(int x, int y, int width, int height)
	{
		graphics::set_scissor_box(x, _viewport_height - (y + height), width, height);
	}

	bool RmlUiRenderInterface::LoadTexture(Rml::TextureHandle& texture_handle, Rml::Vector2i& texture_dimensions, const Rml::String& source)
	{
		const int texture_id = graphics::load_texture(source);
		if (texture_id == -1) return false;
		unsigned int width, height;
		graphics::get_texture_size(texture_id, width, height);
		texture_handle = (Rml::TextureHandle)(texture_id + 1);
		texture_dimensions.x = width;
		texture_dimensions.y = height;
		return true;
	}

	bool RmlUiRenderInterface::GenerateTexture(Rml::TextureHandle& texture_handle, const Rml::byte* source, const Rml::Vector2i& source_dimensions)
	{
		const int texture_id = graphics::create_texture(
			source_dimensions.x, source_dimensions.y, 4, source, "rmlui texture");
		if (texture_id == -1) return false;
		texture_handle = (Rml::TextureHandle)(texture_id + 1);
		return true;
	}

	void RmlUiRenderInterface::ReleaseTexture(Rml::TextureHandle texture_handle) {
		//TODO: implement
		//LEAK!!!!!!
	}

	void RmlUiRenderInterface::SetTransform(const Rml::Matrix4f* transform)
	{
		if (!transform) {
			graphics::set_modelview_matrix_to_identity();
		} else if constexpr (std::is_same_v<Rml::Matrix4f, Rml::ColumnMajorMatrix4f>) {
			graphics::set_modelview_matrix(transform->data());
		} else if constexpr (std::is_same_v<Rml::Matrix4f, Rml::RowMajorMatrix4f>) {
			graphics::set_modelview_matrix(transform->Transpose().data());
		}
	}
}
