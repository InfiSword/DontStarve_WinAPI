#pragma once
#include "../Tool.h"

class Pickaxe : public Tool
{
public:
	Pickaxe(UINT id, const std::wstring& name, const std::wstring& desc, const std::wstring& baseDir, const std::wstring& imageName);
	virtual ~Pickaxe();
	virtual void Use(float durabilityCost) override;
};
