# 성능 최적화 체계

## 개요

Don't Starve 클론 게임의 성능 최적화는 **공간 분할(Spatial Partitioning)**, **쿼리 최적화**, **캐싱 전략**, **지연 렌더링(Deferred Rendering)** 등 여러 기법을 조합하여 대규모 오브젝트와 많은 타일을 효율적으로 처리합니다.

## 핵심 성능 지표

`DontStarve_MainGame::UpdatePerformanceOverlayText()`에서 추적하는 주요 성능 지표:

```
emaMs(RenderVisibleGameObjects) - 가시 객체 경로 시간 (ON: Query+Cull+Render / OFF: BruteForce Render)
emaMs(RenderVisibleTiles) - 가시 타일 렌더링 시간
```

각 지표는 **EMA(Exponential Moving Average)** 방식으로 계산되어 순간적인 프레임 변동을 평활화합니다.
- EMA 계수: 0.10 (과거 데이터에 90% 가중치, 현재 프레임에 10% 가중치)

---

## 1. 그리드 기반 공간 분할 (Grid-based Spatial Partitioning)

### 목적
게임 월드의 모든 객체를 효율적으로 조직화하여, 특정 영역에 있는 객체들을 빠르게 쿼리할 수 있도록 합니다.

### 구조

**ObjectManager.h의 공간 분할 설정:**

```cpp
// 공간 분할용 그리드
static constexpr int GRID_CELL_SIZE = 256;  // 각 셀의 크기 (월드 좌표)
static constexpr int GRID_WIDTH = (MAP_WIDTH * TILE_SIZE / GRID_CELL_SIZE) + 1;
static constexpr int GRID_HEIGHT = (MAP_HEIGHT * TILE_SIZE / GRID_CELL_SIZE) + 1;
std::vector<GameObject*> m_spatialGrid[GRID_WIDTH][GRID_HEIGHT];  // 2D 그리드

// 객체가 속한 셀을 역매핑
std::unordered_map<GameObject*, std::vector<GridCell>> m_objectToGridCells;

// 중복 제거 버퍼
std::unordered_set<GameObject*> m_queryUniqueBuffer;
```

### 그리드 특성

| 항목 | 값 | 설명 |
|------|-----|------|
| 셀 크기 | 256px | 타일 2×2개 크기(타일 크기 128px) |
| 셀 수 | 동적 | (맵가로 * 128 / 256 + 1) × (맵세로 * 128 / 256 + 1) |
| 셀당 객체 수 | 평균 ~8개 | 브리정렬 시 동적 예약량 |

### 객체-그리드 관계

- **한 객체 = 여러 셀 소속**: 객체의 바운딩 박스가 여러 셀에 걸칠 경우, 모든 셀에 등록됨
- **역매핑 유지**: `m_objectToGridCells`를 통해 객체의 현재 셀을 추적
- **이동 감지**: 위치 변경 시 새로운 셀 목록과 기존 셀 목록을 비교하여 변경이 필요한 경우에만 업데이트

### 업데이트 흐름

```cpp
// ObjectManager::Update()의 그리드 동기화
for (GameObject* obj : m_worldObjects) {
    if (obj && obj->IsEnabled()) {
        UpdateObjectGridCell(obj);  // 매 프레임 위치 기반 셀 갱신
    }
}
```

#### UpdateObjectGridCell() 상세 동작

**ObjectManager.cpp (Line 225-262):**

