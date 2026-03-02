#pragma once
#include "../Entity.h"

enum class TreeState {
	IDLE = 0,    // 일반 상태 (서있음)
	CHOP,        // 벌목중인 상태
	FALL,        // 넘어지는 중인 상태
	FALLEN,      // 넘어진 후의 상태
	COUNT
};

class Tree : public Entity
{
public:
    Tree(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& baseDir = L"", const std::wstring& imageName = L"");
    virtual ~Tree();

    virtual void Init() override;
    virtual void Update(float deltaTime) override;
    virtual void Release() override;

    virtual void Damaged(int damage) override;
    virtual void Die() override;

    TreeState GetTreeState() const { return m_treeState; }

private:
    TreeState m_treeState;
    float m_fallTimer;
	int m_hp;
};
