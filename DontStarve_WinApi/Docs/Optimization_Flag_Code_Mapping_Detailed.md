# Optimization Flag Code Mapping (상세)

이 문서는 최적화 플래그와 실제 코드(클래스/함수/변수)의 연결관계를 한 번에 추적하기 위한 문서입니다.

- 기준 파일:
  - `DontStarve_Client/99_Default/ClientOptimatzationOption.h`
  - `DontStarve_Client/99_Default/Client.cpp`
  - `DontStarve_Client/00_MainGame/DontStarve_MainGame.cpp`
  - `DontStarve_Client/01_Manager/ObjectManager/ObjectManager.cpp`
  - `DontStarve_Client/01_Manager/CameraManager/CameraManager.cpp`
  - `DontStarve_Client/01_Manager/RenderManager/RenderManager.cpp`
  - `DontStarve_Client/01_Manager/SceneManager/SceneManager.cpp`

---

## 1) 플래그 선언 위치

### 1-1. 옵션 헤더

```cpp
// DontStarve_Client/99_Default/ClientOptimatzationOption.h
#ifndef PERF_OPTIMIZATION_DEFAULT_ON
#define PERF_OPTIMIZATION_DEFAULT_ON 1
#endif

extern bool g_bEnableOptimizationMode;
extern bool g_bEnableBufferReuse;           // 버퍼 재사용
extern bool g_bEnableTileCaching;           // 타일 캐싱
extern const char* g_CurrentOptimizationMode;

void ToggleOptimizationMode();
```

### 1-2. 실제 전역 변수/토글 구현

```cpp
// DontStarve_Client/99_Default/Client.cpp
bool g_bEnableOptimizationMode = (PERF_OPTIMIZATION_DEFAULT_ON != 0);
bool g_bEnableBufferReuse = (PERF_OPTIMIZATION_DEFAULT_ON != 0);
bool g_bEnableTileCaching = (PERF_OPTIMIZATION_DEFAULT_ON != 0);

const char* g_CurrentOptimizationMode = (PERF_OPTIMIZATION_DEFAULT_ON != 0) ? "OPTIMIZED" : "BRUTE_FORCE";

void ToggleOptimizationMode()
{
    g_bEnableOptimizationMode = !g_bEnableOptimizationMode;

    if (g_bEnableOptimizationMode) {
        g_bEnableBufferReuse = true;
        g_bEnableTileCaching = true;
        g_CurrentOptimizationMode = "OPTIMIZED";
    } else {
        g_bEnableBufferReuse = false;
        g_bEnableTileCaching = false;
        g_CurrentOptimizationMode = "BRUTE_FORCE";
    }
}
```

---

## 2) MainGame 입력/오버레이 연결

### 2-1. F1/F2 입력 트리거

```cpp
// DontStarve_Client/00_MainGame/DontStarve_MainGame.cpp
const bool isF1Down = InputManager::GetInstance()->IsKeyDown(VK_F1);
if (isF1Down && !m_prevF1Down) {
    m_showPerfOverlay = !m_showPerfOverlay;
}

const bool isF2Down = InputManager::GetInstance()->IsKeyDown(VK_F2);
if (isF2Down && !m_prevF2Down) {
    ToggleOptimizationMode();
}
```

### 2-2. 성능 지표 수집(오버레이)

```cpp
const float avgRenderVisibleGameObjectsMs = CameraManager::GetInstance()->GetAvgRenderVisibleGameObjectsMs();
const float avgRenderVisibleTilesMs = CameraManager::GetInstance()->GetAvgRenderVisibleTilesMs();
```

---

## 3) 클래스별 핵심 분기 코드

## 3-1. ObjectManager (브루트포스 쿼리 + Spatial 계측)

```cpp
// DontStarve_Client/01_Manager/ObjectManager/ObjectManager.cpp
void ObjectManager::Update(float deltaTime)
{
    // ... 객체 업데이트 ...

    // 공간 분할 제거 정책: GridSync는 0으로 유지
    m_lastSpatialPerfSnapshot.gridSyncTargets = 0;
    m_lastSpatialPerfSnapshot.gridSyncMs = 0.0f;
}
```

```cpp
void ObjectManager::QueryObjectsInRect(const Gdiplus::RectF& rect, std::vector<GameObject*>& outObjects)
{
    outObjects.clear();
    if (rect.Width <= 0.0f || rect.Height <= 0.0f) return;

    // 공간 분할 제거 정책: 항상 브루트포스
    for (GameObject* obj : m_worldObjects) {
        if (obj && rect.IntersectsWith(obj->GetBounds())) {
            outObjects.push_back(obj);
        }
    }
}
```

## 3-2. CameraManager (가시성/버퍼/타일 캐시)

