#include "../../99_Default/pch.h"
#include "CameraManager.h"
#include "../InputManager/InputManager.h" 
#include "../ObjectManager/ObjectManager.h"
#include "../ResourceManager/ResourceManager.h"
#include "../RenderManager/RenderManager.h"
#include "../../03_Animation/Animator.h"
#include "../../02_GameObject/GameObject.h"

CameraManager::CameraManager()
    : m_cameraPos({ 0,0 }), m_zoomFactor(1.0f), m_target(nullptr),
    m_followMode(true), m_viewportChanged(true),
    m_lastStartTileX(0), m_lastStartTileY(0), m_lastEndTileX(0), m_lastEndTileY(0),
    m_tileViewportChanged(true)
{
}

CameraManager::~CameraManager()
{
	Release();
}

void CameraManager::Init()
{
    m_cameraPos = { 0,0 }; 
    m_zoomFactor = 1.0f; 
    m_viewportChanged = true;
    m_visibleObjects.clear();
    m_visibleObjectSet.clear();
    
    m_tileCache.clear();
    m_visibleTileIndices.clear();
    m_tileViewportChanged = true;
}

void CameraManager::LateInit()
{

}

void CameraManager::Update(float deltaTime)
{
    if (!InputManager::GetInstance()) return;

    // �÷��̾� ���� ��尡 Ȱ��ȭ�Ǿ� ������ �÷��̾ ����
    if (m_followMode && m_target) {
        FollowTarget(deltaTime);
    }
    
    // ����Ʈ ���� ����
    CheckViewportChanged();
    
    // ī�޶� ����Ʈ �� ���� ������Ʈ�� �� ������ �����Ͽ�
    // ������Ʈ ����/����/�̵����� ��� ����
    UpdateVisibleObjects();
}

void CameraManager::LateUpdate()
{

}

void CameraManager::Render()
{

}

void CameraManager::Release()
{
	m_visibleObjects.clear();
	m_visibleObjectSet.clear();
	
	// Ÿ�� ĳ�� ����
	ClearTileCache();
}

// ���� ��ǥ�� ȭ�� �ȼ� ��ǥ�� ��ȯ
Gdiplus::PointF CameraManager::WorldToScreen(float worldX, float worldY)
{

    float transformedX = (worldX - m_cameraPos.X);
    float transformedY = (worldY - m_cameraPos.Y);

    transformedX += WINCX / 2.0f;
    transformedY += WINCY / 2.0f;

    return Gdiplus::PointF(transformedX, transformedY);
}

// ȭ�� �ȼ� ��ǥ�� ���� ��ǥ�� ��ȯ
Gdiplus::PointF CameraManager::ScreenToWorld(float screenX, float screenY) 
{
    float uncenteredScreenX = screenX - WINCX / 2.0f;
    float uncenteredScreenY = screenY - WINCY / 2.0f;

    // ī�޶� ������ ����
    float worldX = uncenteredScreenX + m_cameraPos.X;
    float worldY = uncenteredScreenY + m_cameraPos.Y;

    return Gdiplus::PointF(worldX, worldY);
}

Gdiplus::PointF CameraManager::GetCameraPos()
{
    return m_cameraPos;
}

// ����Ʈ�� ���� ��ǥ ������ ��ȯ
Gdiplus::RectF CameraManager::GetViewportWorldRect() const
{
    // ī�޶� ��ġ�� �߽����� �� ����Ʈ ���
    float halfWidth = WINCX / 2.0f;
    float halfHeight = WINCY / 2.0f;
    
    float left = m_cameraPos.X - halfWidth;
    float top = m_cameraPos.Y - halfHeight;
    float right = m_cameraPos.X + halfWidth;
    float bottom = m_cameraPos.Y + halfHeight;
    
    return Gdiplus::RectF(
        left,
        top,
        right - left,
        bottom - top
    );
}


void CameraManager::SetTarget(GameObject* target)
{
    m_target = target;
}

