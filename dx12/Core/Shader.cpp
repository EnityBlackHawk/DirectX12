#include "Shader.h"

Shader::Shader(std::string_view name)
{
	wchar_t path[MAX_PATH];
	GetModuleFileNameW(nullptr, path, MAX_PATH);

	std::filesystem::path shader_dir(path);
	shader_dir.remove_filename();

	std::ifstream file(shader_dir / name, std::ios::binary);
	
	if (file.is_open())
	{
		file.seekg(0, std::ios::end);
		_size = file.tellg();
		file.seekg(0, std::ios::beg);

		_data = std::make_unique<void>(_size);
		file.read((char*)_data.get(), _size);
	}
	

}
