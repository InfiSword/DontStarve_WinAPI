#pragma once
#include "../Entity.h" 

class BerryBush : public Entity<GrassState>
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

    virtual void RegisterAllAnimations() override {}
    virtual void UpdateAnimatorState() override {}

    virtual GameObjectID GetDropItemID() const override { return m_dropItemID; }
    virtual int GetDropItemCount() const override { return m_dropItemCount; }
    virtual void SetDropItem(GameObjectID itemID, int count = 1) override;

private:
    GameObjectID m_dropItemID;  // 드롭 아이템의 ID
    int m_dropItemCount;        // 드롭 아이템의 개수
};