const GameObject* CameraManager::GetTarget()
{
    return m_target;
}

void CameraManager::FollowTarget(float deltaTime)
{
	if (!m_target)
		return;
	// �÷��̾��� (x, y)�� �� �� �߾��̸�, �״�� ���
	m_cameraPos.X = m_target->GetX();
	m_cameraPos.Y = m_target->GetY();
}

void CameraManager::SetCameraPosition(float x, float y)
{
    m_cameraPos.X = x;
    m_cameraPos.Y = y;
}

// === ȭ�鿡 ���̴� ������Ʈ ���� ��� ===

void CameraManager::UpdateVisibleObjects()
{
	m_visibleObjects.clear();
	m_visibleObjectSet.clear();
	
	// ObjectManager���� ��� ������Ʈ ��������
	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (!objectManager) return;
	
	const std::vector<GameObject*>& allObjects = objectManager->GetGameObjects();
	if (allObjects.empty()) return;
	
	// ���� ����Ʈ ���� ��������
	Gdiplus::RectF viewportRect = GetViewportWorldRect();
	
	// ���� ������ ������ �˻� ���� (������Ʈ ũ��� �ִϸ��̼� ������ ���)
	const float MARGIN = 200.0f;
	float startX = viewportRect.X - MARGIN;
	float endX = viewportRect.X + viewportRect.Width + MARGIN;
	float startY = viewportRect.Y - MARGIN;
	float endY = viewportRect.Y + viewportRect.Height + MARGIN;
	
	// �ߺ� üũ�� ���� �ӽ� set (������ �ߺ� ����)
	std::unordered_set<std::wstring> addedIngredients;
	
	// Ȱ��ȭ�� ������Ʈ �߿��� ȭ�鿡 ���̴� �͸� ���͸�
	for (GameObject* obj : allObjects) {
		if (!obj || !obj->GetActive()) {
			continue;
		}
		
		// ������ ������ ������Ʈ���� Ȯ��
		// Player�� ������ Animator�� ����� �� �����Ƿ� �� �� Ȯ��
		if (!obj->GetBitmap() && !obj->GetComponent<Animator>()) {
			continue;
		}
		
		// Ingredient�� ��� �ߺ� üũ
		if (obj->GetType() == GOBJ_ITEM) {
			std::wstring ingredientKey = std::to_wstring(obj->GetID()) + L"_" + 
				std::to_wstring(obj->GetX()) + L"_" + std::to_wstring(obj->GetY());
			if (addedIngredients.find(ingredientKey) != addedIngredients.end()) {
				continue;
			}
			addedIngredients.insert(ingredientKey);
		}
		
		// ������Ʈ�� ���� �ٿ�� �ڽ� ���
		Gdiplus::RectF objBounds = obj->GetWorldBoundingBox();
		
		// ȭ�� ������ ��ġ���� Ȯ�� (AABB �浹 �˻�)
		if (objBounds.X < endX && objBounds.X + objBounds.Width > startX &&
			objBounds.Y < endY && objBounds.Y + objBounds.Height > startY) {
			
			m_visibleObjects.push_back(obj);
			m_visibleObjectSet.insert(obj);
		}
	}
	
	// �����: ���� ������Ʈ ������Ʈ Ȯ�� (30�����Ӹ��� �� ���� ���)
	static int updateCounter = 0;
	if (++updateCounter % 30 == 0) {
		OutputDebugStringW((L"[CameraManager] ���� ������Ʈ ������Ʈ: " + 
			std::to_wstring(m_visibleObjects.size()) + L"�� (ī�޶�: " + 
			std::to_wstring((int)m_cameraPos.X) + L", " + std::to_wstring((int)m_cameraPos.Y) + L")\n").c_str());
	}
}

