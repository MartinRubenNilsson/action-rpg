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
		DynamicLinkLibrary, // .dll
		Executable, // .exe
	};

	void initialize();

	//std::span<const std::string> get_paths_in_directory(const std::string& directory);

	std::string get_normalized_path(const std::string& path);
	std::string get_parent_path(const std::string& path);
	std::string get_filename(const std::string& path);
	std::string get_stem(const std::string& path);
	std::string get_extension(const std::string& path);
}