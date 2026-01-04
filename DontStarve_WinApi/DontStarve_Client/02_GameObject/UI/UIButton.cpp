#include "../../99_Default/pch.h"
#include "UIButton.h"
#include "../Component/Transform/RectTransform.h"
#include "../Component/Sprite/Image.h"
#include "../../01_Manager/InputManager/InputManager.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../01_Manager/RenderManager/RenderManager.h"

// UIButton 헬퍼 함수
static RectTransform* GetRectTransform(const UIButton* button) {
	return button->GetComponent<RectTransform>();
}

UIButton::UIButton(GameObjectID id, float x, float y, float width, float height,
	const std::wstring& normalImagePath, const std::wstring& hoverImagePath, const std::wstring buttonText)
	: GameObject(GOBJ_UI, id, L"", L"", true, false),
	m_buttonState(ButtonState::NORMAL),
	m_hoverBitmap(nullptr),
	m_isMouseOver(false),
	m_wasClicked(false),
	m_isDisabled(false),
	m_buttonText(buttonText),
	m_font(nullptr),
	m_textBrush(nullptr),
	m_stringFormat(nullptr)
{
	// RectTransform 컴포넌트 추가
	RectTransform* rectTransform = AddComponent<RectTransform>();
	rectTransform->SetPosition(x, y);
	rectTransform->SetPivot(0.5f, 0.5f);
	LoadBitmaps(normalImagePath, hoverImagePath);
	
	// 비트맵 로드 후 크기에 맞춰 scale 계산
	::Image* image = GetComponent<::Image>();
	if (image && image->GetSprite()) {
		Gdiplus::Bitmap* bitmap = image->GetSprite();
		float bitmapWidth = static_cast<float>(bitmap->GetWidth());
		float bitmapHeight = static_cast<float>(bitmap->GetHeight());
		if (bitmapWidth > 0 && bitmapHeight > 0) {
			float scaleX = width / bitmapWidth;
			float scaleY = height / bitmapHeight;
			rectTransform->SetScale(scaleX, scaleY);
		}
	}
	
	InitializeText();
}

UIButton::~UIButton()
{
	Release();
}

void UIButton::LoadBitmaps(const std::wstring& normalImagePath, const std::wstring& hoverImagePath)
{
	// Image 컴포넌트 가져오기
	::Image* image = GetComponent<::Image>();
	if (!image) {
		// Image 컴포넌트가 없으면 추가
		image = AddComponent<::Image>();
		image->SetLayer(LAYER_UI_FOREGROUND);
	}

	// Normal 비트맵 로드
	if (!normalImagePath.empty()) {
		image->LoadSprite(normalImagePath);
	}

	// Hover 비트맵 로드
	if (!hoverImagePath.empty()) {
		m_hoverBitmap = new Gdiplus::Bitmap(hoverImagePath.c_str());
		if (m_hoverBitmap && m_hoverBitmap->GetLastStatus() != Gdiplus::Ok) {
			delete m_hoverBitmap;
			m_hoverBitmap = nullptr;
		}
	}
}

void UIButton::InitializeText()
{
	// 기존 텍스트 리소스가 있으면 해제
	if (m_font) {
		delete m_font;
		m_font = nullptr;
	}
	if (m_textBrush) {
		delete m_textBrush;
		m_textBrush = nullptr;
	}
	if (m_stringFormat) {
		delete m_stringFormat;
		m_stringFormat = nullptr;
	}

	// 기본 폰트 생성 (크기 16)
	m_font = new Gdiplus::Font(L"Arial", 16.0f);

	// 기본 브러시 생성 (검은색)
	m_textBrush = new Gdiplus::SolidBrush(Gdiplus::Color::Black);

	// 기본 문자열 포맷 생성 (중앙 정렬)
	m_stringFormat = new Gdiplus::StringFormat();

	m_stringFormat->SetAlignment(Gdiplus::StringAlignmentCenter);
	m_stringFormat->SetLineAlignment(Gdiplus::StringAlignmentCenter);

	// 폰트 유효성을 확인하고 설정
	Gdiplus::FontFamily fontFamily;
	if (m_font->GetFamily(&fontFamily) != Gdiplus::Ok) {
		Release();
		return;
	}


}

void UIButton::Update(float deltaTime)
{
	if (!IsEnabled()) return;

	CheckMouseInteraction();
}

