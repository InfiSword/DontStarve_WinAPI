#include "../../99_Default/pch.h"
#include "UIManager.h"
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
}

void UIManager::LateUpdate()
{
}

void UIManager::Render()
{
	RenderManager* renderManager = RenderManager::GetInstance();
	if (!renderManager) return;


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

void UIManager::SetUIVisibility(bool visible)
{
	m_isUIVisible = visible;
}