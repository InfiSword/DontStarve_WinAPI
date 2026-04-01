#pragma once

#include "../GameObject.h"

class RectTransform;
class Text;
namespace ComponentElement { class Image; }

class UIElement : public GameObject
{
protected:
	RectTransform* m_rectTransform;  // 자동 생성 및 관리

public:
	UIElement(GameObjectID id, 
		const std::wstring& resourcePath = L"", const std::wstring& imageName = L"", 
		bool isActive = true, bool isInteractive = false);
	virtual ~UIElement();

	virtual void Update(float deltaTime) override;
	virtual void Render() override {}
	virtual void Release() override;

	// UI 여부 반환 (dynamic_cast 대체용)
	virtual bool IsUI() const override { return true; }

	// RectTransform 접근
	RectTransform* GetRectTransform() const { return m_rectTransform; }
};
