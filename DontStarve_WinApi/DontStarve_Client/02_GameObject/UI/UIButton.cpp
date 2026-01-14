#include "99_Default/pch.h"
#include "UIButton.h"
#include "../Component/Transform/RectTransform.h"
#include "../Component/Sprite/Image.h"
#include "../Component/Sprite/Sprite.h"
#include "../Component/Text/Text.h"
#include "../Component/Button/Button.h"
#include "../../01_Manager/InputManager/InputManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../01_Manager/RenderManager/RenderManager.h"

UIButton::UIButton(GameObjectID id, float x, float y, float width, float height,
	const std::wstring& normalImagePath, const std::wstring& hoverImagePath, const std::wstring buttonText)
	: UIElement(GOBJ_UI, id, L"", L"", true, false),
	m_image(nullptr),
	m_buttonComp(nullptr),
	m_textComp(nullptr),
	m_normalSprite(nullptr),
	m_hoverSprite(nullptr),
	m_textNormalColor(Gdiplus::Color(Gdiplus::Color::Black)),
	m_textHoverColor(Gdiplus::Color(Gdiplus::Color::Black)),
	m_textClickedColor(Gdiplus::Color(Gdiplus::Color::Black)),
	m_textDisabledColor(Gdiplus::Color(160, 160, 160)),
	m_previousState(ButtonState::NORMAL)
{
	// UIElement에서 이미 RectTransform이 생성되었으므로 GetRectTransform() 사용
	m_rectTransform = GetRectTransform();
	m_rectTransform->SetPosition(x, y);
	m_rectTransform->SetPivot(0.5f, 0.5f);
	
	// Image 컴포넌트 추가 (생성자에서 초기화)
	m_image = AddComponent<ComponentElement::Image>();
	m_image->SetLayer(LAYER_UI_FOREGROUND);
	m_image->SetSortKey(0.0f);

	std::wstring normalFullPath = normalImagePath;
	std::wstring hoverFullPath = hoverImagePath;

	// Button 컴포넌트 스타일 설정
	ButtonStateStyle normalStyle{
		Gdiplus::Color(255, 255, 255, 255),     // spriteColor (기본: 흰색)
		LAYER_UI_FOREGROUND,
		0.0f
	};
	ButtonStateStyle hoverStyle{
		Gdiplus::Color(200, 200, 200, 255),     // spriteColor (연한 회색빛)
		LAYER_UI_FOREGROUND,
		0.0f
	};
	ButtonStateStyle clickedStyle{
		Gdiplus::Color(200, 200, 200, 255),     // spriteColor (연한 회색빛, 필요시 변경 가능)
		LAYER_UI_FOREGROUND,
		0.0f
	};
	ButtonStateStyle disabledStyle{
		Gdiplus::Color(160, 160, 160, 255),     // spriteColor (회색 톤)
		LAYER_UI_FOREGROUND,
		0.0f
	};

	m_buttonComp = AddComponent<Button>(normalStyle, hoverStyle, clickedStyle, disabledStyle);
	
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

	GameObject::Update(deltaTime);
}

