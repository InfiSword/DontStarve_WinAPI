#include "99_Default/pch.h"
#include "UIManager.h"
#include "../../02_GameObject/UI/UIImage.h"
#include "../../02_GameObject/UI/UIButton.h"
#include "../../02_GameObject/UI/UIText.h"
#include "../RenderManager/RenderManager.h"
#include "../InputManager/InputManager.h"

UIManager::UIManager() : m_isUIVisible(true)
{
}

UIManager::~UIManager()
{
	Release();
}

void UIManager::Init()
{
	ClearAllUI();
}

void UIManager::LateInit()
{
}

void UIManager::Update(float deltaTime)
{
	for (size_t i = 0; i < m_uiImages.size(); ++i) {
		auto* image = m_uiImages[i];
		if (image && image->IsEnabled()) {
			image->Update(deltaTime);
		}
	}

	for (size_t i = 0; i < m_uiButtons.size(); ++i) {
		auto* button = m_uiButtons[i];
		if (button && button->IsEnabled()) {
			button->Update(deltaTime);
		}
	}

	for (size_t i = 0; i < m_uiTexts.size(); ++i) {
		auto* text = m_uiTexts[i];
		if (text && text->IsEnabled()) {
			text->Update(deltaTime);
		}
	}
}

void UIManager::LateUpdate()
{
}

void UIManager::Render()
{
	RenderManager* renderManager = RenderManager::GetInstance();
	if (!renderManager) return;

	// 디버그: 렌더링할 UI 개수 출력
	static int frameCount = 0;
	if (frameCount++ % 60 == 0) {  // 60프레임마다 한 번씩 출력
		OutputDebugStringW((L"UIManager::Render - Images: " + std::to_wstring(m_uiImages.size()) +
			L", Buttons: " + std::to_wstring(m_uiButtons.size()) +
			L", Texts: " + std::to_wstring(m_uiTexts.size()) + L"\n").c_str());
	}

	for (auto* image : m_uiImages) {
		if (image && image->IsEnabled()) {
			image->Render();
		}
	}

	for (auto* button : m_uiButtons) {
		if (button && button->IsEnabled()) {
			button->Render();
		}
	}

	for (auto* text : m_uiTexts) {
		if (text && text->IsEnabled()) {
			text->Render();
		}
	}
}

void UIManager::Release()
{
	ClearAllUI();
}

void UIManager::AddUIImage(UIImage* image)
{
	if (image) {
		auto it = std::find(m_uiImages.begin(), m_uiImages.end(), image);
		if (it == m_uiImages.end()) {
			m_uiImages.push_back(image);
		}
	}
}

void UIManager::AddUIButton(UIButton* button)
{
	if (button) {
		auto it = std::find(m_uiButtons.begin(), m_uiButtons.end(), button);
		if (it == m_uiButtons.end()) {
			m_uiButtons.push_back(button);
		}
	}
}

void UIManager::AddUIText(UIText* text)
{
	if (text) {
		auto it = std::find(m_uiTexts.begin(), m_uiTexts.end(), text);
		if (it == m_uiTexts.end()) {
			m_uiTexts.push_back(text);
		}
	}
}

void UIManager::RemoveUIImage(UIImage* image)
{
	if (!image) return;

	auto it = std::find(m_uiImages.begin(), m_uiImages.end(), image);
	if (it != m_uiImages.end()) {
		m_uiImages.erase(it);
	}
}

void UIManager::RemoveUIButton(UIButton* button)
{
	if (!button) return;

	auto it = std::find(m_uiButtons.begin(), m_uiButtons.end(), button);
	if (it != m_uiButtons.end()) {
		m_uiButtons.erase(it);
	}
}

void UIManager::RemoveUIText(UIText* text)
{
	if (!text) return;

	auto it = std::find(m_uiTexts.begin(), m_uiTexts.end(), text);
	if (it != m_uiTexts.end()) {
		m_uiTexts.erase(it);
	}
}

void UIManager::ClearAllUI()
{
	for (auto* image : m_uiImages) {
		if (image) {
			image->Release();
			delete image;
		}
	}
	m_uiImages.clear();

	for (auto* button : m_uiButtons) {
		if (button) {
			button->Release();
			delete button;
		}
	}
	m_uiButtons.clear();

	for (auto* text : m_uiTexts) {
		if (text) {
			text->Release();
			delete text;
		}
	}
	m_uiTexts.clear();
}

UIImage* UIManager::FindUIImage(GameObjectID id)
{
	for (auto* image : m_uiImages) {
		if (image && image->GetID() == id) {
			return image;
		}
	}
	return nullptr;
}

UIButton* UIManager::FindUIButton(GameObjectID id)
{
	for (auto* button : m_uiButtons) {
		if (button && button->GetID() == id) {
			return button;
		}
	}
	return nullptr;
}

UIText* UIManager::FindUIText(GameObjectID id)
{
	for (auto* text : m_uiTexts) {
		if (text && text->GetID() == id) {
			return text;
		}
	}
	return nullptr;
}

void UIManager::SetUIVisibility(bool visible)
{
	m_isUIVisible = visible;
}
