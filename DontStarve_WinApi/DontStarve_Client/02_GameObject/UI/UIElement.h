#pragma once

#include "../GameObject.h"

class RectTransform;

class UIElement : public GameObject
{
protected:
	RectTransform* m_rectTransform;  // 자동 생성 및 관리

public:
	UIElement(GameObjectID id, 
		const std::wstring& resourcePath = L"", const std::wstring& imageName = L"", 
		ColliderType colliderType = NONE, bool isActive = true, bool isInteractive = false);
	virtual ~UIElement();

	virtual void Update(float deltaTime) override;
	virtual void Render() override {}
	virtual void Release() override;

	RectTransform* GetRectTransform() const { return m_rectTransform; }
};