GameObject* CameraManager::FindObjectAtPosition(float worldX, float worldY)
{
	
	// ȭ�鿡 ���̴� ������Ʈ�� �˻� (���� ����ȭ)
	for (int i = (int)m_visibleObjects.size() - 1; i >= 0; --i) {
		GameObject* obj = m_visibleObjects[i];
		if (!obj) {
			continue;
		}
		
		// ��ȣ�ۿ� ������ ������Ʈ���� ���� Ȯ��
		if (!obj->CanInteract()) {
			continue;
		}
		
		// ������Ʈ�� ���� �ٿ�� �ڽ� ���
		Gdiplus::RectF objBounds = obj->GetWorldBoundingBox();
		
		// Ŭ���� ��ġ�� ������Ʈ ���� �ȿ� �ִ��� Ȯ��
		if (objBounds.Contains(worldX, worldY)) {
			return obj;
		}
	}
	
	return nullptr;
}

bool CameraManager::IsObjectVisible(GameObject* obj) const
{
	return m_visibleObjectSet.find(obj) != m_visibleObjectSet.end();
}

bool CameraManager::IsObjectInViewport(GameObject* obj) const
{
	if (!obj || !obj->GetActive()) return false;
	
	// ������Ʈ�� ���� �ٿ�� �ڽ� ���
	Gdiplus::RectF objBounds = obj->GetWorldBoundingBox();
	
	// ���� ����Ʈ ���� ��������
	Gdiplus::RectF viewportRect = GetViewportWorldRect();
	
	// ���� ������ ������ �˻� ����
	const float MARGIN = 200.0f;
	float startX = viewportRect.X - MARGIN;
	float endX = viewportRect.X + viewportRect.Width + MARGIN;
	float startY = viewportRect.Y - MARGIN;
	float endY = viewportRect.Y + viewportRect.Height + MARGIN;
	
	// ȭ�� ������ ��ġ���� Ȯ��
	return (objBounds.X < endX && objBounds.X + objBounds.Width > startX &&
			objBounds.Y < endY && objBounds.Y + objBounds.Height > startY);
}

void CameraManager::CheckViewportChanged()
{
	Gdiplus::RectF currentViewport = GetViewportWorldRect();
	
	// ����Ʈ�� ����Ǿ����� Ȯ�� (ī�޶� ��ġ�� ũ�Ⱑ ����Ǿ��� ��)
	if (currentViewport.X != m_lastViewportRect.X || 
		currentViewport.Y != m_lastViewportRect.Y ||
		currentViewport.Width != m_lastViewportRect.Width || 
		currentViewport.Height != m_lastViewportRect.Height) {
		
		m_viewportChanged = true;
		m_lastViewportRect = currentViewport;
	}
}

// === Ÿ�� ������ ���� ��� ===

void CameraManager::RenderVisibleTiles(RenderManager* renderManager, const MapData* mapData)
{
	if (!mapData || !renderManager) {
		return;
	}

	// ���� CheckViewportChanged() �Լ��� ����� ���
	// Update()���� �̹� ȣ��Ǿ� m_viewportChanged�� ������
	if (m_viewportChanged) {
		m_tileViewportChanged = true;
		m_viewportChanged = false; // Ÿ�� �������� ���� �� ��� Ŭ����
	}

	// ����Ʈ�� ������� �ʾ����� ���� ���� ���
	if (!m_tileViewportChanged) {
		// ������ ���� ������ ������
		for (int y = m_lastStartTileY; y < m_lastEndTileY; ++y) {
			float worldY = y * TILE_SIZE + TILE_SIZE / 2.0f;
			for (int x = m_lastStartTileX; x < m_lastEndTileX; ++x) {
				RenderSingleTile(renderManager, mapData, x, y, worldY);
			}
		}
		return;
	}

	// ����Ʈ�� ����Ǿ��� ���� ���ο� ���� ���
	Gdiplus::RectF currentViewport = GetViewportWorldRect();
	const float MARGIN = TILE_SIZE;
	float startX = currentViewport.X - MARGIN;
	float endX = currentViewport.X + currentViewport.Width + MARGIN;
	float startY = currentViewport.Y - MARGIN;
	float endY = currentViewport.Y + currentViewport.Height + MARGIN;

	// Ÿ�� �ε��� ���� ���
	int startTileX = max(0, (int)floor(startX / TILE_SIZE));
	int endTileX = min(MAP_WIDTH, (int)ceil(endX / TILE_SIZE));
	int startTileY = max(0, (int)floor(startY / TILE_SIZE));
	int endTileY = min(MAP_HEIGHT, (int)ceil(endY / TILE_SIZE));

	// ���� ������Ʈ
	m_lastStartTileX = startTileX;
	m_lastStartTileY = startTileY;
	m_lastEndTileX = endTileX;
	m_lastEndTileY = endTileY;
	m_tileViewportChanged = false;

	// ����ȭ�� Ÿ�� ������
	for (int y = startTileY; y < endTileY; ++y) {
		float worldY = y * TILE_SIZE + TILE_SIZE / 2.0f;
		for (int x = startTileX; x < endTileX; ++x) {
			RenderSingleTile(renderManager, mapData, x, y, worldY);
		}
	}
}

