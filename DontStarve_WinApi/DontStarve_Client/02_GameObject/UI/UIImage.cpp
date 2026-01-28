#include "99_Default/pch.h"
#include "UIImage.h"
#include "../Component/Transform/RectTransform.h"
#include "../Component/Sprite/Image.h"
#include "../../01_Manager/RenderManager/RenderManager.h"

UIImage::UIImage(GameObjectID id, float width, float height, RenderLayer layer, const std::wstring& imagePath, float sortKey,
                 float anchorMinX, float anchorMinY, float anchorMaxX, float anchorMaxY,
                 float anchoredPosX, float anchoredPosY)
	: UIElement(GOBJ_UI, id, L"", L"", true, false)
{
	// UIElement에서 이미 RectTransform이 생성되었으므로 GetRectTransform() 사용
	m_rectTransform = GetRectTransform();
	m_rectTransform->SetAnchorMin(anchorMinX, anchorMinY);
	m_rectTransform->SetAnchorMax(anchorMaxX, anchorMaxY);
	m_rectTransform->SetAnchoredPosition(anchoredPosX, anchoredPosY);
	m_rectTransform->SetPivot(0.5f, 0.5f);

	m_image = AddComponent<ComponentElement::Image>();
	m_image->SetLayer(layer);
	m_image->SetSortKey(sortKey);
	m_image->LoadSprite(imagePath);
	
	
	if (m_image && m_image->GetSprite()) {
		Gdiplus::Bitmap* bitmap = m_image->GetSprite();
		float bitmapWidth = static_cast<float>(bitmap->GetWidth());
		float bitmapHeight = static_cast<float>(bitmap->GetHeight());
		if (bitmapWidth > 0 && bitmapHeight > 0) {
			float scaleX = width / bitmapWidth;
			float scaleY = height / bitmapHeight;
			m_rectTransform->SetScale(scaleX, scaleY);
		}
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
    if (!IsEnabled() || !m_image || !m_rectTransform) return;

	std::shared_ptr<Sprite> spriteHandle = m_image->GetSpriteHandle();
	if (!spriteHandle || !spriteHandle->bitmap) return;

	Gdiplus::Bitmap* bitmap = spriteHandle->bitmap.get();

	Gdiplus::RectF srcRect = spriteHandle->sourceRect;
	float width = srcRect.Width * m_rectTransform->GetScaleX();
	float height = srcRect.Height * m_rectTransform->GetScaleY();
	
	float x = m_rectTransform->GetX();
	float y = m_rectTransform->GetY();
	float pivotX = m_rectTransform->GetPivotX();
	float pivotY = m_rectTransform->GetPivotY();

    RenderManager::GetInstance()->RenderUIImageWithPivot(
        bitmap,
        x,
        y,
        width,
        height,
		pivotX,
		pivotY,
		m_image->GetLayer(),  // Image 컴포넌트의 레이어 사용
		m_image->GetSortKey()  // Image 컴포넌트의 정렬 키 사용
    );
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

void UIImage::Release()
{
	// UIImage 전용 정리 작업
	m_rectTransform = nullptr;
	m_image = nullptr;
	
	// 부모 클래스의 Release() 호출하여 컴포넌트 정리
	UIElement::Release();
}
