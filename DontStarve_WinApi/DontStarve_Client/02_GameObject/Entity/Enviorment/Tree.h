#pragma once
#include "../Entity.h"

class ResourceManager;

class Tree : public Entity {
public:
    Tree(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath = L"", const std::wstring& imageName = L"");
    virtual ~Tree();

public:
    virtual void Init() override;
    virtual void LateInit() override;
    virtual void Update(float deltaTime) override;
    virtual void LateUpdate() override;
    virtual void Release() override;

    virtual bool OnInteraction(GameObject* obj) override;

    virtual void Damaged(int damage) override;
    virtual void Die() override;

protected:
    int m_hp;
    int maxHp;
    float m_baseX, m_baseY;
    float m_shakeDuration;
    float m_shakeAmount;
    float m_shakeSpeed;
    bool m_isShaking;
};