void UIButton::CheckMouseInteraction()
{
	// 비활성화된 버튼은 마우스 이벤트를 처리하지 않음
	if (m_isDisabled) {
		m_isMouseOver = false;
		m_buttonState = ButtonState::DISABLED;
		return;
	}

	// 마우스 위치 가져오기
	POINT mousePos = InputManager::GetInstance()->GetMousePos();
	float mouseX = static_cast<float>(mousePos.x);
	float mouseY = static_cast<float>(mousePos.y);

	// 마우스가 버튼 영역 안에 있는지 확인
	bool wasMouseOver = m_isMouseOver;
	m_isMouseOver = IsPointInside(mouseX, mouseY);

	// 마우스 클릭 처리
	if (m_isMouseOver && InputManager::GetInstance()->IsLButtonClicked()) {
		m_buttonState = ButtonState::CLICKED;
		if (m_onClickCallback) {
			m_onClickCallback();
		}
	}
	else if (m_isMouseOver) {
		m_buttonState = ButtonState::HOVER;
	}
	else {
		m_buttonState = ButtonState::NORMAL;
	}
}

bool UIButton::IsPointInside(float x, float y) const
{
	RectTransform* rectTransform = GetComponent<RectTransform>();
	if (!rectTransform) return false;

	// Sprite 크기 * scale 계산
	::Image* image = GetComponent<::Image>();
	if (!image || !image->GetSprite()) return false;
	
	Gdiplus::Bitmap* bitmap = image->GetSprite();
	float bitmapWidth = static_cast<float>(bitmap->GetWidth());
	float bitmapHeight = static_cast<float>(bitmap->GetHeight());
	float width = bitmapWidth * rectTransform->GetScaleX();
	float height = bitmapHeight * rectTransform->GetScaleY();
	
	float rectX = rectTransform->GetX();
	float rectY = rectTransform->GetY();
	float pivotX = rectTransform->GetPivotX();
	float pivotY = rectTransform->GetPivotY();

	float left = rectX - (width * pivotX);
	float top = rectY - (height * pivotY);
	float right = left + width;
	float bottom = top + height;

	return (x >= left && x <= right && y >= top && y <= bottom);
}

void UIButton::Render()
{
	if (!IsEnabled()) return;

	// 비활성화된 버튼은 비활성화 스타일로 렌더링
	if (m_isDisabled) {
		RenderDisabled();
		return;
	}

	Gdiplus::Bitmap* currentBitmap = GetBitmap();
	if (!currentBitmap) return;

	RectTransform* rectTransform = GetComponent<RectTransform>();
	if (!rectTransform) return;

	// Sprite 크기 * scale 계산
	float bitmapWidth = static_cast<float>(currentBitmap->GetWidth());
	float bitmapHeight = static_cast<float>(currentBitmap->GetHeight());
	float width = bitmapWidth * rectTransform->GetScaleX();
	float height = bitmapHeight * rectTransform->GetScaleY();
	
	float x = rectTransform->GetX();
	float y = rectTransform->GetY();
	float pivotX = rectTransform->GetPivotX();
	float pivotY = rectTransform->GetPivotY();

	// RenderManager를 통해 UI 이미지 렌더링
	RenderManager::GetInstance()->RenderUIImage(
		currentBitmap,
		x - (pivotX * width),  // destLeft
		y - (pivotY * height), // destTop
		width,
		height,
		LAYER_UI_FOREGROUND,
		static_cast<float>(m_buttonState)  // 버튼 상태를 sortKey로 사용
	);

	// 텍스트 렌더링 (버튼 이미지 위에 표시)
	// 모든 텍스트 리소스가 유효한지 다시 한번 확인
	if (!m_buttonText.empty() && m_font && m_textBrush && m_stringFormat) {
		// 폰트와 브러시 유효성 확인
		Gdiplus::FontFamily fontFamily;
		if (m_font->GetFamily(&fontFamily) == Gdiplus::Ok) {
			// 텍스트가 비어있지 않고 모든 텍스트 리소스가 유효한 경우에만 렌더링
			RectTransform* rectTransform = GetComponent<RectTransform>();
			if (rectTransform) {
				// Sprite 크기 * scale 계산
				::Image* image = GetComponent<::Image>();
				if (!image || !image->GetSprite()) return;
				
				Gdiplus::Bitmap* bitmap = image->GetSprite();
				float bitmapWidth = static_cast<float>(bitmap->GetWidth());
				float bitmapHeight = static_cast<float>(bitmap->GetHeight());
				float width = bitmapWidth * rectTransform->GetScaleX();
				float height = bitmapHeight * rectTransform->GetScaleY();
				
				float x = rectTransform->GetX();
				float y = rectTransform->GetY();
				float pivotX = rectTransform->GetPivotX();
				float pivotY = rectTransform->GetPivotY();

				RenderManager::GetInstance()->RenderUIText(
					m_buttonText,
					m_font,
					m_textBrush,
					x - (pivotX * width),  // x
					y - (pivotY * height), // y
					width,
					height,
					LAYER_UI_FOREGROUND,
					static_cast<float>(m_buttonState) + 0.1f  // 이미지보다 위에 표시
				);
			}
		}
	}
}