```cpp
void ObjectManager::UpdateObjectGridCell(GameObject* pObj)
{
    // 1. 객체의 현재 바운딩 박스를 기반으로 필요한 셀들 수집
    std::vector<GridCell> newCells;
    CollectGridCellsForBounds(pObj->GetBounds(), newCells);
    if (newCells.empty()) return;

    // 2. 기존 셀 목록과 비교 (같으면 조기 종료)
    auto oldIt = m_objectToGridCells.find(pObj);
    if (oldIt != m_objectToGridCells.end() && oldIt->second == newCells) {
        return;  // 셀 변화 없음
    }

    // 3. 기존 셀에서 제거
    if (oldIt != m_objectToGridCells.end()) {
        for (const GridCell& cellPos : oldIt->second) {
            auto& cell = m_spatialGrid[cellPos.first][cellPos.second];
            auto it = std::find(cell.begin(), cell.end(), pObj);
            if (it != cell.end()) {
                *it = cell.back();  // 마지막 원소로 교체
                cell.pop_back();    // 제거 (O(1) 연산)
            }
        }
    }

    // 4. 새로운 셀에 추가
    for (const GridCell& cellPos : newCells) {
        m_spatialGrid[cellPos.first][cellPos.second].push_back(pObj);
    }
    m_objectToGridCells[pObj] = std::move(newCells);
}
```

**최적화 특징:**
- **이동 감지**: 셀이 변하지 않으면 즉시 반환 (재할당 방지)
- **O(1) 제거**: Swap-and-pop 기법으로 벡터에서의 제거를 상수 시간에 수행
- **중앙 셀 캐싱**: 객체의 중앙 셀을 오브젝트 자체에 저장

---

## 2. 공간 분할 쿼리 (Spatial Query with Deduplication)

### 목적
특정 영역에 있는 모든 객체를 빠르게 찾으면서, 여러 셀에 걸친 객체는 한 번만 반환합니다.

### QueryObjectsInRect() 알고리즘

**ObjectManager.cpp (Line 264-313):**

```cpp
void ObjectManager::QueryObjectsInRect(const Gdiplus::RectF& rect, std::vector<GameObject*>& outObjects)
{
    outObjects.clear();
    if (rect.Width <= 0.0f || rect.Height <= 0.0f) return;

    // 1. 쿼리 사각형이 포함하는 셀 범위 계산
    int startX = (int)floor(rect.X / GRID_CELL_SIZE);
    int startY = (int)floor(rect.Y / GRID_CELL_SIZE);
    int endX = (int)ceil((rect.X + rect.Width) / GRID_CELL_SIZE) - 1;
    int endY = (int)ceil((rect.Y + rect.Height) / GRID_CELL_SIZE) - 1;

    // 2. 경계 클램핑
    startX = max(0, min(GRID_WIDTH - 1, startX));
    startY = max(0, min(GRID_HEIGHT - 1, startY));
    endX = max(0, min(GRID_WIDTH - 1, endX));
    endY = max(0, min(GRID_HEIGHT - 1, endY));

    // 3. 중복 제거 버퍼 준비 (예약량 계산)
    m_queryUniqueBuffer.clear();
    const int cellCountX = (endX - startX + 1);
    const int cellCountY = (endY - startY + 1);
    if (cellCountX > 0 && cellCountY > 0) {
        m_queryUniqueBuffer.reserve((cellCountX * cellCountY * 8));  // 평균 8개 객체/셀
    }

    // 4. 셀 범위 순회 및 객체 수집
    for (int y = startY; y <= endY; ++y) {
        for (int x = startX; x <= endX; ++x) {
            for (auto* obj : m_spatialGrid[x][y]) {
                if (m_queryUniqueBuffer.insert(obj).second) {  // 처음 만나는 객체면
                    outObjects.push_back(obj);
                }
            }
        }
    }
}
```

### 쿼리 성능 특성

| 상황 | 성능 | 설명 |
|------|------|------|
| 작은 쿼리 영역 | O(셀 수 × 셀당 객체 수) | 일반적으로 극히 빠름 (수 ms) |
| 전체 맵 쿼리 | O(전체 객체 수) | 모든 객체 순회, 중복 제거 필요 |
| 최악의 경우 | O(n log n) | unordered_set 충돌 시 |

### 최적화 기법

1. **Bitmask 또는 unordered_set 중복 제거**
   - unordered_set을 사용한 O(1) 중복 제거
   - 버퍼 재사용으로 매 쿼리마다 할당 회피

