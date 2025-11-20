#pragma once
#include "../GameObject.h"

class Entity : public GameObject
{
protected:
    GameObjectID m_dropItemID;  // 드롭 아이템의 ID
    int m_dropItemCount;

public:
    Entity(GameObjectType type, GameObjectID id, float x, float y, float pivotX, float pivotY, Direction _dir,
           const std::wstring& resourcePath = L"", const std::wstring& imageName = L"");
    virtual ~Entity();

    // 데미지 처리
    virtual void Damaged(int damage) = 0;

    // 드롭 아이템 관련 함수
    virtual GameObjectID GetDropItemID() const { return GOID_NONE; }
    virtual int GetDropItemCount() const { return 0; }
    virtual void SetDropItem(GameObjectID itemID, int count = 1) {}
    
    // 방향 관련 유틸리티 함수들
    Direction GetOppositeDirection(Direction dir);

    // 거리 계산 유틸리티 함수들
    float CalculateDistance(float x1, float y1, float x2, float y2);
    Direction GetDirectionToTarget(float fromX, float fromY, float toX, float toY);

    // 화면 범위 확인 함수
    bool IsPositionInScreenBounds(float x, float y);
};


