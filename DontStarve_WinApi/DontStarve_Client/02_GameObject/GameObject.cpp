#include "../99_Default/pch.h"
#include "GameObject.h"
#include "../01_Manager/ResourceManager/ResourceManager.h"

GameObject::GameObject(GameObjectType type, GameObjectID id, float x, float y, float pivotX, float pivotY, Direction dir, const std::wstring& resourcePath, const std::wstring& imageName)
	:m_type(type), m_id(id), m_x(x), m_y(y), m_pivotX(pivotX), m_pivotY(pivotY), m_direction(dir), m_width(0), m_height(0), resourcePath(resourcePath), imageName(imageName), m_isActive(true), m_layer(LAYER_WORLD_OBJECT)
{
	// 기본 이미지 GameObject에서만 기본 비트맵 로드
	LoadBitmap();
	
	// 비트맵 크기 설정
	if (m_bitmap) {
		m_width = (float)m_bitmap->GetWidth();
		m_height = (float)m_bitmap->GetHeight();
	}
}

GameObject::~GameObject() { Release(); }

void GameObject::Init() {
	for (auto& component : m_components) {
		component->Init();
	}
}

void GameObject::LateInit() {
	for (auto& component : m_components) {
		component->LateInit();
	}
}

void GameObject::Update(float deltaTime) {
	for (auto& component : m_components) {
		if (component->IsEnabled()) {
			component->Update(deltaTime);
		}
	}
}

void GameObject::LateUpdate() {
	for (auto& component : m_components) {
		if (component->IsEnabled()) {
			component->LateUpdate();
		}
	}
}

void GameObject::Release() 
{ 
	// 컴포넌트 해제
	for (auto& component : m_components) {
		component->Release();
		SafeDelete(component);
	}
	m_components.clear();

	SafeDelete(m_bitmap);
}

void GameObject::OnInteraction(GameObject* obj)
{
}

// 기본 LoadBitmap 구현 확인
void GameObject::LoadBitmap()
{
	// 애니메이션을 사용하는 GameObject 타입에서는 기본 비트맵 로드를 건너뜀
	if (m_type == GOBJ_PLAYER || m_type == GOBJ_NATURAL_ENVIR) {
		OutputDebugStringW((L"GameObject: LoadBitmap 건너뜀 - 애니메이션 사용 객체 (ID: " + std::to_wstring(m_id) + L", Type: " + std::to_wstring(m_type) + L")\n").c_str());
		return;
	}
	
	if (resourcePath.empty() || imageName.empty()) {
		OutputDebugStringW((L"GameObject: LoadBitmap 실패 - 경로나 이미지명이 비어있음 (ID: " + std::to_wstring(m_id) + L")\n").c_str());
		m_bitmap = nullptr;
		return;
	}
	
	std::wstring fullPath = ResourceManager::GetInstance()->BuildObjectResourcePath(m_id, L"", imageName);
	
	OutputDebugStringW((L"GameObject: LoadBitmap - 전체 경로: " + fullPath + L"\n").c_str());
	
	// 비트맵 로드
	m_bitmap = new Gdiplus::Bitmap(fullPath.c_str());
	if (m_bitmap && m_bitmap->GetLastStatus() != Gdiplus::Ok) {
		OutputDebugStringW((L"GameObject: LoadBitmap 실패 - 비트맵 파일 로드 실패 (ID: " + std::to_wstring(m_id) + L")\n").c_str());
		delete m_bitmap;
		m_bitmap = nullptr;
	} else if (m_bitmap) {
		OutputDebugStringW((L"GameObject: LoadBitmap 성공 - ID: " + std::to_wstring(m_id) + L"\n").c_str());
	} else {
		OutputDebugStringW((L"GameObject: LoadBitmap 실패 - 비트맵 생성 실패 (ID: " + std::to_wstring(m_id) + L")\n").c_str());
	}
}

