#pragma once
#include "../Entity.h"

class ResourceManager;

class Grass : public Entity
{
public:
    Grass(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath = L"", const std::wstring& imageName = L"");
    virtual ~Grass();

    virtual void Init() override;
    virtual void LateInit() override;
    virtual void Update(float deltaTime) override;
    virtual void LateUpdate() override;
    virtual void Release() override;

    virtual void Damaged(int damage) override;
    virtual bool OnInteraction(GameObject* obj) override;

private:
    GrassState m_state;
};

