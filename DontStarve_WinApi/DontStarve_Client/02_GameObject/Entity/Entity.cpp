#include "../../99_Default/pch.h"
#include "Entity.h"
#include "../../01_Manager/CameraManager/CameraManager.h"

Entity::Entity(GameObjectType type, GameObjectID id, float x, float y, float pivotX, float pivotY, Direction _dir,
	const std::wstring& resourcePath, const std::wstring& imageName, bool isActive, bool isInteractive)
    :GameObject(type, id, x, y, pivotX, pivotY, _dir, resourcePath, imageName, isActive, isInteractive)
{

}

Entity::~Entity()
{
}

// 방향 관련 유틸리티 함수들
Direction Entity::GetOppositeDirection(Direction dir)
{
    switch (dir)
    {
    case DIR_UP:    return DIR_DOWN;
    case DIR_DOWN:  return DIR_UP;
    case DIR_LEFT:  return DIR_RIGHT;
    case DIR_RIGHT: return DIR_LEFT;
    default:        return DIR_NONE;
    }
}

// 거리 계산 유틸리티 함수들
float Entity::CalculateDistance(float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}

Direction Entity::GetDirectionToTarget(float fromX, float fromY, float toX, float toY)
{
    float dx = toX - fromX;
    float dy = toY - fromY;
    
    // 절댓값이 더 큰 방향을 우선적으로 선택
    if (abs(dx) > abs(dy))
    {
        return (dx > 0) ? DIR_RIGHT : DIR_LEFT;
    }
    else
    {
        return (dy > 0) ? DIR_DOWN : DIR_UP;
    }
}

// 화면 범위 확인 함수
bool Entity::IsPositionInScreenBounds(float x, float y)
{
    // 화면 좌표로 변환하여 확인
    Gdiplus::PointF screenPos = CameraManager::GetInstance()->WorldToScreen(x, y);
    
    return (screenPos.X >= 0 && screenPos.X <= WINCX && 
            screenPos.Y >= 0 && screenPos.Y <= WINCY);
}
