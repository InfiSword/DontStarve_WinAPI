#include "99_Default/pch.h"
#include "UIButton.h"
#include "../../01_Manager/InputManager/InputManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../01_Manager/RenderManager/RenderManager.h"
#include "../Component/Transform/RectTransform.h"
#include "../Component/Sprite/Image.h"
#include "../Component/Sprite/Sprite.h"
#include "../Component/Button/Button.h"

UIButton::UIButton(GameObjectID id, float width, float height,
	const std::shared_ptr<Sprite>& normalSprite, const std::shared_ptr<Sprite>& hoverSprite,
	float anchorMinX, float anchorMinY, float anchorMaxX, float anchorMaxY,
	float anchoredPosX, float anchoredPosY)
	: UIElement(GOBJ_UI, id, L"", L"", true, false),
	m_image(nullptr),
	m_buttonComp(nullptr),
	m_normalSprite(nullptr),
	m_hoverSprite(nullptr),
	m_previousState(ButtonState::NORMAL)
{
	// UIElement에서 이미 RectTransform이 생성되었으므로 GetRectTransform() 사용
	RectTransform* rectTransform = GetRectTransform();
	if (!rectTransform) {
		OutputDebugStringW(L"UIButton: RectTransform이 nullptr입니다!\n");
		return;
	}
	
	rectTransform->SetAnchorMin(anchorMinX, anchorMinY);
	rectTransform->SetAnchorMax(anchorMaxX, anchorMaxY);
	rectTransform->SetAnchoredPosition(anchoredPosX, anchoredPosY);
	rectTransform->SetPivot(0.5f, 0.5f);
	
	// 기본 크기 설정 (스케일은 1.0으로 유지)
	rectTransform->SetSize(width, height);
	rectTransform->SetScale(1.0f, 1.0f);
	
	// Image 컴포넌트 추가 (생성자에서 초기화)
	m_image = AddComponent<ComponentElement::Image>();
	if (!m_image) {
		OutputDebugStringW(L"UIButton: Image 컴포넌트 생성 실패!\n");
		return;
	}
	m_image->SetLayer(LAYER_UI_FOREGROUND);
	m_image->SetSortKey(0.0f);

	// 스프라이트 핸들 저장
	m_normalSprite = normalSprite;
	m_hoverSprite = hoverSprite;

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
	if (!m_buttonComp) {
		OutputDebugStringW(L"UIButton: Button 컴포넌트 생성 실패!\n");
		return;
	}
	
	// normal 스프라이트가 있으면 Image 컴포넌트에 설정
	if (m_normalSprite) {
		m_image->SetSprite(m_normalSprite);
	}
	
	// 스프라이트가 있으면 목표 크기(width, height)에 맞춰 스케일·크기 설정 (생성 시에는 그대로 채움)
	Gdiplus::Bitmap* bitmap = m_image->GetSprite();
	if (bitmap) {
		float bitmapWidth = static_cast<float>(bitmap->GetWidth());
		float bitmapHeight = static_cast<float>(bitmap->GetHeight());
		if (bitmapWidth > 0 && bitmapHeight > 0) {
			float scaleX = width / bitmapWidth;
			float scaleY = height / bitmapHeight;
			rectTransform->SetScale(scaleX, scaleY);
			rectTransform->SetSize(bitmapWidth, bitmapHeight);
		}
	}
	// 스프라이트가 없는 경우는 이미 width/height가 설정되어 있음
}

UIButton::~UIButton()
{
	Release();
}

void UIButton::Update(float deltaTime)
{
	// 먼저 필수 컴포넌트 체크
	if (!m_buttonComp || !m_image) return;
	
	if (!m_rectTransform) return;
	
	// Image 컴포넌트만 업데이트 (Button은 수동으로 UpdateState 호출)
	if (m_image && m_image->IsEnabled()) {
		m_image->Update(deltaTime);
	}
	
	// Button 상태 업데이트 (이 안에서 클릭 콜백이 호출될 수 있음 → 씬 전환 시 이 UI가 파괴됨)
	ButtonState previousState = m_buttonComp->GetState();
	bool callbackInvoked = m_buttonComp->UpdateState(m_rectTransform, m_image);
	if (callbackInvoked) return; // 콜백에서 씬 전환 등으로 이 객체가 파괴되었을 수 있음. 역참조 금지.
	
	if (!m_buttonComp || !m_image) return;
	
	ButtonState currentState = m_buttonComp->GetState();
	
	// 상태가 변경되었을 때 스프라이트 변경 (hover callback은 Button에서 처리)
	if (previousState != currentState) {
		if (currentState == ButtonState::HOVER && m_hoverSprite) {
			m_image->SetSprite(m_hoverSprite);
		}
		else if (m_normalSprite) {
			m_image->SetSprite(m_normalSprite);
		}
	}
	
	// 상태 업데이트 (Update() 함수의 마지막에 수행)
	m_previousState = currentState;
}

