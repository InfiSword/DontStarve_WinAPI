#include "../../99_Default/pch.h"
#include "UIImage.h"
#include "../../01_Manager/RenderManager/RenderManager.h"

UIImage::UIImage(GameObjectID id, float x, float y, float width, float height, RenderLayer layer, const std::wstring& imagePath, float sortKey)
	: GameObject(GOBJ_UI, id, x, y, 0.5f, 0.5f, DIR_DOWN, L"", L""), m_layer(layer), m_sortKey(sortKey)
{
	m_orignalBitmap = nullptr;
	m_width = width;
	m_height = height;
	LoadBitmap(imagePath);
}

UIImage::~UIImage()
{
	Release();
}

void UIImage::LoadBitmap(const std::wstring& imagePath)
{
	if (!imagePath.empty()) {
		m_orignalBitmap = new Gdiplus::Bitmap(imagePath.c_str());
		if (m_orignalBitmap && m_orignalBitmap->GetLastStatus() != Gdiplus::Ok) {
			delete m_orignalBitmap;
			m_orignalBitmap = nullptr;
		}
	}
}

void UIImage::Update(float deltaTime)
{
	// UIImage는 정적이므로 특별한 업데이트 없음
}

void UIImage::Render()
{
    if (!GetActive() || !m_orignalBitmap) return;

    // RenderManager를 통한 UI 렌더링으로 통일
    RenderManager::GetInstance()->RenderUIImage(
        m_orignalBitmap,
        m_x - (m_pivotX * m_width),  // destLeft
        m_y - (m_pivotY * m_height), // destTop
        m_width,
        m_height,
		m_layer,  // UIImage의 레이어 사용
		m_sortKey  // UIImage의 정렬 키 사용
    );
}

Gdiplus::Bitmap* UIImage::GetBitmap() const
{
	return m_orignalBitmap;
}

void UIImage::Release()
{
	if (m_orignalBitmap) {
		delete m_orignalBitmap;
		m_orignalBitmap = nullptr;
	}
}