#include "../../99_Default/pch.h"
#include "UIButton.h"
#include "../../01_Manager/InputManager/InputManager.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../01_Manager/RenderManager/RenderManager.h"

UIButton::UIButton(GameObjectID id, float x, float y, float width, float height,
	const std::wstring& normalImagePath, const std::wstring& hoverImagePath, const std::wstring buttonText)
	: GameObject(GOBJ_UI, id, x, y, 0.5f, 0.5f, DIR_DOWN, L"", L""),
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
	m_width = width;
	m_height = height;
	LoadBitmaps(normalImagePath, hoverImagePath);
	InitializeText();
}

UIButton::~UIButton()
{
	Release();
}

void UIButton::LoadBitmaps(const std::wstring& normalImagePath, const std::wstring& hoverImagePath)
{
	// Normal 비트맵 로드
	if (!normalImagePath.empty()) {
		m_orignalBitmap = new Gdiplus::Bitmap(normalImagePath.c_str());
		if (m_orignalBitmap && m_orignalBitmap->GetLastStatus() != Gdiplus::Ok) {
			delete m_orignalBitmap;
			m_orignalBitmap = nullptr;
		}
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

	// 기본 문자열 정렬 설정 (중앙 정렬)
	m_stringFormat = new Gdiplus::StringFormat();

	m_stringFormat->SetAlignment(Gdiplus::StringAlignmentCenter);
	m_stringFormat->SetLineAlignment(Gdiplus::StringAlignmentCenter);

	// 폰트 패밀리 유효성 재확인
	Gdiplus::FontFamily fontFamily;
	if (m_font->GetFamily(&fontFamily) != Gdiplus::Ok) {
		Release();
		return;
	}


}

void UIButton::Update(float deltaTime)
{
	if (!GetActive()) return;

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
	float left = m_x - (m_width * m_pivotX);
	float top = m_y - (m_height * m_pivotY);
	float right = left + m_width;
	float bottom = top + m_height;

	return (x >= left && x <= right && y >= top && y <= bottom);
}

void UIButton::Render()
{
	if (!GetActive()) return;

	// 비활성화된 버튼은 비활성화 스타일로 렌더링
	if (m_isDisabled) {
		RenderDisabled();
		return;
	}

	Gdiplus::Bitmap* currentBitmap = GetBitmap();
	if (!currentBitmap) return;

	// RenderManager를 통해 UI 이미지 렌더링
	RenderManager::GetInstance()->RenderUIImage(
		currentBitmap,
		m_x - (m_pivotX * m_width),  // destLeft
		m_y - (m_pivotY * m_height), // destTop
		m_width,
		m_height,
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
			RenderManager::GetInstance()->RenderUIText(
				m_buttonText,
				m_font,
				m_textBrush,
				m_x - (m_pivotX * m_width),  // x
				m_y - (m_pivotY * m_height), // y
				m_width,
				m_height,
				LAYER_UI_FOREGROUND,
				static_cast<float>(m_buttonState) + 0.1f  // 이미지보다 위에 표시
			);
		}
	}
}

Gdiplus::Bitmap* UIButton::GetBitmap() const
{
	switch (m_buttonState) {
	case ButtonState::HOVER:
	case ButtonState::CLICKED:
		return (m_hoverBitmap) ? m_hoverBitmap : m_orignalBitmap;
	case ButtonState::DISABLED:
	case ButtonState::NORMAL:
	default:
		return m_orignalBitmap;
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
	if (m_orignalBitmap) {
		delete m_orignalBitmap;
		m_orignalBitmap = nullptr;
	}
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
        // 비활성화 상태에서는 마우스 이벤트를 받지 않음
    } else {
        m_buttonState = ButtonState::NORMAL;
    }
}

void UIButton::RenderDisabled()
{
    if (!m_orignalBitmap) return;
    
    // 비활성화된 버튼을 어둡게 렌더링 (색상을 30%로 줄임)
    Gdiplus::ColorMatrix colorMatrix = {
        0.3f, 0.0f, 0.0f, 0.0f, 0.0f,  // Red (30%)
        0.0f, 0.3f, 0.0f, 0.0f, 0.0f,  // Green (30%)
        0.0f, 0.0f, 0.3f, 0.0f, 0.0f,  // Blue (30%)
        0.0f, 0.0f, 0.0f, 0.7f, 0.0f,  // Alpha (70% 투명도)
        0.0f, 0.0f, 0.0f, 0.0f, 1.0f   // Additional
    };
    
    Gdiplus::ImageAttributes imgAttr;
    imgAttr.SetColorMatrix(&colorMatrix, Gdiplus::ColorMatrixFlagsDefault, Gdiplus::ColorAdjustTypeBitmap);
    
    Gdiplus::RectF destRect(GetX() - GetWidth() / 2.0f, GetY() - GetHeight() / 2.0f, GetWidth(), GetHeight());

	RenderManager::GetInstance()->RenderUIImage(
		m_orignalBitmap,
		m_x - (m_pivotX * m_width),  // destLeft
		m_y - (m_pivotY * m_height), // destTop
		m_width,
		m_height,
		LAYER_UI_FOREGROUND,
		static_cast<float>(m_buttonState)  // 버튼 상태를 sortKey로 사용
	);

   /* pGraphics->DrawImage(m_normalBitmap, destRect, 0, 0, 
                        static_cast<float>(m_normalBitmap->GetWidth()), 
                        static_cast<float>(m_normalBitmap->GetHeight()), 
                        Gdiplus::UnitPixel, &imgAttr);*/
}
