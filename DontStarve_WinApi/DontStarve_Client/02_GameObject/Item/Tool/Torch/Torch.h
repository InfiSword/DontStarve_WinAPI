#pragma once
#include "../Tool.h"

class Torch : public Tool
{
public:
	Torch(UINT id, const std::wstring& name, const std::wstring& desc, const std::wstring& baseDir, const std::wstring& imageName);
	virtual ~Torch();
	virtual void Use(float durabilityCost) override;
};
