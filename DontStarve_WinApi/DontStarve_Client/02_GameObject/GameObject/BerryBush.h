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

    // Entity 인터페이스 구현 (더미 구현)
    virtual void RegisterAllAnimations() override {}
    virtual void UpdateAnimatorState() override {}

    void OnPlayerInteraction(class Player* player);
    
    // 아이템 제공 관련 메서드 (Entity의 가상 함수 재정의)
    virtual GameObjectID GetDropItemID() const override { return m_dropItemID; }
    virtual int GetDropItemCount() const override { return m_dropItemCount; }
    virtual void SetDropItem(GameObjectID itemID, int count = 1) override;

private:
    GameObjectID m_dropItemID;  // 제공할 아이템 ID
    int m_dropItemCount;        // 제공할 아이템 개수
};

