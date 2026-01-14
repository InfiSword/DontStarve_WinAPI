#include "99_Default/pch.h"
#include "UIButton.h"
#include "../Component/Transform/RectTransform.h"
#include "../Component/Sprite/Image.h"
#include "../Component/Text/Text.h"
#include "../Component/Button/Button.h"
#include "../../01_Manager/InputManager/InputManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../01_Manager/RenderManager/RenderManager.h"

UIButton::UIButton(GameObjectID id, float x, float y, float width, float height,
	const std::wstring& normalImagePath, const std::wstring& hoverImagePath, const std::wstring buttonText)
	: GameObject(GOBJ_UI, id, L"", L"", true, false),
	m_rectTransform(nullptr),
	m_image(nullptr),
	m_buttonComp(nullptr),
	m_textComp(nullptr)
{
	// RectTransform 컴포넌트 추가
	m_rectTransform = AddComponent<RectTransform>();
	m_rectTransform->SetPosition(x, y);
	m_rectTransform->SetPivot(0.5f, 0.5f);
	
	// Image 컴포넌트 추가 (생성자에서 초기화)
	m_image = AddComponent<ComponentElement::Image>();
	m_image->SetLayer(LAYER_UI_FOREGROUND);
	m_image->SetSortKey(0.0f);

	// Button 컴포넌트 추가 (상태별 스타일 전달)
	// UIButton은 UIImage처럼 전체 경로를 직접 사용
	std::wstring normalFullPath = normalImagePath;
	std::wstring hoverFullPath = hoverImagePath;

	ButtonVisualState normal{
		Gdiplus::Color(Gdiplus::Color::Black),
		LAYER_UI_FOREGROUND,
		0.0f,
		L"", // 기본 상태는 현재 sprite 유지
		width,
		height
	};
	ButtonVisualState hover{
		Gdiplus::Color(Gdiplus::Color::Black),
		LAYER_UI_FOREGROUND,
		0.0f, // 이미지 sortKey offset은 버튼 상태 값으로 더해짐
		L"", // hover sprite는 직접 관리하므로 경로 불필요
		width,
		height
	};
	ButtonVisualState disabled{
		Gdiplus::Color(160, 160, 160), // 회색 톤
		LAYER_UI_FOREGROUND,
		0.0f,
		L"",
		width,
		height
	};

	m_buttonComp = AddComponent<Button>(normal, hover, disabled);
	
	// normal과 hover 스프라이트 미리 로드
	LoadBitmaps(normalFullPath, hoverFullPath);
	
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
	
	// Text 컴포넌트 추가 (한글 지원 폰트 사용)
	m_textComp = AddComponent<Text>(
		buttonText,
		Gdiplus::Color::Black,
		width,
		height,
		LAYER_UI_FOREGROUND,
		m_image ? m_image->GetSortKey() + 0.1f : 0.1f,
		L"맑은 고딕",  // 한글 지원 폰트
		16.0f,
		Gdiplus::StringAlignmentCenter,
		Gdiplus::StringAlignmentCenter
	);
}

UIButton::~UIButton()
{
	Release();
}

void UIButton::LoadBitmaps(const std::wstring& normalImagePath, const std::wstring& hoverImagePath)
{
	// Image 컴포넌트 가져오기
	if (!m_image) {
		m_image = AddComponent<ComponentElement::Image>();
		m_image->SetLayer(LAYER_UI_FOREGROUND);
		m_image->SetSortKey(0.0f);
	}

	// Normal 비트맵 로드 및 저장
	if (!normalImagePath.empty()) {
		m_normalSprite = ResourceManager::GetInstance()->LoadSprite(normalImagePath);
		if (m_normalSprite) {
			m_image->SetSprite(m_normalSprite);
		}
	}

	// Hover 비트맵 미리 로드 및 저장
	if (!hoverImagePath.empty()) {
		m_hoverSprite = ResourceManager::GetInstance()->LoadSprite(hoverImagePath);
	}
}

void UIButton::Update(float deltaTime)
{
	if (!IsEnabled()) return;

	// UpdateState는 Render()에서 호출됩니다
}

