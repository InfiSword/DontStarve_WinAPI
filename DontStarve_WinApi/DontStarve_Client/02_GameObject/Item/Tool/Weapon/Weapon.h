#pragma once
#include "../Tool.h"

class Weapon : public Tool
{
public:
	Weapon(UINT id, const std::wstring& name, const std::wstring& desc, const std::wstring& baseDir, const std::wstring& imageName);
	virtual ~Weapon();
	virtual void Use(float durabilityCost) override;
};
