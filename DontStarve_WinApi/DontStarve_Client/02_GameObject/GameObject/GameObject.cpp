#include "../../99_Default/pch.h"
#include "GameObject.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"


GameObject::GameObject(GameObjectType type, GameObjectID id, float x, float y, float pivotX, float pivotY, Direction dir, const std::wstring& resourcePath, const std::wstring& imageName)
	:m_type(type), m_id(id), m_x(x), m_y(y), m_pivotX(pivotX), m_pivotY(pivotY), m_direction(dir), m_width(0), m_height(0), resourcePath(resourcePath), imageName(imageName), m_animator(nullptr), m_isActive(true)
{
	// 단일 이미지 GameObject들을 위한 기본 비트맵 로드
	LoadBitmap();
	
	// 비트맵 크기 설정
	if (m_bitmap) {
		m_width = (float)m_bitmap->GetWidth();
		m_height = (float)m_bitmap->GetHeight();
	}
}

GameObject::~GameObject() { Release(); }

void GameObject::Init() {}
void GameObject::LateInit() {}
void GameObject::Update(float deltaTime) {}
void GameObject::LateUpdate() {}

void GameObject::Release() 
{ 
	SafeDelete(m_animator); 
	SafeDelete(m_bitmap);
}

void GameObject::OnInteraction(GameObject* obj)
{
}


Gdiplus::RectF GameObject::GetWorldBoundingBox()
{
	return Gdiplus::RectF(m_x - m_width * m_pivotX, m_y - m_height * m_pivotY, m_width, m_height);
}

// 기본 구현: 항상 m_bitmap 반환 (애니메이션 변경 시 업데이트됨)
Gdiplus::Bitmap* GameObject::GetBitmap() const
{
	return m_bitmap;
}

// 기본 LoadBitmap 구현
void GameObject::LoadBitmap()
{
	// 애니메이션이 있는 GameObject 타입들은 기본 비트맵 로드하지 않음
	if (m_type == GOBJ_PLAYER || m_type == GOBJ_NATURAL_ENVIR) {
		OutputDebugStringW((L"GameObject: LoadBitmap 스킵 - 애니메이션 사용 예정 (ID: " + std::to_wstring(m_id) + L", Type: " + std::to_wstring(m_type) + L")\n").c_str());
		return;
	}
	
	if (resourcePath.empty() || imageName.empty()) {
		OutputDebugStringW((L"GameObject: LoadBitmap 실패 - 경로나 이미지명이 비어있음 (ID: " + std::to_wstring(m_id) + L")\n").c_str());
		m_bitmap = nullptr;
		return;
	}
	
	// ResourceManager를 사용하여 경로 구성
	auto* pRM = ResourceManager::GetInstance();
	std::wstring fullPath = pRM->BuildObjectResourcePath(m_id, L"", imageName);
	
	OutputDebugStringW((L"GameObject: LoadBitmap - 전체 경로: " + fullPath + L"\n").c_str());
	
	// 비트맵 로드
	m_bitmap = new Gdiplus::Bitmap(fullPath.c_str());
	if (m_bitmap && m_bitmap->GetLastStatus() != Gdiplus::Ok) {
		OutputDebugStringW((L"GameObject: LoadBitmap 실패 - 비트맵 상태 오류 (ID: " + std::to_wstring(m_id) + L")\n").c_str());
		delete m_bitmap;
		m_bitmap = nullptr;
	} else if (m_bitmap) {
		OutputDebugStringW((L"GameObject: LoadBitmap 성공 - ID: " + std::to_wstring(m_id) + L"\n").c_str());
	} else {
		OutputDebugStringW((L"GameObject: LoadBitmap 실패 - 비트맵 생성 실패 (ID: " + std::to_wstring(m_id) + L")\n").c_str());
	}
}

bool GameObject::GetActive() const 
{ 
	return m_isActive; 
}

float GameObject::GetX() const { return m_x; }
float GameObject::GetY() const { return m_y; }
float GameObject::GetWidth() const {  return m_width;}
float GameObject::GetHeight() const {  return m_height; }
float GameObject::GetPivotX() const { return m_pivotX; }
float GameObject::GetPivotY() const { return m_pivotY; }
std::wstring GameObject::GetImageName() const { return imageName; }

GameObjectID GameObject::GetID() const { return m_id; }
GameObjectType GameObject::GetType() const { return m_type; }
const std::wstring& GameObject::GetName() const { return m_name; }
const std::wstring& GameObject::GetDescription() const { return m_description; }

Direction GameObject::GetDir() const { return m_direction; }

// 새로 추가된 메서드들
void GameObject::SetPivot(float pivotX, float pivotY) {
	m_pivotX = pivotX;
	m_pivotY = pivotY;
}

void GameObject::SetActive(bool active) {
	m_isActive = active;
}

void GameObject::SetPosition(float x, float y) {
	m_x = x;
	m_y = y;
}