```cpp
// DontStarve_Client/01_Manager/CameraManager/CameraManager.cpp
constexpr float kTileCullMargin = -8.0f;
constexpr float kObjectCullMargin = -16.0f;

void CameraManager::RenderVisibleGameObjects()
{
    Gdiplus::RectF vp = GetViewportWorldRect();
    const float M = kObjectCullMargin;
    Gdiplus::RectF queryRect(vp.X - M, vp.Y - M, vp.Width + 2 * M, vp.Height + 2 * M);

    std::vector<GameObject*> localQueryBuffer;
    std::vector<GameObject*>& queryBuffer = g_bEnableBufferReuse ? m_queryBuffer : localQueryBuffer;
    ObjectManager::GetInstance()->QueryObjectsInRect(queryRect, queryBuffer);

    for (GameObject* obj : queryBuffer) {
        if (!obj || !obj->IsEnabled() || obj->IsDead()) continue;
        if (!IsObjectInViewport(obj)) continue;
        obj->Render();
        obj->RenderDebugOverlay();
    }
}
```

```cpp
void CameraManager::RenderVisibleTiles(const MapData* mapData)
{
    // ... 가시 타일 범위 계산 ...
    const float tileCullPadding = kTileCullMargin;

    if (!g_bEnableTileCaching) {
        ClearTileCache(); // 캐시 미사용 경로
    } else {
        CleanupUnusedTileCache(mapData, sx, ex, sy, ey);
    }

    // ... 타일 렌더 커맨드 등록 ...
}
```

## 3-3. RenderManager (정렬/플러시)

```cpp
// DontStarve_Client/01_Manager/RenderManager/RenderManager.cpp
void RenderManager::Flush(Gdiplus::Graphics* pGraphics)
{
    if (!pGraphics) return;

    for (int i = LAYER_TILE_BACKGROUND; i < LAYER_COUNT; ++i) {
        if (m_layerCommands[i].empty()) continue;

        // 입력이 역전된 레이어만 정렬
        if (m_layerCommands[i].size() > 1 && m_layerSortDirty[i]) {
            std::stable_sort(m_layerCommands[i].begin(), m_layerCommands[i].end(), CompareDrawCommands);
        }

        // ... 커맨드 타입별 렌더 + EMA 측정 ...
    }
}
```

## 3-4. SceneManager (렌더 파이프라인 진입)

```cpp
// DontStarve_Client/01_Manager/SceneManager/SceneManager.cpp
void SceneManager::Render()
{
    if (m_currentScene) {
        m_currentScene->Render();
    }
}
```

---

## 4) 매핑 표 (플래그 -> 코드 -> 지표)

| 플래그 | 담당 최적화 | 클래스 | 핵심 함수 | 관련 지표 |
|---|---|---|---|---|
| `g_bEnableBufferReuse` | 쿼리 버퍼 재사용 | `CameraManager` | `RenderVisibleGameObjects`, `FindInteractableObjectAtPosition`, `FindObjectsIntersectingCollider` | `GetAvgRenderVisibleGameObjectsMs()` 간접 영향 |
| `g_bEnableTileCaching` | 타일 비트맵 캐시 | `CameraManager` | `RenderVisibleTiles`, `CleanupUnusedTileCache`, `ClearTileCache` | `GetAvgRenderVisibleTilesMs()` |
| `g_bEnableOptimizationMode` | 전체 모드 마스터 스위치 | `Client` + 전 매니저 | `ToggleOptimizationMode` | 오버레이 모드/EMA 지표 |

---

## 5) 매핑 표 (오버레이 항목 -> 출처 코드)

| F1 오버레이 항목 | 클래스 | 함수/변수 |
|---|---|---|
| FPS(현재/목표) | `TimeManager` | `GetCurrentFPS()`, `GetFPS()` |
| 가시 경로(판정+렌더 제출) | `CameraManager` | `GetAvgRenderVisibleGameObjectsMs()` |
| 가시 타일 렌더 | `CameraManager` | `GetAvgRenderVisibleTilesMs()` |

---

## 6) 실행 흐름(요약)

1. `MainGame::Update()`
   - `F1` -> 오버레이 표시 토글
   - `F2` -> `ToggleOptimizationMode()`
2. `SceneManager::Update()` -> 씬 로직 진행
3. `ObjectManager` / `CameraManager` / `RenderManager`에서 플래그별 분기 수행
4. `MainGame::UpdatePerformanceOverlayText()`에서 지표 수집/텍스트 생성
5. `MainGame::RenderPerformanceOverlay()`가 F1 디버그 패널 렌더

---

## 7) 확인 체크리스트

- [ ] F2 전환 시 `모드: 최적화 ON / 비최적화`가 즉시 바뀌는지
- [ ] `RenderVisibleGameObjects` 값이 모드 전환(F2)에 따라 기대 범위로 변하는지
- [ ] `g_bEnableTileCaching` OFF 시 타일 렌더 시간이 상승하는지
- [ ] 카메라 마진이 타일 `-8.0f`, 오브젝트 `-16.0f`로 분리 적용되는지

---

## 8) 비고

- 현재 렌더 정렬(`RenderManager::Flush`)은 항상 수행하도록 고정되어 있습니다.
- 따라서 렌더 배치/정렬 전용 플래그(`g_bEnableRenderBatching`)는 제거된 상태입니다.

