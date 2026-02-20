#pragma once
#include "../Tool.h"

/** 도끼 종류별 능력치. 생성자에서 ID로 조회 후 Tool(damage, durability, effectiveness) 초기화에 사용 */
struct AxeStats {
	int damage;
	float durability;
	float effectiveness;
};

class Axe : public Tool
{
public:
	Axe(UINT id, const std::wstring& name, const std::wstring& desc, const std::wstring& baseDir, const std::wstring& imageName);
	virtual ~Axe();
	virtual void Use(float durabilityCost) override;
};