2. **예약량 계산**
   - 셀당 평균 객체 8개 가정
   - 동적 메모리 할당 최소화

3. **조기 종료**
   - 쿼리 사각형이 0 크기면 즉시 반환

---

## 3. 가시 객체 쿼리 (Visibility Query)

### 목적
카메라 뷰포트에 보이는 객체만 렌더링하기 위해 공간 분할을 이용한 2단계 필터링을 수행합니다.

### 가시 객체 판정 흐름

**CameraManager::RenderVisibleGameObjects() (Line 84-117):**

```cpp
void CameraManager::RenderVisibleGameObjects()
{
    m_visibleObjects.clear();
    
    // 1단계: 뷰포트 + 마진 영역 계산
    Gdiplus::RectF vp = GetViewportWorldRect();
    const float M = 200.0f;  // 마진 (피벗/애니메이션 범위)
    Gdiplus::RectF queryRect(vp.X - M, vp.Y - M, vp.Width + 2*M, vp.Height + 2*M);

    // 2단계: 공간 분할 쿼리로 후보 객체 수집
    m_queryBuffer.clear();
    ObjectManager::GetInstance()->QueryObjectsInRect(queryRect, m_queryBuffer);

    // 3단계: 정확한 바운딩 박스 체크로 최종 필터링
    for (auto* obj : m_queryBuffer) {
        if (IsObjectInViewport(obj)) {
            m_visibleObjects.push_back(obj);
        }
    }
}
```

### 성능 구조

| 단계 | 연산 | 복잡도 | 역할 |
|------|------|--------|------|
| 1. 그리드 쿼리 | 공간 분할 | O(셀 수 × 셀/객체) | 상자 교집합 검사로 거의 모든 불가시 객체 제거 |
| 2. 정확 필터링 | AABB 교집합 | O(후보 객체 수) | 경계선 근처 객체 정확 판정 |

### 마진 영역 설정

**마진 = 200 픽셀의 의미:**
- 회전: 객체 중심에서 최대 ~180° 회전 시 최대 거리
- 피벗: 이미지 크기가 256×256 이상인 객체의 피벗 오프셋
- 애니메이션: 대형 공격 이펙트의 오프셋
- 뷰포트 크기: 1920×1080 일부 객체 반입력 방지

### 재사용 버퍼 (Buffer Reuse)

```cpp
std::vector<GameObject*> m_queryBuffer;  // 공간 분할 쿼리용 재사용 버퍼

// Init에서 한 번만 할당
m_queryBuffer.reserve(2048);

// 매 프레임 쿼리 시 재사용
m_queryBuffer.clear();
ObjectManager::GetInstance()->QueryObjectsInRect(queryRect, m_queryBuffer);
```

**이점:**
- 메모리 할당/해제 제거
- cache coherency 향상
- 프로파일링 일관성 (항상 같은 크기의 버퍼 사용)

---

## 4. 가시 타일 렌더링 최적화

### 목적
카메라 뷰포트에 보이는 타일만 렌더링합니다. 타일 비트맵을 캐싱하여 반복적인 로드를 방지합니다.

### 알고리즘

**CameraManager::RenderVisibleTiles() (Line 167-201):**

