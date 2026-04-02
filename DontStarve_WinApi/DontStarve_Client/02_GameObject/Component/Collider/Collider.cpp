#include "99_Default/pch.h"
#include "Collider.h"
#include "../../../01_Manager/ColliderManager/ColliderManager.h"
#include "../../GameObject.h"

Collider::Collider(GameObject* owner) 
    : Component(owner)
{
    m_isInteractionCollider = true;
    m_isPhysicalCollider = true;
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

