#include "99_Default/pch.h"
#include "UIElement.h"
#include "../Component/Transform/RectTransform.h"

UIElement::UIElement(GameObjectType type, GameObjectID id, 
	const std::wstring& resourcePath, const std::wstring& imageName, 
	bool isActive, bool isInteractive)
	: GameObject(type, id, resourcePath, imageName, isActive, isInteractive),
	m_rectTransform(nullptr)
{
	m_rectTransform = AddComponent<RectTransform>();
}

UIElement::~UIElement()
{
	// 소멸자 호출 시 Release()를 통해 정리 작업 수행
	Release();
}

void UIElement::Release()
{
	// UIElement 전용 정리 작업
	m_rectTransform = nullptr;
	
	// 부모 클래스의 Release() 호출하여 컴포넌트 정리
	GameObject::Release();
}
