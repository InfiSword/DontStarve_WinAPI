#include "99_Default/pch.h"
#include "Torch.h"

Torch::Torch(GameObjectID id, const std::wstring& name, const std::wstring& desc, const std::wstring& baseDir, const std::wstring& imageName)
	: Tool(id, name, desc, baseDir, imageName)
{
}

Torch::~Torch()
{
}
