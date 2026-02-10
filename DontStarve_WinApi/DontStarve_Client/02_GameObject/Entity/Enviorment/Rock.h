#pragma once
#include "../Entity.h"

class ResourceManager;

class Rock : public Entity
{
public:
	static void RegisterResources(ResourceManager* rm);

    Rock(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath = L"", const std::wstring& imageName = L"");
    virtual ~Rock();

    virtual void Init() override;
    virtual void LateInit() override;
    virtual void Update(float deltaTime) override;
    virtual void LateUpdate() override;
    virtual void Release() override;

    virtual void OnInteraction(GameObject* obj) override;

    virtual void Damaged(int damage) override;

private:
    int m_hp;
    int maxHp;
    float m_hitAnimTimer;
    RockState m_state;

private:
    Gdiplus::Bitmap* m_rockCracked;
    Gdiplus::Bitmap* m_rockBroken;
}; 
