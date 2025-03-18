#pragma once

namespace filesystem
{
	enum class FileFormat
	{
		Unknown,
		Text, // .txt
		PngImage, // .png
		KhronosTexture, // .ktx2
		TiledMap, // .tmx
		TiledTileset, // .tsx
		TiledTemplate, // .tx
		TrueTypeFont, // .ttf
		GLSLVertexShader, // .vert
		GLSLFragmentShader, // .frag
		RmlUiDocument, // .rml
		RmlUiStyleSheet, // .rcss
		FmodStudioBank, // .bank
	};

	struct File
	{
		std::string path;
		FileFormat format = FileFormat::Unknown;
	};

	void initialize();

	// FILES

	size_t get_file_count();
	std::span<const File> get_files();
	std::span<const File> get_files_in_directory(const std::string& directory_path); // recursive
	bool file_exists(const std::string& path);

	// READING/WRITING FILES

	bool read_text_file(const std::string& path, std::string& text);
	bool write_text_file(const std::string& path, std::string_view text);
	bool read_binary_file(const std::string& path, std::vector<unsigned char>& data);
	bool write_binary_file(const std::string& path, std::span<const unsigned char> data);

	// PATHS

	std::string get_normalized_path(const std::string& path);
	std::string get_parent_path(const std::string& path);
	std::string get_filename(const std::string& path);
	std::string get_stem(const std::string& path);
	std::string get_extension(const std::string& path);
	std::string replace_extension(const std::string& path, const std::string& new_extension);
}