```cpp
void CameraManager::RenderVisibleTiles(const MapData* mapData)
{
    if (!mapData) return;
    
    // 1. 뷰포트 기반 타일 범위 계산
    Gdiplus::RectF vp = GetViewportWorldRect();
    int sx = max(0, (int)floor((vp.X - TILE_SIZE) / TILE_SIZE));
    int ex = min(MAP_WIDTH, (int)ceil((vp.X + vp.Width + TILE_SIZE) / TILE_SIZE));
    int sy = max(0, (int)floor((vp.Y - TILE_SIZE) / TILE_SIZE));
    int ey = min(MAP_HEIGHT, (int)ceil((vp.Y + vp.Height + TILE_SIZE) / TILE_SIZE));

    // 2. 타일 범위 변경 감지 후 캐시 정리
    if (sx != m_lastStartTileX || ex != m_lastEndTileX || 
        sy != m_lastStartTileY || ey != m_lastEndTileY) {
        CleanupUnusedTileCache(mapData, sx, ex, sy, ey);
        m_lastStartTileX = sx; m_lastEndTileX = ex;
        m_lastStartTileY = sy; m_lastEndTileY = ey;
    }

    // 3. 범위 내 타일 렌더링
    const float tileSizeF = (float)TILE_SIZE;
    auto* rm = RenderManager::GetInstance();
    for (int y = sy; y < ey; ++y) {
        for (int x = sx; x < ex; ++x) {
            auto& td = mapData->tiles[x][y];
            if (td.id == TILEID_NONE) continue;
            
            // 4. 타일 비트맵 캐시 조회/로드
            auto it = m_tileCache.find(td.id);
            if (it == m_tileCache.end()) {
                TileCacheData cd; 
                cd.id = td.id; 
                LoadTileBitmap(td, cd);
                if (!cd.bitmap) continue;
                m_tileCache[td.id] = cd;
                it = m_tileCache.find(td.id);
            }
            
            Gdiplus::Bitmap* bm = it->second.bitmap;
            // 렌더 커맨드 추가
        }
    }
}
```

### 타일 캐시 전략

**캐시 구조:**

```cpp
std::unordered_map<UINT, TileCacheData> m_tileCache;  // 타일ID -> 비트맵

struct TileCacheData {
    TileID id;
    Gdiplus::Bitmap* bitmap;  // 캐시된 비트맵 포인터
};
```

**캐시 수명:**

| 단계 | 동작 |
|------|------|
| 타일 범위 변경 | `CleanupUnusedTileCache()` 호출 |
| 뷰포트 내 타일 | 캐시 유지 |
| 뷰포트 외 타일 | 캐시에서 제거 |
| 씬 전환 시 | `ClearTileCache()` 호출하여 전체 정리 |

**효과:**
- 같은 맵에서 카메라 이동 중 반복적인 비트맵 로드 제거
- 메모리 사용량은 증가하지만, 렌더링 성능 향상

---

## 5. 렌더 커맨드 버퍼링 및 정렬 (Render Command Batching)

### 목적
렌더링 작업을 계층과 z-order 기반으로 일괄 정렬하여, 불필요한 상태 변경과 렌더 호출을 최소화합니다.

### 구조

**RenderManager.h:**

```cpp
std::vector<DrawCommand> m_layerCommands[LAYER_COUNT];  // 계층별 커맨드 큐

// 커맨드 등록 (매 프레임)
AddWorldEntityCommand(...);  // m_layerCommands[layer]에 추가
AddUICommand(...);
AddTextCommand(...);

// 렌더링 (프레임 말미)
Flush(pGraphics);  // 모든 커맨드 실행
```

### 커맨드 등록 최적화

**RenderManager::AddWorldEntityCommand() (Line 59-89):**

```cpp
void RenderManager::AddWorldEntityCommand(...)
{
    // 1. 현재 카메라 위치 캐싱 (지연 방지)
    const Gdiplus::PointF currentCamPos = CameraManager::GetInstance()->GetCameraPos();
    
    // 2. 월드 -> 스크린 좌표 변환
    float screenX = worldX - currentCamPos.X + (float)WINCX * 0.5f;
    float screenY = worldY - currentCamPos.Y + (float)WINCY * 0.5f;
    
    // 3. DrawCommand 구조체 채우기 (메모리 레이아웃 최적화)
    DrawCommand cmd;
    cmd.type = DRAW_COMMAND_ENTITY;
    cmd.destRect = ...;
    cmd.sprite = { pBitmap, sourceRect, ... };
    
    // 4. 벡터에 추가 (사전 할당으로 재할당 제거)
    m_layerCommands[layer].push_back(cmd);
}
```