void UIButton::Render()
{
	if (!IsEnabled()) return;

	if (!m_rectTransform || !m_image || !m_buttonComp) return;

	// 버튼 상태 업데이트 (내부에서 상태 변경 시 ApplyVisualState 호출됨)
	m_buttonComp->UpdateState(m_rectTransform, m_image);
	
	// UpdateState() 후 현재 상태 확인
	ButtonState currentState = m_buttonComp->GetState();
	
	// 상태가 변경되었을 때 스프라이트 교체 (UIButton에서 관리)
	if (m_previousState != currentState) {
		if (currentState == ButtonState::HOVER || currentState == ButtonState::CLICKED) {
			if (m_hoverSprite) {
				m_image->SetSprite(m_hoverSprite);
			}
		}
		else {
			if (m_normalSprite) {
				m_image->SetSprite(m_normalSprite);
			}
		}
	}

	// 비활성화된 버튼은 비활성화 스타일로 렌더링
	if (m_buttonComp->IsDisabled()) {
		RenderDisabled();
		// 상태 업데이트 (Render() 함수의 마지막에 수행)
		m_previousState = currentState;
		return;
	}

	// 스프라이트가 없으면 렌더링 불가
	Gdiplus::Bitmap* bitmap = m_image->GetSprite();
	if (!bitmap) {
		// 상태 업데이트 (Render() 함수의 마지막에 수행)
		m_previousState = currentState;
		return;
	}

	float x = m_rectTransform->GetX();
	float y = m_rectTransform->GetY();
	
	// 비트맵 크기와 스케일 계산
	float bitmapWidth = static_cast<float>(bitmap->GetWidth());
	float bitmapHeight = static_cast<float>(bitmap->GetHeight());
	float width = bitmapWidth * m_rectTransform->GetScaleX();
	float height = bitmapHeight * m_rectTransform->GetScaleY();

	// 현재 상태에 따른 색상 틴트 정보 가져오기 (Button의 스타일에서)
	const ButtonStateStyle& style = m_buttonComp->GetStateStyle(currentState);
	Gdiplus::Color tintColor = style.spriteColor;
	bool hasTint = (tintColor.GetR() != 255 || tintColor.GetG() != 255 || tintColor.GetB() != 255 || tintColor.GetA() != 255);

	RenderManager::GetInstance()->RenderUIImageWithPivot(
		bitmap,
		x,
		y,
		width,
		height,
		m_rectTransform->GetPivotX(),
		m_rectTransform->GetPivotY(),
		m_image->GetLayer(),
		m_image->GetSortKey(),
		tintColor,
		hasTint
	);

	// Text 컴포넌트 스타일 적용 (Button과 독립적으로 관리)
	// 상태가 변경되었을 때만 스타일 재적용
	if (m_textComp && m_previousState != currentState) {
		// 현재 상태에 따른 Text 색상 결정
		Gdiplus::Color textColor = m_textNormalColor;
		if (currentState == ButtonState::DISABLED) {
			textColor = m_textDisabledColor;
		}
		else if (currentState == ButtonState::CLICKED) {
			textColor = m_textClickedColor;
		}
		else if (currentState == ButtonState::HOVER) {
			textColor = m_textHoverColor;
		}
		
		// Button의 스타일에서 layer와 sortKeyOffset 가져오기
		const ButtonStateStyle& style = m_buttonComp->GetStateStyle(currentState);
		
		TextStyle tstyle{
			textColor,
			style.layer,
			style.sortKeyOffset + 0.1f + static_cast<float>(currentState),
			width,
			height
		};
		m_textComp->ApplyStyle(tstyle);
	}
	
	// 상태 업데이트 (Render() 함수의 마지막에 수행)
	m_previousState = currentState;
	
	// 텍스트 렌더링 (항상 수행)
	if (m_textComp) {
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
	// UIButton 전용 정리 작업
	m_rectTransform = nullptr;
	m_image = nullptr;
	m_buttonComp = nullptr;
	m_textComp = nullptr;
	
	// 부모 클래스의 Release() 호출하여 컴포넌트 정리
	UIElement::Release();
}

void UIButton::SetDisabled(bool disabled)
{
	if (m_buttonComp) {
		m_buttonComp->SetDisabled(disabled);
	}
}

void UIButton::RenderDisabled()
{
	if (!m_image || !m_rectTransform || !m_buttonComp) return;
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

	// Disabled 상태의 색상 틴트 적용
	const ButtonStateStyle& disabledStyle = m_buttonComp->GetStateStyle(ButtonState::DISABLED);
	Gdiplus::Color tintColor = disabledStyle.spriteColor;
	bool hasTint = (tintColor.GetR() != 255 || tintColor.GetG() != 255 || tintColor.GetB() != 255 || tintColor.GetA() != 255);

	RenderManager::GetInstance()->RenderUIImageWithPivot(
		normalBitmap,
		x,
		y,
		width,
		height,
		pivotX,
		pivotY,
		LAYER_UI_FOREGROUND,
		static_cast<float>(ButtonState::DISABLED),  // 버튼 상태를 sortKey로 사용
		tintColor,
		hasTint
	);  
}
