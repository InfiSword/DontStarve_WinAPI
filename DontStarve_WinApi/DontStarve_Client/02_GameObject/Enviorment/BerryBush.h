#pragma once
#include "Entity.h"

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

    // Entity 인터페이스 구현 (필수 오버라이드)
    virtual void RegisterAllAnimations() override {}
    virtual void UpdateAnimatorState() override {}

    void OnPlayerInteraction(class Player* player);
    
    // 드롭 아이템 관련 함수들 (Entity의 추상 함수 구현)
    virtual GameObjectID GetDropItemID() const override { return m_dropItemID; }
    virtual int GetDropItemCount() const override { return m_dropItemCount; }
    virtual void SetDropItem(GameObjectID itemID, int count = 1) override;

private:
    GameObjectID m_dropItemID;  // 드롭 아이템의 ID
    int m_dropItemCount;        // 드롭 아이템의 개수
};

