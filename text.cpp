#include "stdafx.h"
#include "text.h"
#include "fonts.h"
#include "graphics.h"

namespace text
{
	void render(const Text& text)
	{
#if 0
        if (text.string.empty()) return;

#if 0
        // Compute values related to the text style
        const bool  isBold = m_style & Bold;
        const bool  isUnderlined = m_style & Underlined;
        const bool  isStrikeThrough = m_style & StrikeThrough;
        const float italicShear = (m_style & Italic) ? degrees(12).asRadians() : 0.f;
        const float underlineOffset = m_font->getUnderlinePosition(text.character_size);
        const float underlineThickness = m_font->getUnderlineThickness(text.character_size);
#else
        const bool isBold = false;
        const bool isUnderlined = false;
        const bool isStrikeThrough = false;
        const float italicShear = 0.f
#endif

        // Compute the location of the strike through dynamically
        // We use the center point of the lowercase 'x' glyph as the reference
        // We reuse the underline thickness as the thickness of the strike through as well
        const float strikeThroughOffset = m_font->getGlyph(U'x', text.character_size, isBold).bounds.getCenter().y;

        // Precompute the variables needed by the algorithm
        float       whitespaceWidth = m_font->getGlyph(U' ', text.character_size, isBold).advance;
        const float letterSpacing = (whitespaceWidth / 3.f) * (text.letter_spacing_factor - 1.f);
        whitespaceWidth += letterSpacing;
        const float lineSpacing = m_font->getLineSpacing(text.character_size) * text.line_spacing_factor;
        float       x = 0.f;
        auto        y = static_cast<float>(text.character_size);

        // Create one quad for each character
        std::vector<graphics::Vertex> vertices;
        uint32_t prevChar = 0;
        float minX = (float)text.character_size;
        float minY = (float)text.character_size;
        float maxX = 0.f;
        float maxY = 0.f;

        for (const uint32_t curChar : text.string) {
            // Skip the \r char to avoid weird graphical issues
            if (curChar == U'\r')
                continue;

            // Apply the kerning offset
            x += m_font->getKerning(prevChar, curChar, text.character_size, isBold);

#if 0
            // If we're using the underlined style and there's a new line, draw a line
            if (isUnderlined && (curChar == U'\n' && prevChar != U'\n')) {
                addLine(m_vertices, x, y, m_fillColor, underlineOffset, underlineThickness);

                if (m_outlineThickness != 0)
                    addLine(m_outlineVertices, x, y, m_outlineColor, underlineOffset, underlineThickness, m_outlineThickness);
            }

            // If we're using the strike through style and there's a new line, draw a line across all characters
            if (isStrikeThrough && (curChar == U'\n' && prevChar != U'\n')) {
                addLine(m_vertices, x, y, m_fillColor, strikeThroughOffset, underlineThickness);

                if (m_outlineThickness != 0)
                    addLine(m_outlineVertices, x, y, m_outlineColor, strikeThroughOffset, underlineThickness, m_outlineThickness);
            }
#endif

            prevChar = curChar;

            // Handle special characters
            if ((curChar == U' ') || (curChar == U'\n') || (curChar == U'\t')) {
                // Update the current bounds (min coordinates)
                minX = std::min(minX, x);
                minY = std::min(minY, y);

                switch (curChar) {
                case U' ':
                    x += whitespaceWidth;
                    break;
                case U'\t':
                    x += whitespaceWidth * 4;
                    break;
                case U'\n':
                    y += lineSpacing;
                    x = 0;
                    break;
                }

                // Update the current bounds (max coordinates)
                maxX = std::max(maxX, x);
                maxY = std::max(maxY, y);

                // Next glyph, no need to create a quad for whitespace
                continue;
            }

            // Apply the outline
            if (m_outlineThickness != 0) {
                const Glyph& glyph = m_font->getGlyph(curChar, text.character_size, isBold, m_outlineThickness);

                // Add the outline glyph to the vertices
                addGlyphQuad(m_outlineVertices, Vector2f(x, y), m_outlineColor, glyph, italicShear);
            }

            // Extract the current glyph's description
            const Glyph& glyph = m_font->getGlyph(curChar, text.character_size, isBold);

            // Add the glyph to the vertices
            addGlyphQuad(m_vertices, Vector2f(x, y), m_fillColor, glyph, italicShear);

            // Update the current bounds
            const Vector2f p1 = glyph.bounds.position;
            const Vector2f p2 = glyph.bounds.position + glyph.bounds.size;

            minX = std::min(minX, x + p1.x - italicShear * p2.y);
            maxX = std::max(maxX, x + p2.x - italicShear * p1.y);
            minY = std::min(minY, y + p1.y);
            maxY = std::max(maxY, y + p2.y);

            // Advance to the next character
            x += glyph.advance + letterSpacing;
        }

#if 0
        // If we're using outline, update the current bounds
        if (m_outlineThickness != 0) {
            const float outline = std::abs(std::ceil(m_outlineThickness));
            minX -= outline;
            maxX += outline;
            minY -= outline;
            maxY += outline;
        }

        // If we're using the underlined style, add the last line
        if (isUnderlined && (x > 0)) {
            addLine(m_vertices, x, y, m_fillColor, underlineOffset, underlineThickness);

            if (m_outlineThickness != 0)
                addLine(m_outlineVertices, x, y, m_outlineColor, underlineOffset, underlineThickness, m_outlineThickness);
        }

        // If we're using the strike through style, add the last line across all characters
        if (isStrikeThrough && (x > 0)) {
            addLine(m_vertices, x, y, m_fillColor, strikeThroughOffset, underlineThickness);

            if (m_outlineThickness != 0)
                addLine(m_outlineVertices, x, y, m_outlineColor, strikeThroughOffset, underlineThickness, m_outlineThickness);
        }
#endif
#endif
	}
}