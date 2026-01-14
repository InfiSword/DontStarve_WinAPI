#pragma once
#include "../GameObject.h"

class Animator;
class Transform;
class SpriteRenderer;
class Collider;

class Entity : public GameObject
{
protected:
    GameObjectID m_dropItemID;  // 드롭 아이템 ID
    int m_dropItemCount;

    Animator* m_animator;       // 애니메이션을 위한 Animator 컴포넌트
    Transform* transform;        // Transform 컴포넌트 캐시
    SpriteRenderer* spriteRenderer;  // SpriteRenderer 컴포넌트 캐시

public:
    Entity(GameObjectType type, GameObjectID id, float x, float y, float pivotX, float pivotY, Direction _dir,
           const std::wstring& imageName = L"",
           bool isActive = true, bool isInteractive = false);
    virtual ~Entity();

    // 초기화 - Animator 설정
    virtual void Init() override;

    // 데미지 처리
    virtual void Damaged(int damage) = 0;

    // 드롭 아이템 관련 가상 함수들
    virtual GameObjectID GetDropItemID() const;
    virtual int GetDropItemCount() const;
    virtual void SetDropItem(GameObjectID itemID, int count = 1);   

    // Animator 접근자
    Animator* GetAnimator() const { return m_animator; }
};
