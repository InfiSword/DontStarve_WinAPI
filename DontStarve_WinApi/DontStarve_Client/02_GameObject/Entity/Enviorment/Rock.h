#pragma once
#include "../Entity.h"

class ResourceManager;
class Sprite;

class Rock : public Entity
{
public:
    Rock(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath = L"", const std::wstring& imageName = L"");
    virtual ~Rock();

    virtual void Init() override;
    virtual void LateInit() override;
    virtual void Update(float deltaTime) override;
    virtual void LateUpdate() override;
    virtual void Release() override;

    virtual bool OnInteraction(GameObject* obj) override;

    virtual void Damaged(int damage) override;
    virtual void Die() override;

private:
    int m_hp;
    int maxHp;
    RockState m_state;

    std::shared_ptr<Sprite> m_spriteIntact;
    std::shared_ptr<Sprite> m_spriteCracked;
    std::shared_ptr<Sprite> m_spriteBroken;
}; 
