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