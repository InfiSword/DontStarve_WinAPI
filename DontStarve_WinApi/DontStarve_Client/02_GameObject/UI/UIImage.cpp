#include "99_Default/pch.h"
#include "UIImage.h"
#include "../Component/Transform/RectTransform.h"
#include "../Component/Sprite/Image.h"
#include "../../01_Manager/RenderManager/RenderManager.h"

UIImage::UIImage(GameObjectID id, float x, float y, float width, float height, RenderLayer layer, const std::wstring& imagePath, float sortKey)
	: GameObject(GOBJ_UI, id, L"", L"", true, false)
{
	m_rectTransform = AddComponent<RectTransform>();
	m_rectTransform->SetPosition(x, y);
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

void UIImage::Release()
{
	m_rectTransform = nullptr;
	m_image = nullptr;
}