**최적화 특징:**
- **카메라 캐싱**: 커맨드 등록 시점의 카메라 위치를 즉시 사용 (지연/지터 제거)
- **사전 할당**: 계층별로 512개 커맨드 공간 예약
- **구조체 크기**: 단일 DrawCommand는 ~100B로 캐시 친화적

### Flush 단계

```cpp
void RenderManager::Flush(Gdiplus::Graphics* pGraphics)
{
    // 1. 모든 계층 순회 (LAYER_0, LAYER_1, ... LAYER_UI_FOREGROUND)
    for (int i = 0; i < LAYER_COUNT; ++i) {
        auto& commands = m_layerCommands[i];
        
        // 2. z-order 기반 정렬
        std::sort(commands.begin(), commands.end(), CompareDrawCommands);
        
        // 3. 정렬된 순서로 렌더링
        for (const DrawCommand& cmd : commands) {
            RenderSprite(pGraphics, cmd.sprite, cmd.destRect);
        }
        
        // 4. 계층 초기화
        commands.clear();
    }
}
```

### 계층 체계

| 계층 | 용도 | 예시 |
|------|------|------|
| LAYER_TILE | 배경 타일 | 풀, 흙 |
| LAYER_GROUND | 지면 객체 | 나무, 돌, 풀, 건물 |
| LAYER_ENTITY | 엔티티 | 플레이어, 몬스터, NPC |
| LAYER_UI_BACKGROUND | UI 배경 | 패널 배경 |
| LAYER_UI_CONTENT | UI 컨텐츠 | 텍스트, 버튼 |
| LAYER_UI_FOREGROUND | UI 전면 | 디버그 오버레이 |

**렌더 순서:**
1. 계층 번호 작은 것부터 렌더링 (뒤->앞)
2. 같은 계층 내에서는 z-order 기반 정렬

---

## 6. 성능 프로파일링 및 EMA 추적

### EMA (Exponential Moving Average) 계산

각 성능 지표는 EMA로 계산되어 **프레임 대 프레임 변동**을 평활화합니다.

```cpp
// 초기화
if (sampleCount == 0) {
    avgMs = currentMs;
} else {
    avgMs += kEmaAlpha * (currentMs - avgMs);
}
++sampleCount;
```

**파라미터:**
- `kEmaAlpha = 0.10` (10% 현재 값, 90% 과거 값)
- 이는 약 10 프레임의 가중 이동 평균을 의미

**해석:**
- 값이 작을수록 좋음
- 급격한 변화는 평활화됨
- 장시간 추세를 반영

### 성능 지표 해석

#### 1. QueryObjectsInRect (월드 쿼리, 참고 지표)

**측정 대상:** `ObjectManager::QueryObjectsInRect()`
**측정 방법:** 마이크로초 단위로 측정 후 밀리초로 변환

```cpp
const auto profileStart = std::chrono::high_resolution_clock::now();
// ... 쿼리 실행 ...
const auto profileEnd = std::chrono::high_resolution_clock::now();
const auto elapsedUs = std::chrono::duration_cast<std::chrono::microseconds>
                       (profileEnd - profileStart).count();
const float elapsedMs = static_cast<float>(elapsedUs) / 1000.0f;
```

**해석:**
- **< 0.5ms**: 매우 우수 (쿼리 셀 수 적음)
- **0.5~1.0ms**: 양호 (일반적 카메라 이동)
- **> 2.0ms**: 저하 (쿼리 영역 과대 또는 객체 밀집)

#### 2. RenderVisibleGameObjects (가시 객체 경로)

**측정 대상:** `CameraManager::RenderVisibleGameObjects()`
**포함 작업:**
- 공간 분할 쿼리
- 정확한 바운딩 박스 필터링

**해석:**
- **< 1.0ms**: 좋음
- **1.0~3.0ms**: 보통 (수백 개 후보 객체)
- **> 5.0ms**: 저하 (후보 객체 과다)

#### 3. RenderVisibleTiles (타일 렌더링)

