#pragma once
#include "Entity.h"

class Grass : public Entity<GrassState>
{
public:
    Grass(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath = L"", const std::wstring& imageName = L"");
    virtual ~Grass();

    virtual void Init() override;
    virtual void LateInit() override;
    virtual void Update(float deltaTime) override;
    virtual void LateUpdate() override;
    virtual void Render(Gdiplus::Graphics* pGraphics) override; // RenderManager에서 처리
    virtual void Release() override;

    // Unity Animator 스타일로 최적화된 애니메이션 메소드들
    virtual void UpdateAnimation(float deltaTime) override;
    virtual Gdiplus::Bitmap* GetBitmap() const override;

    virtual void OnInteraction(GameObject* obj) override;

    // Entity 인터페이스 구현 (Unity Animator 스타일 - Enum + Direction 키값)
    virtual void RegisterAllAnimations() override;
    virtual void UpdateAnimatorState() override;

    void OnPlayerInteraction(class Player* player);
    
    // 아이템 제공 관련 메서드 (Entity의 가상 함수 재정의)
    virtual GameObjectID GetDropItemID() const override { return m_dropItemID; }
    virtual int GetDropItemCount() const override { return m_dropItemCount; }
    virtual void SetDropItem(GameObjectID itemID, int count = 1) override;

private:
    GameObjectID m_dropItemID;  // 제공할 아이템 ID
    int m_dropItemCount;        // 제공할 아이템 개수
};

