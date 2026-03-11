#pragma once
#include "../Tool.h"

class Axe : public Tool
{
public:
	Axe(GameObjectID id, const std::wstring& name, const std::wstring& desc, const std::wstring& baseDir, const std::wstring& imageName, int damage, float attackRange);
	virtual ~Axe();

};
