#include "99_Default/pch.h"
#include "UIButton.h"
#include "../Component/Transform/RectTransform.h"
#include "../Component/Sprite/Image.h"
#include "../Component/Sprite/Sprite.h"
#include "../Component/Button/Button.h"
#include "../../01_Manager/InputManager/InputManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../01_Manager/RenderManager/RenderManager.h"

UIButton::UIButton(GameObjectID id, float width, float height,
	const std::wstring& normalImagePath, const std::wstring& hoverImagePath,
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
	RectTransform* rectTransform = m_rectTransform;
	rectTransform->SetAnchorMin(anchorMinX, anchorMinY);
	rectTransform->SetAnchorMax(anchorMaxX, anchorMaxY);
	rectTransform->SetAnchoredPosition(anchoredPosX, anchoredPosY);
	rectTransform->SetPivot(0.5f, 0.5f);
	
	// 기본 크기 설정 (스케일은 1.0으로 유지)
	rectTransform->SetSize(width, height);
	rectTransform->SetScale(1.0f, 1.0f);
	
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
	
	// 스프라이트가 있으면 스프라이트 크기에 맞춰 scale 조정
	if (m_image && m_image->GetSprite()) {
		Gdiplus::Bitmap* bitmap = m_image->GetSprite();
		float bitmapWidth = static_cast<float>(bitmap->GetWidth());
		float bitmapHeight = static_cast<float>(bitmap->GetHeight());
		if (bitmapWidth > 0 && bitmapHeight > 0) {
			// 목표 크기(width, height)에 맞추기 위한 스케일 계산
			float scaleX = width / bitmapWidth;
			float scaleY = height / bitmapHeight;
			rectTransform->SetScale(scaleX, scaleY);
			// 스프라이트 원본 크기로 width/height 업데이트
			rectTransform->SetSize(bitmapWidth, bitmapHeight);
		}
	}
	// 스프라이트가 없는 경우는 이미 width/height가 설정되어 있음
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
	// 빈 경로인 경우 스프라이트를 로드하지 않음 (투명 버튼)

	// Hover 비트맵 로드 및 저장
	if (!hoverImagePath.empty()) {
		m_hoverSprite = ResourceManager::GetInstance()->LoadSprite(hoverImagePath);
	}
	// 빈 경로인 경우 hover 스프라이트를 로드하지 않음
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
