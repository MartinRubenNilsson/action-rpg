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
		VertexShader, // .vert
		FragmentShader, // .frag
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

	// PATHS

	std::string get_normalized_path(const std::string& path);
	std::string get_parent_path(const std::string& path);
	std::string get_filename(const std::string& path);
	std::string get_stem(const std::string& path);
	std::string get_extension(const std::string& path);
	std::string replace_extension(const std::string& path, const std::string& new_extension);
}