#include "99_Default/pch.h"
#include "UIImage.h"
#include "../Component/Transform/RectTransform.h"
#include "../Component/Sprite/Image.h"
#include "../../01_Manager/RenderManager/RenderManager.h"

UIImage::UIImage(GameObjectID id, float width, float height, RenderLayer layer, const std::wstring& imagePath, float sortKey,
                 float anchorMinX, float anchorMinY, float anchorMaxX, float anchorMaxY,
                 float anchoredPosX, float anchoredPosY)
	: UIElement(id, L"", L"", true, false)
{
	m_rectTransform = GetRectTransform();
	m_rectTransform->SetAnchorMin(anchorMinX, anchorMinY);
	m_rectTransform->SetAnchorMax(anchorMaxX, anchorMaxY);
	m_rectTransform->SetAnchoredPosition(anchoredPosX, anchoredPosY);

	m_image = AddComponent<ComponentElement::Image>();
	m_image->SetLayer(layer);
	m_image->SetSortKey(sortKey);
	m_image->LoadSprite(imagePath);
	m_image->SetPivot(0.5f, 0.5f);

	Gdiplus::Bitmap* bitmap = m_image->GetSprite();
	if (bitmap) 
	{
		float bw = static_cast<float>(bitmap->GetWidth());
		float bh = static_cast<float>(bitmap->GetHeight());
		m_rectTransform->SetSize(bw, bh);
		m_rectTransform->SetScale(width / bw, height / bh);
	}
	else
	{
		m_rectTransform->SetSize(width, height);
		m_rectTransform->SetScale(1.0f, 1.0f);
	}
}


UIImage::~UIImage()
{
	Release();
}

void UIImage::Update(float deltaTime)
{
	// UIElement는 Update()를 오버라이드하지 않으므로 GameObject::Update() 호출
	GameObject::Update(deltaTime);
}

void UIImage::Render()
{
    if (!IsEnabled() || !m_image) return;

	m_image->Render();
}

Gdiplus::Bitmap* UIImage::GetBitmap() const
{
	return m_image ? m_image->GetSprite() : nullptr;
}

void UIImage::SetSprite(const std::shared_ptr<Sprite>& sprite) 
{
	if(m_image != nullptr)
		m_image->SetSprite(sprite);
}

void UIImage::LoadSprite(const std::wstring& imagePath)
{
	if (m_image != nullptr) {
		m_image->LoadSprite(imagePath);
	}
}

void UIImage::SetTintColor(const Gdiplus::Color& color)
{
	if (m_image != nullptr) {
		m_image->SetTintColor(color);
	}
}

void UIImage::SetTintColor(BYTE r, BYTE g, BYTE b, BYTE a)
{
	if (m_image != nullptr) {
		m_image->SetTintColor(r, g, b, a);
	}
}

void UIImage::SetAlpha(BYTE alpha)
{
	if (m_image != nullptr) {
		m_image->SetAlpha(alpha);
	}
}

Gdiplus::Color UIImage::GetTintColor() const
{
	if (m_image != nullptr) {
		return m_image->GetTintColor();
	}
	return Gdiplus::Color(255, 255, 255, 255);
}

void UIImage::Release()
{
	// UIImage 전용 정리 작업
	m_rectTransform = nullptr;
	m_image = nullptr;
	
	// 부모 클래스의 Release() 호출하여 컴포넌트 정리
	UIElement::Release();
}
