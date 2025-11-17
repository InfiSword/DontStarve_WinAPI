#pragma once
#include "GameObject.h"

// Entity는 생명체나 상호작용 가능한 객체의 기본 클래스
// StateEnum: 각 Entity 타입별로 정의된 상태 enum 타입
template<typename StateEnum>
class Entity : public GameObject
{
protected:
    StateEnum m_state;
    bool m_acquired;  // 획득/소유 상태 관리

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

    // 데미지 처리 (필요한 Entity만 오버라이드)
    virtual void Damaged(int damage) {}

    // 아이템 드롭 관련 (자연 환경 오브젝트만 오버라이드)
    virtual GameObjectID GetDropItemID() const { return GOID_NONE; }
    virtual int GetDropItemCount() const { return 0; }
    virtual void SetDropItem(GameObjectID itemID, int count = 1) {}

    // Unity Animator 스타일 애니메이션 관리 - Enum값과 Direction을 키로 자동 업데이트
    virtual void RegisterAllAnimations() = 0;  // 모든 애니메이션을 등록
    virtual void UpdateAnimatorState() = 0;    // 상태(Enum)와 방향(Direction)으로 자동 애니메이션 선택

    // GameObject의 애니메이션 메소드들 오버라이드
    virtual void UpdateAnimation(float deltaTime) override {}
};

// 템플릿 구현부
template<typename StateEnum>
Entity<StateEnum>::Entity(GameObjectType type, GameObjectID id, float x, float y, float pivotX, float pivotY, Direction _dir,
                          const std::wstring& resourcePath, const std::wstring& imageName)
    : GameObject(type, id, x, y, pivotX, pivotY, _dir, resourcePath, imageName), m_acquired(false)
{
    // 상태 초기화는 각 파생 클래스에서 적절한 기본값으로 설정
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
// Entity 관련 유틸리티 함수 선언부
// ========================================

// 방향 관련 유틸리티 함수들
Direction GetOppositeDirection(Direction dir);

// 거리 계산 유틸리티 함수들
float CalculateDistance(float x1, float y1, float x2, float y2);
Direction GetDirectionToTarget(float fromX, float fromY, float toX, float toY);

// 화면 경계 체크 함수
bool IsPositionInScreenBounds(float x, float y);

