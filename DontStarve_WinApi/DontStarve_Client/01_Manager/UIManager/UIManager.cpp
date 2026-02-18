#include "99_Default/pch.h"
#include "UIManager.h"
#include "../../02_GameObject/UI/UIElement.h"
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
	for (auto* element : m_uiElements) {
		if (element && element->IsEnabled()) {
			element->Update(deltaTime);
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

	for (auto* element : m_uiElements) {
		if (element && element->IsEnabled()) {
			element->Render();
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
		auto it = std::find(m_uiElements.begin(), m_uiElements.end(), static_cast<UIElement*>(image));
		if (it == m_uiElements.end()) {
			m_uiElements.push_back(image);
		}
	}
}

void UIManager::AddUIButton(UIButton* button)
{
	if (button) {
		auto it = std::find(m_uiElements.begin(), m_uiElements.end(), static_cast<UIElement*>(button));
		if (it == m_uiElements.end()) {
			m_uiElements.push_back(button);
		}
	}
}

void UIManager::AddUIText(UIText* text)
{
	if (text) {
		auto it = std::find(m_uiElements.begin(), m_uiElements.end(), static_cast<UIElement*>(text));
		if (it == m_uiElements.end()) {
			m_uiElements.push_back(text);
		}
	}
}

void UIManager::RemoveUIImage(UIImage* image)
{
	if (!image) return;
	auto it = std::find(m_uiElements.begin(), m_uiElements.end(), static_cast<UIElement*>(image));
	if (it != m_uiElements.end()) {
		m_uiElements.erase(it);
	}
}

void UIManager::RemoveUIButton(UIButton* button)
{
	if (!button) return;
	auto it = std::find(m_uiElements.begin(), m_uiElements.end(), static_cast<UIElement*>(button));
	if (it != m_uiElements.end()) {
		m_uiElements.erase(it);
	}
}

void UIManager::RemoveUIText(UIText* text)
{
	if (!text) return;
	auto it = std::find(m_uiElements.begin(), m_uiElements.end(), static_cast<UIElement*>(text));
	if (it != m_uiElements.end()) {
		m_uiElements.erase(it);
	}
}

void UIManager::ClearAllUI()
{
	for (auto* element : m_uiElements) {
		if (element) {
			element->Release();
			delete element;
		}
	}
	m_uiElements.clear();
	m_uiElements.shrink_to_fit();
}

UIImage* UIManager::FindUIImage(GameObjectID id)
{
	for (auto* element : m_uiElements) {
		if (element && element->GetID() == id) {
			UIImage* img = dynamic_cast<UIImage*>(element);
			if (img) return img;
		}
	}
	return nullptr;
}

UIButton* UIManager::FindUIButton(GameObjectID id)
{
	for (auto* element : m_uiElements) {
		if (element && element->GetID() == id) {
			UIButton* btn = dynamic_cast<UIButton*>(element);
			if (btn) return btn;
		}
	}
	return nullptr;
}

UIText* UIManager::FindUIText(GameObjectID id)
{
	for (auto* element : m_uiElements) {
		if (element && element->GetID() == id) {
			UIText* txt = dynamic_cast<UIText*>(element);
			if (txt) return txt;
		}
	}
	return nullptr;
}

void UIManager::SetUIVisibility(bool visible)
{
	m_isUIVisible = visible;
}
