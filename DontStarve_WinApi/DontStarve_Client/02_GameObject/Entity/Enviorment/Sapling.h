#pragma once
#include "../Entity.h"

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

    virtual GameObjectID GetDropItemID() const override { return m_dropItemID; }
    virtual int GetDropItemCount() const override { return m_dropItemCount; }
    virtual void SetDropItem(GameObjectID itemID, int count = 1) override;

private:
    GameObjectID m_dropItemID;  
    int m_dropItemCount;   
    GrassState m_state;
};
