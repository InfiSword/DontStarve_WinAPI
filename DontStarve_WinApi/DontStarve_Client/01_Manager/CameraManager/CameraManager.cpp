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
    m_tileCache.clear();
    m_hasWalkableBounds = false;
    m_lastStartTileX = -1;
}

void CameraManager::Update(float deltaTime) {
    if (m_followMode && m_target) FollowTarget();
    UpdateVisibleObjects();
}

void CameraManager::Release() {
    m_visibleObjects.clear();
    ClearTileCache();
}

Gdiplus::PointF CameraManager::WorldToScreen(float x, float y) const {
    return { x - m_cameraPos.X + WINCX * 0.5f, y - m_cameraPos.Y + WINCY * 0.5f };
}

Gdiplus::PointF CameraManager::ScreenToWorld(float x, float y) const {
    return { x + m_cameraPos.X - WINCX * 0.5f, y + m_cameraPos.Y - WINCY * 0.5f };
}

Gdiplus::RectF CameraManager::GetViewportWorldRect() const {
    return { m_cameraPos.X - WINCX * 0.5f, m_cameraPos.Y - WINCY * 0.5f, (float)WINCX, (float)WINCY };
}

void CameraManager::FollowTarget() {
    if (Transform* t = m_target->GetComponent<Transform>()) {
        m_cameraPos = { t->GetX(), t->GetY() };
        if (m_hasWalkableBounds) {
            m_cameraPos.X = std::max(m_walkableMinX + WINCX * 0.5f, std::min(m_walkableMaxX - WINCX * 0.5f, m_cameraPos.X));
            m_cameraPos.Y = std::max(m_walkableMinY + WINCY * 0.5f, std::min(m_walkableMaxY - WINCY * 0.5f, m_cameraPos.Y));
        }
    }
}

bool CameraManager::IsObjectInViewport(GameObject* obj) const {
    if (!obj || !obj->IsEnabled()) return false;
    Gdiplus::RectF bounds = GetSpriteBoundingBox(obj);
    Gdiplus::RectF vp = GetViewportWorldRect();
    const float M = 200.0f;
    return bounds.X < vp.X + vp.Width + M && bounds.X + bounds.Width > vp.X - M &&
           bounds.Y < vp.Y + vp.Height + M && bounds.Y + bounds.Height > vp.Y - M;
}

void CameraManager::UpdateVisibleObjects() {
    m_visibleObjects.clear();
    for (auto* obj : ObjectManager::GetInstance()->GetGameObjects())
        if (IsObjectInViewport(obj)) m_visibleObjects.push_back(obj);
}

void CameraManager::RemoveFromVisibleObjects(GameObject* obj) {
    auto it = std::find(m_visibleObjects.begin(), m_visibleObjects.end(), obj);
    if (it != m_visibleObjects.end()) m_visibleObjects.erase(it);
}

void CameraManager::TryAddToVisibleIfInViewport(GameObject* obj) {
    if (IsObjectInViewport(obj)) m_visibleObjects.push_back(obj);
}

GameObject* CameraManager::FindInteractableObjectAtPosition(float x, float y) {
    GameObject* best = nullptr; float maxY = -1e9f;
    for (auto* obj : m_visibleObjects) {
        if (!obj->CanInteract()) continue;
        for (auto* col : obj->GetComponents<Collider>()) {
            if (col->IsEnabled() && col->ContainsPoint(x, y)) {
                float curY = obj->GetComponent<Transform>()->GetY();
                if (!best || curY > maxY) { best = obj; maxY = curY; }
                break;
            }
        }
    }
    return best;
}

void CameraManager::FindObjectsIntersectingCollider(Collider* pCol, std::vector<GameObject*>& out) {
    out.clear();
    for (auto* obj : m_visibleObjects) {
        for (auto* col : obj->GetComponents<Collider>()) {
            if (col->IsEnabled() && ColliderManager::GetInstance()->Intersects(pCol, col)) {
                out.push_back(obj); break;
            }
        }
    }
}

Gdiplus::RectF CameraManager::GetSpriteBoundingBox(GameObject* obj) const {
    Transform* t = obj->GetComponent<Transform>();
    if (!t) return {0,0,0,0};
    float w = 32, h = 32, px = 0.5f, py = 0.5f;
    if (auto* anim = obj->GetComponent<Animator>()) {
        if (auto sprite = anim->GetCurrentFrame().sprite) {
            w = sprite->sourceRect.Width; h = sprite->sourceRect.Height;
            px = sprite->pivot.X; py = sprite->pivot.Y;
        }
    } else if (auto* sr = obj->GetComponent<SpriteRenderer>()) {
        if (auto sprite = sr->GetSpriteHandle()) {
            w = sprite->sourceRect.Width; h = sprite->sourceRect.Height;
            px = sprite->pivot.X; py = sprite->pivot.Y;
        }
    }
    w *= t->GetScaleX(); h *= t->GetScaleY();
    return { t->GetX() - w * px, t->GetY() - h * py, w, h };
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
    for (auto* obj : m_visibleObjects) { obj->Render(); obj->RenderDebugOverlay(); }
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
    for (auto& p : m_tileCache) delete p.second.bitmap;
    m_tileCache.clear();
}

void CameraManager::SetWalkableBoundsFromMapData(const MapData* md) {
    if (!md) return;
    float minX = 1e9f, minY = 1e9f, maxX = -1e9f, maxY = -1e9f; bool found = false;
    for (int y = 0; y < md->mapHeight; ++y) for (int x = 0; x < md->mapWidth; ++x) {
        if (!md->walkableAreas[x][y]) continue;
        float wx = x * TILE_SIZE + TILE_SIZE * 0.5f, wy = y * TILE_SIZE + TILE_SIZE * 0.5f;
        minX = std::min(minX, wx); minY = std::min(minY, wy); maxX = std::max(maxX, wx); maxY = std::max(maxY, wy);
        found = true;
    }
    if (found) SetWalkableBounds(minX, minY, maxX, maxY);
}

void CameraManager::SetWalkableBounds(float minX, float minY, float maxX, float maxY) {
    m_hasWalkableBounds = true; m_walkableMinX = minX; m_walkableMinY = minY; m_walkableMaxX = maxX; m_walkableMaxY = maxY;
}

void CameraManager::LoadTileBitmap(const ResourcePathUtils::TileResourceDef& td, TileCacheData& cd) {
    std::wstring path = ResourcePathUtils::BuildResourcePath(td.baseDir, td.imageName);
    if (path.empty()) return;
    cd.bitmap = new Gdiplus::Bitmap(path.c_str());
    if (cd.bitmap->GetLastStatus() != Gdiplus::Ok) { delete cd.bitmap; cd.bitmap = nullptr; }
}
