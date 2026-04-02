#pragma once

#include "../Component.h"

// 월드 오브젝트의 위치, 스케일, 피벗, 방향을 관리하는 컴포넌트
class Transform : public Component
{
protected:
	float m_x, m_y;				// 월드 좌표
	float m_scaleX, m_scaleY;	// 스케일 (기본값 1.0f)
	float m_rotation;           // 회전 (도 단위)
	Direction m_direction;		// 방향

public:
	Transform(GameObject* owner, float x = 0.0f, float y = 0.0f,
		float scaleX = 1.0f, float scaleY = 1.0f,
		float rotation = 0.0f,
		Direction dir = DIR_DOWN);
	virtual ~Transform();

	// 위치 Getter/Setter
	float GetX() const { return m_x; }
	float GetY() const { return m_y; }
	void SetPosition(float x, float y);

	// 스케일 Getter/Setter
	float GetScaleX() const { return m_scaleX; }
	float GetScaleY() const { return m_scaleY; }
	void SetScale(float scaleX, float scaleY);
	void SetScale(float scale);

	// 회전 Getter/Setter
	float GetRotation() const { return m_rotation; }
	void SetRotation(float rotation);

	// 방향 Getter/Setter
	Direction GetDirection() const { return m_direction; }
	void SetDirection(Direction dir);

	static Direction GetOppositeDirection(Direction dir)
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
};
