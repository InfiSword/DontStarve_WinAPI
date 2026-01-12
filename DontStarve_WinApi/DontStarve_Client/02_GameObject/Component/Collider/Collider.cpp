#include "99_Default/pch.h"
#include "Collider.h"
#include "../../../01_Manager/ColliderManager/ColliderManager.h"
#include "../../GameObject.h"

Collider::Collider(GameObject* owner) 
    : Component(owner),
    m_mapColliderDataValid(false), m_mapColliderOffsetX(0), m_mapColliderOffsetY(0),
    m_mapColliderWidth(0), m_mapColliderHeight(0)
{
}

void Collider::Init()
{
    Component::Init();
    ColliderManager::GetInstance()->AddCollider(this);
}

void Collider::Release()
{
    ColliderManager::GetInstance()->RemoveCollider(this);
    Component::Release();
}

void Collider::Update(float deltaTime)
{
    Component::Update(deltaTime);
}

void Collider::SetMapColliderData(bool hasCollider, int offsetX, int offsetY, int width, int height)
{
    m_mapColliderDataValid = hasCollider;
    if (hasCollider) {
        m_mapColliderOffsetX = offsetX;
        m_mapColliderOffsetY = offsetY;
        m_mapColliderWidth = width;
        m_mapColliderHeight = height;
    }
}

void Collider::GetMapColliderData(int& offsetX, int& offsetY, int& width, int& height) const
{
    offsetX = m_mapColliderOffsetX;
    offsetY = m_mapColliderOffsetY;
    width = m_mapColliderWidth;
    height = m_mapColliderHeight;
}