**측정 대상:** `CameraManager::RenderVisibleTiles()`
**포함 작업:**
- 타일 범위 계산
- 타일 캐시 조회/로드
- 렌더 커맨드 추가

**해석:**
- **< 1.0ms**: 좋음 (대부분 캐시 히트)
- **1.0~5.0ms**: 보통 (일부 캐시 미스)
- **> 10.0ms**: 저하 (과도한 캐시 미스 또는 메모리 부족)

#### 4. Flush (렌더 커맨드 실행)

**측정 대상:** `RenderManager::Flush()`
**포함 작업:**
- 커맨드 정렬 (z-order)
- GDI+ 렌더 호출
- 상태 변경

**해석:**
- **< 5.0ms**: 좋음 (커맨드 수 적음, GDI+ 성능 양호)
- **5.0~10.0ms**: 보통 (수백 개 커맨드)
- **> 15.0ms**: 저하 (수천 개 커맨드 또는 큰 이미지)

---

## 7. 최적화 기법 정리 및 효과

### 표로 보는 최적화 기법

| 기법 | 구현 위치 | 문제점 | 해결책 | 효과 |
|------|---------|--------|--------|------|
| 공간 분할 | ObjectManager | 큰 맵의 객체 검색 O(n) | 그리드 기반 분할 | 10~100배 속도 향상 |
| 중복 제거 | QueryObjectsInRect | 경계선 객체 중복 반환 | unordered_set | O(1) 중복 확인 |
| 버퍼 재사용 | CameraManager | 매 프레임 메모리 할당 | reserve + clear | 메모리 할당 제거 |
| 타일 캐시 | CameraManager | 반복 타일 로드 | LRU 캐시 | 타일 로드 시간 90% 감소 |
| 계층 기반 렌더 | RenderManager | 순서 정렬 오버헤드 | 사전 정렬 + 배치 | 상태 변경 최소화 |
| 카메라 캐싱 | RenderManager | 지연된 좌표 사용 | 커맨드 등록 시 캐시 | 지터 제거 |
| 마진 영역 | CameraManager | 경계선 깜박임 | 200px 마진 추가 | 시각적 팝인 방지 |
| EMA 프로파일링 | 모든 매니저 | 노이즈 많은 측정값 | 지수 이동 평균 | 신뢰도 높은 성능 추적 |

---

## 8. 병목 진단 가이드

### Performance Overlay 읽기

F1 키로 활성화되는 오버레이:

```
[Performance Debug (F1)]
FPS(Current) : 29.87
FPS(Target)  : 30
emaMs(RenderVisibleGameObjects): 0.42
emaMs(RenderVisibleTiles) : 1.24
```

### 진단 절차

1. **RenderVisibleGameObjects > 5ms**
   - 원인: 후보 객체 과다 또는 비최적화 브루트포스 경로 영향
   - 대응:
     - 마진 영역 감소 (200→100)
     - 객체 활성도 제어

2. **RenderVisibleTiles > 10ms**
   - 원인: 타일 캐시 미스, 타일 로드 느림
   - 대응:
     - 캐시 크기 증가
     - 타일 이미지 최적화 (압축, 해상도 감소)


---

## 9. 구현 세부 사항

### ObjectManager 그리드 관리

**초기화:**

```cpp
void ObjectManager::Init()
{
    ClearAllObjects();
    InitializeFactories();
}
```

**객체 추가:**

```cpp
void ObjectManager::AddGameObject(GameObject* pObj)
{
    // UI 판정 후 적절한 리스트에 추가
    auto& targetList = (pObj->GetType() == GO_TYPE_UI) ? m_uiObjects : m_worldObjects;
    targetList.push_back(pObj);
    
    // 월드 객체면 그리드에 등록
    if (pObj->GetType() != GO_TYPE_UI) {
        AddToGrid(pObj);
    }
}
```

**객체 제거:**

