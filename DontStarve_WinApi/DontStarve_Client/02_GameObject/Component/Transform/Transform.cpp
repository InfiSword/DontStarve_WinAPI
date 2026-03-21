#include "99_Default/pch.h"
#include "Transform.h"

Transform::Transform(GameObject* owner, float x, float y, float scaleX, float scaleY, float pivotX, float pivotY, float rotation, Direction dir)
	: Component(owner), m_x(x), m_y(y), m_scaleX(scaleX), m_scaleY(scaleY), m_pivotX(pivotX), m_pivotY(pivotY), m_rotation(rotation), m_direction(dir)
{
}

Transform::~Transform()
{
}

float Transform::GetSortKey(RenderLayer layer) const
{
	return static_cast<float>(layer) + m_y;
}
