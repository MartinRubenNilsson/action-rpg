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

	// READING/WRITING

	bool read_text(const std::string& path, std::string& text);
	bool write_text(const std::string& path, const std::string& text);
	bool read_binary(const std::string& path, std::vector<unsigned char>& data);
	bool write_binary(const std::string& path, const std::vector<unsigned char>& data);

	// PATHS

	std::string get_normalized_path(const std::string& path);
	std::string get_parent_path(const std::string& path);
	std::string get_filename(const std::string& path);
	std::string get_stem(const std::string& path);
	std::string get_extension(const std::string& path);
	std::string replace_extension(const std::string& path, const std::string& new_extension);
}