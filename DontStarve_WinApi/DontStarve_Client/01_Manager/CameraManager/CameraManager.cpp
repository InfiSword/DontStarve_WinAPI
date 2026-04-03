#include "99_Default/pch.h"
#include "CameraManager.h"
#include "../../02_GameObject/Component/Transform/Transform.h"
#include "../../02_GameObject/Component/Sprite/SpriteRenderer.h"
#include "../../02_GameObject/Component/Collider/Collider.h"
#include "../../03_Animation/Animator.h"
#include "../../02_GameObject/GameObject.h"
#include "../ObjectManager/ObjectManager.h"
#include "../RenderManager/RenderManager.h"
#include "../ColliderManager/ColliderManager.h"

CameraManager::CameraManager() {}

void CameraManager::Init() {
    m_cameraPos = { 0,0 };
    m_visibleObjects.clear();
	m_queryBuffer.clear();
    ClearTileCache();
    m_hasWalkableBounds = false;
    m_lastStartTileX = -1;
}

void CameraManager::Update(float deltaTime) {
    if (m_followMode && m_target) 
		FollowTarget();

    UpdateVisibleObjects();
}

void CameraManager::Release() {
    m_visibleObjects.clear();
	m_queryBuffer.clear();
	m_queryBuffer.shrink_to_fit();
    ClearTileCache();
}

Gdiplus::RectF CameraManager::GetViewportWorldRect() const {
    return { m_cameraPos.X - WINCX * 0.5f, m_cameraPos.Y - WINCY * 0.5f, (float)WINCX, (float)WINCY };
}

void CameraManager::FollowTarget() {
    if (Transform* t = m_target->GetComponent<Transform>()) {
        float targetX = t->GetX();
        float targetY = t->GetY();

        // 즉시 타겟 위치로 설정 (보간 제거)
        m_cameraPos.X = targetX;
        m_cameraPos.Y = targetY;

        if (m_hasWalkableBounds) {
            m_cameraPos.X = (std::max)(m_walkableMinX + WINCX * 0.5f, (std::min)(m_walkableMaxX - WINCX * 0.5f, m_cameraPos.X));
            m_cameraPos.Y = (std::max)(m_walkableMinY + WINCY * 0.5f, (std::min)(m_walkableMaxY - WINCY * 0.5f, m_cameraPos.Y));
        }
    }
}

bool CameraManager::IsObjectInViewport(GameObject* obj) const {
    if (!obj || !obj->IsEnabled()) return false;
    Gdiplus::RectF bounds = obj->GetBounds();
    Gdiplus::RectF vp = GetViewportWorldRect();
    const float M = 200.0f;
    return bounds.X < vp.X + vp.Width + M && bounds.X + bounds.Width > vp.X - M &&
           bounds.Y < vp.Y + vp.Height + M && bounds.Y + bounds.Height > vp.Y - M;
}

void CameraManager::UpdateVisibleObjects() {
    m_visibleObjects.clear();
    Gdiplus::RectF vp = GetViewportWorldRect();
    const float M = 200.0f;
    Gdiplus::RectF queryRect(vp.X - M, vp.Y - M, vp.Width + 2 * M, vp.Height + 2 * M);

	m_queryBuffer.clear();
    ObjectManager::GetInstance()->GetObjectsInRect(queryRect, m_queryBuffer);

    for (auto* obj : m_queryBuffer)
    {
        if (IsObjectInViewport(obj)) {
            // 중복 방지 (객체가 여러 셀에 걸쳐 있을 수 있으나 현재 구현은 중심점 기반 한 셀에만 존재)
            m_visibleObjects.push_back(obj);
        }
    }
}

GameObject* CameraManager::FindInteractableObjectAtPosition(float x, float y) {
    GameObject* best = nullptr; float maxY = -1e9f;
    
    // 마우스 위치 주변의 객체들만 쿼리 (그리드 최적화 활용)
    float range = 100.0f;
    Gdiplus::RectF queryRect(x - range, y - range, range * 2, range * 2);
    
	m_queryBuffer.clear();
    ObjectManager::GetInstance()->GetObjectsInRect(queryRect, m_queryBuffer);

    for (auto* obj : m_queryBuffer) {
        if (!obj->CanInteract() || !obj->IsEnabled()) continue;
        
        // 수정: 모든 콜라이더 순회 제거, 메인(몸통) 콜라이더만 핀포인트로 체크
        Collider* mainCol = obj->GetMainCollider();
        if (mainCol && mainCol->IsEnabled() && mainCol->ContainsPoint(x, y)) {
            float curY = obj->GetComponent<Transform>()->GetY();
            // 여러 객체가 겹쳐있을 경우 Y값이 큰(아래쪽에 있는) 객체를 우선순위로 선택 (Top-Down 뷰 특성)
            if (!best || curY > maxY) { best = obj; maxY = curY; }
        }
    }
    return best;
}

void CameraManager::FindObjectsIntersectingCollider(Collider* pCol, std::vector<GameObject*>& out, bool onlyInteraction) {
    out.clear();
    if (!pCol) return;

    GameObject* owner = pCol->GetOwner();
    Gdiplus::RectF bounds = pCol->GetWorldRect();

	m_queryBuffer.clear();
    ObjectManager::GetInstance()->GetObjectsInRect(bounds, m_queryBuffer);

    for (auto* obj : m_queryBuffer) {
        if (!obj->IsEnabled() || obj == owner) continue; // 자기 자신 제외

        // 수정: 상대방의 모든 콜라이더가 아닌, 오직 "몸통 콜라이더"와만 충돌 체크
        Collider* mainCol = obj->GetMainCollider();
        if (mainCol && mainCol->IsEnabled()) {
            if (!onlyInteraction || mainCol->IsInteractionCollider()) {
                if (ColliderManager::GetInstance()->Intersects(pCol, mainCol)) {
                    out.push_back(obj); 
                }
            }
        }
    }
}