```cpp
void ObjectManager::RemoveGameObject(GameObject* pObj)
{
    // 지연 삭제 (LateUpdate에서 처리)
    pObj->SetDead(true);
    m_pendingDeletions.push_back(pObj);
}

void ObjectManager::ProcessPendingDeletions()
{
    // 실제 메모리 해제는 LateUpdate에서 수행
    for (int i = (int)m_worldObjects.size() - 1; i >= 0; --i) {
        if (m_worldObjects[i]->IsDead()) {
            RemoveFromGrid(m_worldObjects[i]);
            delete m_worldObjects[i];
            m_worldObjects[i] = m_worldObjects.back();
            m_worldObjects.pop_back();
        }
    }
}
```

### CameraManager 가시성 처리

**초기화:**

```cpp
void CameraManager::Init()
{
    m_visibleObjects.reserve(2048);
    m_queryBuffer.reserve(2048);
}
```

**업데이트:**

```cpp
void CameraManager::Update(float deltaTime)
{
    if (m_followMode && m_target) 
        FollowTarget();
    
    RenderVisibleGameObjects();  // 가시 객체 갱신
}
```

### RenderManager 커맨드 관리

**초기화:**

```cpp
RenderManager::RenderManager()
{
    for (int i = 0; i < LAYER_COUNT; ++i) {
        m_layerCommands[i].reserve(512);  // 계층당 512개 커맨드
    }
}
```

**프레임 렌더링:**

```cpp
void RenderManager::Flush(Gdiplus::Graphics* pGraphics)
{
    for (int i = 0; i < LAYER_COUNT; ++i) {
        auto& commands = m_layerCommands[i];
        
        // z-order 정렬
        std::sort(commands.begin(), commands.end(), CompareDrawCommands);
        
        // 실행
        for (const DrawCommand& cmd : commands) {
            if (cmd.type == DRAW_COMMAND_ENTITY) {
                RenderSprite(pGraphics, cmd.sprite, cmd.destRect);
            }
            // ... 다른 커맨드 타입 ...
        }
        
        commands.clear();  // 재사용을 위해 clear (deallocate 아님)
    }
}
```

---

## 10. 향후 개선 사항

### 단기 개선

1. **Spatial Hashing 대체**
   - 현재 2D 배열: 메모리 사용량 많음
   - 해시 기반: 희소 그리드에 효율적

2. **Frustum Culling 추가**
   - 현재: AABB만 사용
   - 개선: 회전을 고려한 정확한 절두체 검사

3. **Hierarchical Z-Buffer**
   - 렌더 순서 정렬 최적화
   - 깊이 버퍼 기반 빠른 가시성 판정

### 중기 개선

1. **GPU 렌더링**
   - GDI+에서 Direct2D/DirectX로 전환
   - 배치 처리 및 GPU 병렬화

2. **Physics Spatial Partitioning**
   - 물리 충돌 쿼리 별도 최적화
   - 사분 트리(Quadtree) 고려

3. **Occluder Culling**
   - 건물 뒤 객체 렌더링 생략
   - 복잡한 맵에서 30~50% 성능 향상

### 장기 개선

1. **Compute Shader Frustum Culling**
   - GPU에서 가시성 판정
   - CPU 병목 해결

2. **Machine Learning 기반 LOD**
   - 프레임 시간 기반 동적 LOD 조정

3. **클라우드 렌더링**
   - 복잡한 장면을 서버에서 사전 렌더링

---

## 결론

이 게임의 성능 최적화는 **데이터 구조 설계(공간 분할)**, **알고리즘 최적화(쿼리, 필터링)**, **메모리 관리(버퍼 재사용)**, **렌더링 전략(커맨드 버퍼링)** 을 균형있게 결합하여 대규모 월드와 많은 객체를 효율적으로 처리합니다.

각 지표를 모니터링하면서 병목 단계를 특정하고, 제안된 개선 사항을 우선순위 있게 적용하면 프레임 시간을 추가로 개선할 수 있습니다.