// ���� Ÿ�� ������ ���� �Լ�
void CameraManager::RenderSingleTile(RenderManager* renderManager, const MapData* mapData, int x, int y, float worldY)
{
	const TileData& tileData = mapData->tiles[x][y];
	
	// �� Ÿ���̳� TILE_NONE�� Ÿ�� ��ŵ
	if (tileData.id == TILEID_NONE || tileData.type == TILE_NONE) {
		return;
	}

	// Ÿ�� ĳ�ÿ��� ������ �������� (����ȭ�� �˻�)
	auto cacheIt = m_tileCache.find(tileData.id);
	if (cacheIt == m_tileCache.end()) {
		// ĳ�ÿ� ���� Ÿ�ϸ� �ε�
		TileCacheData newCacheData;
		newCacheData.id = tileData.id;
		LoadTileBitmap(tileData.id, newCacheData);
		if (newCacheData.bitmap) {
			m_tileCache[tileData.id] = newCacheData;
			cacheIt = m_tileCache.find(tileData.id);
		}
		else {
			return; // �ε� ������ Ÿ���� ��ŵ
		}
	}

	TileCacheData& cacheData = cacheIt->second;
	Gdiplus::Bitmap* tileBitmap = cacheData.bitmap;

	// ��Ʈ���� �ε���� ���� ��츸 �ε�
	if (!tileBitmap) {
		LoadTileBitmap(tileData.id, cacheData);
		tileBitmap = cacheData.bitmap;
		if (!tileBitmap) return;
	}

	// Ÿ�� ������
	float worldX = x * TILE_SIZE + TILE_SIZE / 2.0f;
	renderManager->RenderTile(tileBitmap, worldX, worldY, TILE_SIZE, TILE_SIZE);
}

void CameraManager::ClearTileCache()
{
	// Ÿ�� ĳ���� ��Ʈ�ʵ� ����
	for (auto& pair : m_tileCache) {
		if (pair.second.bitmap) {
			delete pair.second.bitmap;
			pair.second.bitmap = nullptr;
		}
	}
	m_tileCache.clear();
	m_visibleTileIndices.clear();
}

void CameraManager::LoadTileBitmap(TileID tileID, TileCacheData& cacheData)
{
	ResourceManager* resourceManager = ResourceManager::GetInstance();
	const TileData* resourceTile = resourceManager->GetTileResourceInfo(tileID);
	if (resourceTile) {
		// TileData�� tileImageName�� ����Ͽ� ��� ����
		std::wstring fullPath = resourceManager->BuildTileResourcePath(tileID, L"", resourceTile->tileImageName);
		
		cacheData.bitmap = new Gdiplus::Bitmap(fullPath.c_str());
		if (cacheData.bitmap && cacheData.bitmap->GetLastStatus() == Gdiplus::Ok) {
			cacheData.isAtlasBased = false;
		}
		else {
			delete cacheData.bitmap;
			cacheData.bitmap = nullptr;
		}
	}
}