void CameraManager::RenderVisibleTiles(const MapData* mapData) {
    if (!mapData) return;
    Gdiplus::RectF vp = GetViewportWorldRect();
    int sx = std::max(0, (int)floor((vp.X - TILE_SIZE) / TILE_SIZE));
    int ex = std::min(MAP_WIDTH, (int)ceil((vp.X + vp.Width + TILE_SIZE) / TILE_SIZE));
    int sy = std::max(0, (int)floor((vp.Y - TILE_SIZE) / TILE_SIZE));
    int ey = std::min(MAP_HEIGHT, (int)ceil((vp.Y + vp.Height + TILE_SIZE) / TILE_SIZE));

    if (sx != m_lastStartTileX || ex != m_lastEndTileX || sy != m_lastStartTileY || ey != m_lastEndTileY) {
        CleanupUnusedTileCache(mapData, sx, ex, sy, ey);
        m_lastStartTileX = sx; m_lastEndTileX = ex; m_lastStartTileY = sy; m_lastEndTileY = ey;
    }

    auto* rm = RenderManager::GetInstance();
    for (int y = sy; y < ey; ++y) {
        for (int x = sx; x < ex; ++x) {
            auto& td = mapData->tiles[x][y];
            if (td.id == TILEID_NONE) continue;
            auto it = m_tileCache.find(td.id);
            if (it == m_tileCache.end()) {
                TileCacheData cd; cd.id = td.id; LoadTileBitmap(td, cd);
                if (!cd.bitmap) continue;
                m_tileCache[td.id] = cd; it = m_tileCache.find(td.id);
            }
            Gdiplus::Bitmap* bm = it->second.bitmap;
            float wx = x * TILE_SIZE + TILE_SIZE * 0.5f, wy = y * TILE_SIZE + TILE_SIZE * 0.5f;
            rm->AddWorldEntityCommand(bm, {0,0,(float)bm->GetWidth(),(float)bm->GetHeight()}, wx, wy, (float)TILE_SIZE/bm->GetWidth(), (float)TILE_SIZE/bm->GetHeight(), 0.5f, 0.5f, LAYER_WORLD_TILE, wy);
        }
    }
}

void CameraManager::RenderVisibleGameObjects() {
    for (auto* obj : m_visibleObjects)
		{ obj->Render(); obj->RenderDebugOverlay(); }
}

void CameraManager::CleanupUnusedTileCache(const MapData* md, int sx, int ex, int sy, int ey) {
    std::unordered_set<UINT> visible;
    for (int y = sy; y < ey; ++y) for (int x = sx; x < ex; ++x)
        if (x >= 0 && x < md->mapWidth && y >= 0 && y < md->mapHeight && md->tiles[x][y].id != TILEID_NONE)
            visible.insert(md->tiles[x][y].id);
    for (auto it = m_tileCache.begin(); it != m_tileCache.end();) {
        if (visible.find(it->first) == visible.end()) { delete it->second.bitmap; it = m_tileCache.erase(it); }
        else ++it;
    }
}

void CameraManager::ClearTileCache() {
	if (!m_tileCache.empty()) {
		OutputDebugStringW((L"CameraManager: 타일 캐시 정리 시작 (Count: " + std::to_wstring(m_tileCache.size()) + L")\n").c_str());
		for (auto& p : m_tileCache) {
			if (p.second.bitmap) {
				delete p.second.bitmap;
			}
		}
		m_tileCache.clear();
	}
}

void CameraManager::SetWalkableBoundsFromMapData(const MapData* md) {
    if (!md) return;
    float minX = 1e9f, minY = 1e9f, maxX = -1e9f, maxY = -1e9f; bool found = false;
    for (int y = 0; y < md->mapHeight; ++y) for (int x = 0; x < md->mapWidth; ++x) {
        if (!md->walkableAreas[x][y]) continue;
        float wx = x * TILE_SIZE + TILE_SIZE * 0.5f, wy = y * TILE_SIZE + TILE_SIZE * 0.5f;
        minX = (std::min)(minX, wx); minY = (std::min)(minY, wy); 
        maxX = (std::max)(maxX, wx); maxY = (std::max)(maxY, wy);
        found = true;
    }
    if (found) SetWalkableBounds(minX, minY, maxX, maxY);
}

void CameraManager::SetWalkableBounds(float minX, float minY, float maxX, float maxY) {
    m_hasWalkableBounds = true; m_walkableMinX = minX; m_walkableMinY = minY; m_walkableMaxX = maxX; m_walkableMaxY = maxY;
}

Gdiplus::PointF CameraManager::WorldToScreen(float worldX, float worldY) const {
    Gdiplus::RectF vp = GetViewportWorldRect();
    return { worldX - vp.X, worldY - vp.Y };
}

Gdiplus::PointF CameraManager::ScreenToWorld(float screenX, float screenY) const {
    Gdiplus::RectF vp = GetViewportWorldRect();
    return { screenX + vp.X, screenY + vp.Y };
}

void CameraManager::LoadTileBitmap(const ResourcePathUtils::TileResourceDef& td, TileCacheData& cd) {
    std::wstring path = ResourcePathUtils::BuildResourcePath(td.baseDir, td.imageName);
    if (path.empty()) return;
    cd.bitmap = new Gdiplus::Bitmap(path.c_str());
    if (cd.bitmap->GetLastStatus() != Gdiplus::Ok) { delete cd.bitmap; cd.bitmap = nullptr; }
}
