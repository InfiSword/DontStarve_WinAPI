#pragma once
#include "../Entity.h"

class ResourceManager;

class BerryBush : public Entity
{
public:
    BerryBush(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath = L"", const std::wstring& imageName = L"");
    virtual ~BerryBush();

    virtual void Init() override;
    virtual void LateInit() override;
    virtual void Update(float deltaTime) override;
    virtual void LateUpdate() override;
    virtual void Release() override;

    virtual void OnInteraction(GameObject* obj) override;

    virtual GameObjectID GetDropItemID() const { return m_dropItemID; }
    virtual int GetDropItemCount() const { return m_dropItemCount; }
    virtual void SetDropItem(GameObjectID itemID, int count = 1);

    virtual void Damaged(int damage) override;

protected:
    GrassState m_state;
};

