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
			rectTransform->SetScale(scaleX, scaleY);
		}
	}
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
	
	// Button 상태 업데이트 (이 안에서 콜백이 호출될 수 있음)
	ButtonState previousState = m_buttonComp->GetState();
	m_buttonComp->UpdateState(m_rectTransform, m_image);
	
	// UpdateState 호출 후 객체가 삭제되었을 수 있으므로 다시 체크
	if (!m_buttonComp || !m_image) return;
	
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
	
	const ButtonStateStyle& disabledStyle = m_buttonComp->GetStateStyle(ButtonState::DISABLED);
	Gdiplus::Color tintColor = disabledStyle.spriteColor;
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
