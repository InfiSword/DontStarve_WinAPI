#pragma once
#include "Entity.h"

class Tree : public Entity<TreeState> {
public:
    Tree(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath = L"", const std::wstring& imageName = L"");
    virtual ~Tree();

public:
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

    virtual void Damaged(int damage) override;
    
    void OnPlayerInteraction(class Player* player);
    
private:
    std::wstring tree_Grade;
    int m_hp;
    int maxHp;
    float m_hitAnimTimer;
    
}; 