#pragma once
#include "../GameObject.h"

// Entity는 상호작용 가능한 오브젝트의 기본 클래스
// StateEnum: 각 Entity 타입에서 사용되는 상태 enum 타입
template<typename StateEnum>
class Entity : public GameObject
{
protected:
    StateEnum m_state;
    bool m_acquired;  // 획득/수집 가능 여부 플래그

public:
    Entity(GameObjectType type, GameObjectID id, float x, float y, float pivotX, float pivotY, Direction _dir,
           const std::wstring& resourcePath = L"", const std::wstring& imageName = L"");
    virtual ~Entity();

    // 기본 상태 관리
    virtual void SetState(StateEnum state);
    virtual StateEnum GetState() const;

    // 획득 상태 관리
    virtual void SetAcquired(bool acquired);
    virtual bool IsAcquired() const;

    // 데미지 처리 (필요한 Entity에서 오버라이드)
    virtual void Damaged(int damage) {}

    // 드롭 아이템 관련 함수 (아이템 반환 게임오브젝트에서 오버라이드)
    virtual GameObjectID GetDropItemID() const { return GOID_NONE; }
    virtual int GetDropItemCount() const { return 0; }
    virtual void SetDropItem(GameObjectID itemID, int count = 1) {}

    // Unity Animator 스타일 애니메이션 관리 - Enum과 Direction을 키로 하는 애니메이션 시스템
    virtual void RegisterAllAnimations() = 0;  // 모든 애니메이션을 등록
    virtual void UpdateAnimatorState() = 0;    // 상태(Enum)와 방향(Direction)에 따라 적절한 애니메이션 설정
};

// 템플릿 구현
template<typename StateEnum>
Entity<StateEnum>::Entity(GameObjectType type, GameObjectID id, float x, float y, float pivotX, float pivotY, Direction _dir,
                          const std::wstring& resourcePath, const std::wstring& imageName)
    : GameObject(type, id, x, y, pivotX, pivotY, _dir, resourcePath, imageName), m_acquired(false)
{
    // 상태 초기화는 각 하위 클래스에서 원하는 기본값으로 설정
}

template<typename StateEnum>
Entity<StateEnum>::~Entity()
{
}

template<typename StateEnum>
void Entity<StateEnum>::SetState(StateEnum state)
{
    m_state = state;
}

template<typename StateEnum>
StateEnum Entity<StateEnum>::GetState() const
{
    return m_state;
}

template<typename StateEnum>
void Entity<StateEnum>::SetAcquired(bool acquired)
{
    m_acquired = acquired;
}

template<typename StateEnum>
bool Entity<StateEnum>::IsAcquired() const
{
    return m_acquired;
}

// ========================================
// Entity 관련 유틸리티 함수 선언
// ========================================

// 방향 관련 유틸리티 함수들
Direction GetOppositeDirection(Direction dir);

// 거리 계산 유틸리티 함수들
float CalculateDistance(float x1, float y1, float x2, float y2);
Direction GetDirectionToTarget(float fromX, float fromY, float toX, float toY);

// 화면 범위 확인 함수
bool IsPositionInScreenBounds(float x, float y);