void UIButton::Render()
{
	if (!IsEnabled() || !m_image || !m_buttonComp) return;
	
	RectTransform* rectTransform = GetRectTransform();
	if (!rectTransform) return;
	
	ButtonState currentState = m_buttonComp->GetState();
	
	// 비활성화된 버튼은 비활성화 스타일로 렌더링
	if (m_buttonComp->IsDisabled()) {
		RenderDisabled();
		return;
	}

	// 스프라이트가 없으면 렌더링 불가
	Gdiplus::Bitmap* bitmap = m_image->GetSprite();
	if (!bitmap) {
		return;
	}

	float x = rectTransform->GetX();
	float y = rectTransform->GetY();
	
	// 비트맵 크기와 스케일 계산
	float bitmapWidth = static_cast<float>(bitmap->GetWidth());
	float bitmapHeight = static_cast<float>(bitmap->GetHeight());
	float width = bitmapWidth * rectTransform->GetScaleX();
	float height = bitmapHeight * rectTransform->GetScaleY();

	// Image 컴포넌트의 틴트 색상 사용 (Button의 ApplyVisualState에서 설정됨)
	Gdiplus::Color tintColor = m_image->GetTintColor();
	bool hasTint = (tintColor.GetA() != 255 || tintColor.GetR() != 255 || tintColor.GetG() != 255 || tintColor.GetB() != 255);

	RenderManager::GetInstance()->RenderUIImageWithPivot(
		bitmap,
		x,
		y,
		width,
		height,
		rectTransform->GetPivotX(),
		rectTransform->GetPivotY(),
		m_image->GetLayer(),
		m_image->GetSortKey(),
		tintColor,
		hasTint
	);
}

void UIButton::RenderDisabled()
{
	if (!m_image) return;
	
	if (!m_rectTransform) return;
	
	Gdiplus::Bitmap* bitmap = m_image->GetSprite();
	if (!bitmap) return;
	
	float x = m_rectTransform->GetX();
	float y = m_rectTransform->GetY();
	float bitmapWidth = static_cast<float>(bitmap->GetWidth());
	float bitmapHeight = static_cast<float>(bitmap->GetHeight());
	float width = bitmapWidth * m_rectTransform->GetScaleX();
	float height = bitmapHeight * m_rectTransform->GetScaleY();
	
	// Image 컴포넌트의 틴트 색상 사용 (Button의 ApplyVisualState에서 설정됨)
	Gdiplus::Color tintColor = m_image->GetTintColor();
	bool hasTint = (tintColor.GetA() != 255 || tintColor.GetR() != 255 || tintColor.GetG() != 255 || tintColor.GetB() != 255);
	
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

void UIButton::SetDisabled(bool disabled)
{
	if (m_buttonComp) {
		m_buttonComp->SetDisabled(disabled);
	}
}

ButtonState UIButton::GetButtonState() const
{
	return m_buttonComp ? m_buttonComp->GetState() : ButtonState::NORMAL;
}

void UIButton::SetHoverColor(const Gdiplus::Color& color)
{
	if (m_buttonComp) {
		m_buttonComp->SetHoverColor(color);
	}
}

void UIButton::SetNormalColor(const Gdiplus::Color& color)
{
	if (m_buttonComp) {
		m_buttonComp->SetNormalColor(color);
	}
}

void UIButton::SetClickedColor(const Gdiplus::Color& color)
{
	if (m_buttonComp) {
		m_buttonComp->SetClickedColor(color);
	}
}

void UIButton::SetDisabledColor(const Gdiplus::Color& color)
{
	if (m_buttonComp) {
		m_buttonComp->SetDisabledColor(color);
	}
}

void UIButton::UpdateHoverStateImmediate()
{
	// 즉시 UpdateState 호출 (hover 및 모든 상태 처리)
	// 컴포넌트가 유효한지 확인 (삭제 중일 수 있음)
	if (!m_buttonComp || !m_image || !m_rectTransform) return;
	
	ButtonState previousState = m_buttonComp->GetState();
	
	// UpdateState 호출 (hover, 클릭 등 모든 상태 처리)
	bool callbackInvoked = m_buttonComp->UpdateState(m_rectTransform, m_image);
	
	// 콜백에서 객체가 삭제되었을 수 있으므로 체크
	if (callbackInvoked || !m_buttonComp || !m_image) return;
	
	ButtonState currentState = m_buttonComp->GetState();
	
	// 상태가 변경되었을 때 스프라이트 변경
	if (previousState != currentState) {
		if (currentState == ButtonState::HOVER && m_hoverSprite) {
			m_image->SetSprite(m_hoverSprite);
		}
		else if (m_normalSprite) {
			m_image->SetSprite(m_normalSprite);
		}
	}
}

void UIButton::Release()
{
	// UIButton 전용 정리 작업
	m_image = nullptr;
	m_buttonComp = nullptr;
	m_normalSprite = nullptr;
	m_hoverSprite = nullptr;
	
	// 부모 클래스의 Release() 호출하여 컴포넌트 정리
	UIElement::Release();
}
