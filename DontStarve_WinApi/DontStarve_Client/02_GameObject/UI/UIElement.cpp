#include "99_Default/pch.h"
#include "UIElement.h"
#include "../Component/Transform/RectTransform.h"
#include "../Component/Sprite/SpriteRenderer.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"

UIElement::UIElement(GameObjectID id, const std::wstring& resourcePath, const std::wstring& imageName, ColliderType colliderType, bool isActive, bool isInteractive)
	: GameObject(id, 0,0,0,0, DIR_NONE, resourcePath, imageName, colliderType, isActive, isInteractive)
{
	m_type = GO_TYPE_UI;
	m_rectTransform = AddComponent<RectTransform>();
}

UIElement::~UIElement()
{
}

void UIElement::Update(float deltaTime)
{
	GameObject::Update(deltaTime);
}

void UIElement::Release()
{
	GameObject::Release();
}
