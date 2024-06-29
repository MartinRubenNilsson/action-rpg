#include "stdafx.h"
#include "ui_rmlui_interfaces.h"
#include <glad/glad.h>
#include "window.h"
#include "graphics.h"

namespace ui
{
	double RmlUiSystemInterface::GetElapsedTime() {
		return window::get_elapsed_time();
	}

	void RmlUiSystemInterface::SetMouseCursor(const Rml::String& cursor_name)
	{
#if 0
		if (cursor_name.empty() || cursor_name == "arrow")
			window::set_cursor(sf::Cursor::Arrow);
		else if (cursor_name == "move")
			window::set_cursor(sf::Cursor::SizeAll);
		else if (cursor_name == "pointer")
			window::set_cursor(sf::Cursor::Hand);
		else if (cursor_name == "resize")
			window::set_cursor(sf::Cursor::SizeTopLeftBottomRight);
		else if (cursor_name == "cross")
			window::set_cursor(sf::Cursor::Cross);
		else if (cursor_name == "text")
			window::set_cursor(sf::Cursor::Text);
		else if (cursor_name == "unavailable")
			window::set_cursor(sf::Cursor::NotAllowed);
		else if (cursor_name.starts_with("rmlui-scroll"))
			window::set_cursor(sf::Cursor::SizeAll);
#endif
	}

	void RmlUiSystemInterface::SetClipboardText(const Rml::String& text_utf8) {
		window::set_clipboard_string(text_utf8);
	}

	void RmlUiSystemInterface::GetClipboardText(Rml::String& text) {
		text = window::get_clipboard_string();
	}

	void RmlUiRenderInterface::SetViewport(int in_viewport_width, int in_viewport_height)
	{
		_viewport_width = in_viewport_width;
		_viewport_height = in_viewport_height;
	}

	void RmlUiRenderInterface::BeginFrame()
	{
		RMLUI_ASSERT(_viewport_width >= 0 && _viewport_height >= 0);
		graphics::set_viewport(0, 0, _viewport_width, _viewport_height);

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		Rml::Matrix4f projection = Rml::Matrix4f::ProjectOrtho(0, (float)_viewport_width, (float)_viewport_height, 0, -10000, 10000);
		graphics::set_projection_matrix(projection.data());
		graphics::set_texture_matrix_to_identity();
		graphics::set_modelview_matrix_to_identity(); //IMPORTANT: must come last!

		_has_transform = false;
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
		if (!enable) {
			glDisable(GL_SCISSOR_TEST);
			glDisable(GL_STENCIL_TEST);
		} else if (!_has_transform) {
			glEnable(GL_SCISSOR_TEST);
			glDisable(GL_STENCIL_TEST);
		}
#if 0
		else {
			glDisable(GL_SCISSOR_TEST);
			glEnable(GL_STENCIL_TEST);
		}
#endif
	}

	void RmlUiRenderInterface::SetScissorRegion(int x, int y, int width, int height)
	{
		if (!_has_transform) {
			glScissor(x, _viewport_height - (y + height), width, height);
			return;
		}

#if 0
		// clear the stencil buffer
		glStencilMask(GLuint(-1));
		glClear(GL_STENCIL_BUFFER_BIT);

		// fill the stencil buffer
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_FALSE);
		glStencilFunc(GL_NEVER, 1, GLuint(-1));
		glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);

		float fx = (float)x;
		float fy = (float)y;
		float fwidth = (float)width;
		float fheight = (float)height;

		// draw transformed quad
		GLfloat vertices[] = { fx, fy, 0, fx, fy + fheight, 0, fx + fwidth, fy + fheight, 0, fx + fwidth, fy, 0 };
		glDisableClientState(GL_COLOR_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, vertices);
		GLushort indices[] = { 1, 2, 0, 3 };
		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, indices);
		glEnableClientState(GL_COLOR_ARRAY);

		// prepare for drawing the real thing
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);
		glStencilMask(0);
		glStencilFunc(GL_EQUAL, 1, GLuint(-1));
#endif
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
		_has_transform = transform;
		if (!transform) {
			graphics::set_modelview_matrix_to_identity();
		} else if constexpr (std::is_same_v<Rml::Matrix4f, Rml::ColumnMajorMatrix4f>) {
			graphics::set_modelview_matrix(transform->data());
		} else if constexpr (std::is_same_v<Rml::Matrix4f, Rml::RowMajorMatrix4f>) {
			graphics::set_modelview_matrix(transform->Transpose().data());
		}
	}
}
