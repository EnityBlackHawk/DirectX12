#pragma once

#include <memory>
#include <string_view>
#include <filesystem>
#include <fstream>
#include "../basic_include.h"

class Shader
{

public:

	Shader(std::string_view name);

	inline const void* data() const { return _data.get(); }
	inline int size() const { return _size; }

private:

	std::unique_ptr<void> _data = nullptr;
	int _size = 0;

};

