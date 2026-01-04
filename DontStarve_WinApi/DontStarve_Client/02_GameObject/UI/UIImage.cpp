#include "../../99_Default/pch.h"
#include "UIImage.h"
#include "../Component/Transform/RectTransform.h"
#include "../Component/Sprite/Image.h"
#include "../../01_Manager/RenderManager/RenderManager.h"

UIImage::UIImage(GameObjectID id, float x, float y, float width, float height, RenderLayer layer, const std::wstring& imagePath, float sortKey)
	: GameObject(GOBJ_UI, id, L"", L"", true, false)
{
	// RectTransform 컴포넌트 추가
	m_rectTransform = AddComponent<RectTransform>();
	m_rectTransform->SetPosition(x, y);
	m_rectTransform->SetPivot(0.5f, 0.5f);

	// Image 컴포넌트 추가
	m_image = AddComponent<::Image>();
	m_image->SetLayer(layer);
	m_image->SetSortKey(sortKey);

	// 이미지 로드
	LoadBitmap(imagePath);
	
	// 비트맵 로드 후 크기에 맞춰 scale 계산
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

void UIImage::LoadBitmap(const std::wstring& imagePath)
{
	if (m_image && !imagePath.empty()) {
		m_image->LoadSprite(imagePath);
	}
}

void UIImage::Update(float deltaTime)
{
	// UIImage는 정적 이미지이므로 특별한 업데이트 없음
}

void UIImage::Render()
{
    if (!IsEnabled() || !m_image || !m_rectTransform) return;

	Gdiplus::Bitmap* bitmap = m_image->GetSprite();
	if (!bitmap) return;

    // RenderManager를 통해 UI 이미지 렌더링
	// Sprite 크기 * scale 계산
	float bitmapWidth = static_cast<float>(bitmap->GetWidth());
	float bitmapHeight = static_cast<float>(bitmap->GetHeight());
	float width = bitmapWidth * m_rectTransform->GetScaleX();
	float height = bitmapHeight * m_rectTransform->GetScaleY();
	
	float x = m_rectTransform->GetX();
	float y = m_rectTransform->GetY();
	float pivotX = m_rectTransform->GetPivotX();
	float pivotY = m_rectTransform->GetPivotY();

    RenderManager::GetInstance()->RenderUIImage(
        bitmap,
        x - (pivotX * width),  // destLeft
        y - (pivotY * height), // destTop
        width,
        height,
		m_image->GetLayer(),  // Image 컴포넌트의 레이어 사용
		m_image->GetSortKey()  // Image 컴포넌트의 정렬 키 사용
    );
}

Gdiplus::Bitmap* UIImage::GetBitmap() const
{
	return m_image ? m_image->GetSprite() : nullptr;
}

void UIImage::Release()
{
	// 컴포넌트는 GameObject의 Release에서 자동으로 해제됨
	m_rectTransform = nullptr;
	m_image = nullptr;
}
