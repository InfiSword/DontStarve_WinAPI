#include "99_Default/pch.h"
#include "UIElement.h"
#include "../Component/Transform/RectTransform.h"
#include "../Component/Sprite/SpriteRenderer.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"

UIElement::UIElement(GameObjectID id, const std::wstring& resourcePath, const std::wstring& imageName, bool isActive, bool isInteractive)
	: GameObject(id, resourcePath, imageName, isActive, isInteractive)
{
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
