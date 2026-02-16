#pragma once
#include "../Entity.h"

class ResourceManager;

class Sapling : public Entity
{
public:
    Sapling(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath = L"", const std::wstring& imageName = L"");
    virtual ~Sapling();

    virtual void Init() override;
    virtual void LateInit() override;
    virtual void Update(float deltaTime) override;
    virtual void LateUpdate() override;
    virtual void Release() override;

    virtual void OnInteraction(GameObject* obj) override;
    virtual void Damaged(int damage) override;

    virtual GameObjectID GetDropItemID() const { return m_dropItemID; }
    virtual int GetDropItemCount() const { return m_dropItemCount; }
    virtual void SetDropItem(GameObjectID itemID, int count = 1);

private:
    GrassState m_state;
};
