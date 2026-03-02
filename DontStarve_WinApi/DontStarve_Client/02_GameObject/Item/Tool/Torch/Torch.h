#pragma once
#include "../Tool.h"

class Torch : public Tool
{
public:
	Torch(GameObjectID id, const std::wstring& name, const std::wstring& desc, const std::wstring& baseDir, const std::wstring& imageName);
	virtual ~Torch();

};
