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
    bool m_isDead;               // 죽음 판정

    int m_hp;                    // 현재 체력
    int m_maxHp;                 // 최대 체력
    int m_state;                 // 엔티티 상태 (각 클래스별 Enum 캐스팅용)

    Animator* m_animator;            // 애니메이션을 위한 Animator 컴포넌트
    Transform* transform;            // Transform 컴포넌트 캐시
    SpriteRenderer* spriteRenderer;  // SpriteRenderer 컴포넌트 캐시

public:
    Entity(GameObjectID id, float x, float y, float pivotX, float pivotY, Direction _dir,
           const std::wstring& baseDir = L"",
           const std::wstring& imageName = L"",
           bool isActive = true, bool isInteractive = false);
    virtual ~Entity();

    // 초기화
    virtual void Init() override;
    virtual void Update(float deltaTime) override;
    virtual void Release() override;

    // 데미지 처리
    virtual void Damaged(int damage) = 0;

    // 죽음 처리
    virtual void Die() {}

    // 상태 관련 Getter
    bool IsDead() const { return m_isDead; }
    int GetHp() const { return m_hp; }
    int GetMaxHp() const { return m_maxHp; }
    int GetState() const { return m_state; }

    // 드롭 아이템 관련 가상 함수들
    virtual GameObjectID GetDropItemID() const;
    virtual int GetDropItemCount() const;
    virtual void SetDropItem(GameObjectID itemID, int count = 1);   
	virtual bool OnInteraction(GameObject* obj);

    // Animator 접근자
    Animator* GetAnimator() const { return m_animator; }
};