void UIButton::Render()
{
	if (!IsEnabled()) return;

	if (!m_rectTransform || !m_image || !m_buttonComp) return;

	// 상태 업데이트
	m_buttonComp->UpdateState(m_rectTransform, m_image);

	// 비활성화된 버튼은 비활성화 스타일로 렌더링
	if (IsDisabled()) {
		RenderDisabled();
		return;
	}

	ButtonRenderParams params = m_buttonComp->GetRenderParams(m_rectTransform, m_image);

	// 버튼 상태에 따라 스프라이트 교체
	ButtonState currentState = m_buttonComp->GetState();
	if (currentState == ButtonState::HOVER || currentState == ButtonState::CLICKED) {
		// Hover 상태: hover 스프라이트 사용
		if (m_hoverSprite) {
			m_image->SetSprite(m_hoverSprite);
			params.bitmap = m_hoverSprite->bitmap.get();
		}
	}
	else {
		// Normal 상태: normal 스프라이트 사용
		if (m_normalSprite) {
			m_image->SetSprite(m_normalSprite);
			params.bitmap = m_normalSprite->bitmap.get();
		}
	}

	if (!params.bitmap) return;

	// Image 스타일 적용
	ComponentElement::ImageStyle imgStyle{ params.layer, params.sortKey };
	m_image->ApplyStyle(imgStyle);

	float x = m_rectTransform->GetX();
	float y = m_rectTransform->GetY();

	RenderManager::GetInstance()->RenderUIImageWithPivot(
		params.bitmap,
		x,
		y,
		params.targetWidth,
		params.targetHeight,
		params.pivotX,
		params.pivotY,
		m_image->GetLayer(),
		m_image->GetSortKey()
	);

	// 텍스트 렌더링
	if (m_textComp) {
		// 스타일 적용
		TextStyle tstyle{
			params.textColor,
			params.layer,
			params.textSortKey,
			params.targetWidth,
			params.targetHeight
		};
		m_textComp->ApplyStyle(tstyle);
		auto textParams = m_textComp->BuildRenderParams(m_rectTransform);
		if (!textParams.text.empty() && textParams.font && textParams.brush && textParams.format) {
			RenderManager::GetInstance()->AddTextCommand(
				textParams.text,
				textParams.font,
				textParams.brush,
				textParams.format,
				textParams.destRect,
				textParams.layer,
				textParams.sortKey
			);
		}
	}

}

Gdiplus::Bitmap* UIButton::GetBitmap() const
{
	return m_image ? m_image->GetSprite() : nullptr;
}

void UIButton::SetOnClickCallback(std::function<void()> callback)
{
	if (m_buttonComp) {
		m_buttonComp->SetOnClickCallback(std::move(callback));
	}
}

ButtonState UIButton::GetButtonState() const
{
	return m_buttonComp ? m_buttonComp->GetState() : ButtonState::NORMAL;
}

void UIButton::Release()
{
	m_rectTransform = nullptr;
	m_image = nullptr;
	m_buttonComp = nullptr;
	m_textComp = nullptr;
}

void UIButton::SetDisabled(bool disabled)
{
	if (m_buttonComp) {
		m_buttonComp->SetDisabled(disabled);
	}
}

bool UIButton::IsDisabled() const
{
	return m_buttonComp ? m_buttonComp->IsDisabled() : false;
}

void UIButton::RenderDisabled()
{
	if (!m_image || !m_rectTransform) return;
	Gdiplus::Bitmap* normalBitmap = m_image->GetSprite();
    if (!normalBitmap) return;
    
	// Sprite 크기 * scale 계산
	float bitmapWidth = static_cast<float>(normalBitmap->GetWidth());
	float bitmapHeight = static_cast<float>(normalBitmap->GetHeight());
	float width = bitmapWidth * m_rectTransform->GetScaleX();
	float height = bitmapHeight * m_rectTransform->GetScaleY();
	
	float x = m_rectTransform->GetX();
	float y = m_rectTransform->GetY();
	float pivotX = m_rectTransform->GetPivotX();
	float pivotY = m_rectTransform->GetPivotY();

	RenderManager::GetInstance()->RenderUIImageWithPivot(
		normalBitmap,
		x,
		y,
		width,
		height,
		pivotX,
		pivotY,
		LAYER_UI_FOREGROUND,
		static_cast<float>(ButtonState::DISABLED)  // 버튼 상태를 sortKey로 사용
	);

   /* pGraphics->DrawImage(m_normalBitmap, destRect, 0, 0, 
                        static_cast<float>(m_normalBitmap->GetWidth()), 
                        static_cast<float>(m_normalBitmap->GetHeight()), 
                        Gdiplus::UnitPixel, &imgAttr);*/
}
