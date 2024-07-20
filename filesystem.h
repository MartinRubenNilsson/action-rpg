#pragma once

namespace filesystem
{
	//These are not used for anything rn
	enum class FileFormat
	{
		Unknown,
		Text, // .txt
		PNG, // .png
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

	// PATHS

	std::string get_normalized_path(const std::string& path);
	std::string get_parent_path(const std::string& path);
	std::string get_filename(const std::string& path);
	std::string get_stem(const std::string& path);
	std::string get_extension(const std::string& path);
}