Gdiplus::Bitmap* UIButton::GetBitmap() const
{
	::Image* image = GetComponent<::Image>();
	Gdiplus::Bitmap* normalBitmap = image ? image->GetSprite() : nullptr;

	switch (m_buttonState) {
	case ButtonState::HOVER:
	case ButtonState::CLICKED:
		return (m_hoverBitmap) ? m_hoverBitmap : normalBitmap;
	case ButtonState::DISABLED:
	case ButtonState::NORMAL:
	default:
		return normalBitmap;
	}
}

void UIButton::SetOnClickCallback(std::function<void()> callback)
{
	m_onClickCallback = callback;
}

ButtonState UIButton::GetButtonState() const
{
	return m_buttonState;
}

void UIButton::Release()
{
	// Image 컴포넌트는 GameObject의 Release에서 자동으로 해제됨
	if (m_hoverBitmap) {
		delete m_hoverBitmap;
		m_hoverBitmap = nullptr;
	}

	// 텍스트 관련 리소스 해제
	if (m_font) {
		delete m_font;
		m_font = nullptr;
	}
	if (m_textBrush) {
		delete m_textBrush;
		m_textBrush = nullptr;
	}
	if (m_stringFormat) {
		delete m_stringFormat;
		m_stringFormat = nullptr;
	}
}

void UIButton::SetDisabled(bool disabled)
{
    m_isDisabled = disabled;
    if (disabled) {
        m_buttonState = ButtonState::DISABLED;
        // 비활성화된 버튼은 마우스 이벤트를 받지 않음
    } else {
        m_buttonState = ButtonState::NORMAL;
    }
}

void UIButton::RenderDisabled()
{
	::Image* image = GetComponent<::Image>();
	Gdiplus::Bitmap* normalBitmap = image ? image->GetSprite() : nullptr;
    if (!normalBitmap) return;
    
    // 비활성화된 버튼의 색상을 어둡게 (밝기를 30%로 줄임)
    Gdiplus::ColorMatrix colorMatrix = {
        0.3f, 0.0f, 0.0f, 0.0f, 0.0f,  // Red (30%)
        0.0f, 0.3f, 0.0f, 0.0f, 0.0f,  // Green (30%)
        0.0f, 0.0f, 0.3f, 0.0f, 0.0f,  // Blue (30%)
        0.0f, 0.0f, 0.0f, 0.7f, 0.0f,  // Alpha (70% 불투명도)
        0.0f, 0.0f, 0.0f, 0.0f, 1.0f   // Additional
    };
    
    Gdiplus::ImageAttributes imgAttr;
    imgAttr.SetColorMatrix(&colorMatrix, Gdiplus::ColorMatrixFlagsDefault, Gdiplus::ColorAdjustTypeBitmap);
    
	RectTransform* rectTransform = GetComponent<RectTransform>();
	if (!rectTransform) return;

	// Sprite 크기 * scale 계산
	float bitmapWidth = static_cast<float>(normalBitmap->GetWidth());
	float bitmapHeight = static_cast<float>(normalBitmap->GetHeight());
	float width = bitmapWidth * rectTransform->GetScaleX();
	float height = bitmapHeight * rectTransform->GetScaleY();
	
	float x = rectTransform->GetX();
	float y = rectTransform->GetY();
	float pivotX = rectTransform->GetPivotX();
	float pivotY = rectTransform->GetPivotY();
    Gdiplus::RectF destRect(x - width / 2.0f, y - height / 2.0f, width, height);

	RenderManager::GetInstance()->RenderUIImage(
		normalBitmap,
		x - (pivotX * width),  // destLeft
		y - (pivotY * height), // destTop
		width,
		height,
		LAYER_UI_FOREGROUND,
		static_cast<float>(m_buttonState)  // 버튼 상태를 sortKey로 사용
	);

   /* pGraphics->DrawImage(m_normalBitmap, destRect, 0, 0, 
                        static_cast<float>(m_normalBitmap->GetWidth()), 
                        static_cast<float>(m_normalBitmap->GetHeight()), 
                        Gdiplus::UnitPixel, &imgAttr);*/
}
