#include "99_Default/pch.h"
#include "Transform.h"
#include "../Sprite/SpriteRenderer.h"
#include "../../GameObject.h"

Transform::Transform(GameObject* owner, float x, float y, float scaleX, float scaleY, float rotation, Direction dir)
	: Component(owner), m_x(x), m_y(y), m_scaleX(scaleX), m_scaleY(scaleY), m_rotation(rotation), m_direction(dir)
{
}

Transform::~Transform()
{
}

void Transform::SetPosition(float x, float y)
{
	m_x = x;
	m_y = y;
	if (m_owner) m_owner->SetSpatialDirty();
}

void Transform::SetScale(float scaleX, float scaleY)
{
	m_scaleX = scaleX;
	m_scaleY = scaleY;
	if (m_owner) m_owner->SetSpatialDirty();
}

void Transform::SetScale(float scale)
{
	m_scaleX = scale;
	m_scaleY = scale;
	if (m_owner) m_owner->SetSpatialDirty();
}

void Transform::SetRotation(float rotation)
{
	m_rotation = rotation;
	if (m_owner) m_owner->SetSpatialDirty();
}

void Transform::SetDirection(Direction dir)
{
	m_direction = dir;
}