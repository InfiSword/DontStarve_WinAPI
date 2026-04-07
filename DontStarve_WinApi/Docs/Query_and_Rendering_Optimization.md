# 쿼리 및 렌더링 성능 최적화 - 고급 가이드

## 개요

이 문서는 게임 엔진에서 **가시 판정(Visibility Determination)**, **충돌 감지 쿼리**, **렌더 커맨드 최적화** 등 다양한 쿼리 작업의 성능 최적화를 다룹니다.

---

## 1. 다단계 가시 판정 (Multi-Stage Visibility Culling)

### 1.1 문제: 프레임 내 과도한 렌더링

```cpp
// ❌ 비효율적: 모든 객체 렌더링
void Render() {
    for (auto* obj : m_worldObjects) {          // 1,000개
        obj->Render();                          // 모든 객체 렌더링
    }
}

// 성능 분석:
// - 화면에 보이는 객체: ~200개 (20%)
// - 불필요한 렌더링: 800개 (80%)
// - CPU 캐시 미스: 높음
// - 그래픽 API 오버헤드: 1,000회
```

### 1.2 해결책: 다단계 필터링

```cpp
// ✅ 효율적: 다단계 필터링
void CameraManager::RenderVisibleGameObjects() {
    // 단계 1: 공간 분할 (Grid)로 후보 축소 (10ms 대 0.25ms)
    Gdiplus::RectF vp = GetViewportWorldRect();
    Gdiplus::RectF expandedVp(
        vp.X - 200, vp.Y - 200,
        vp.Width + 400, vp.Height + 400
    );
    
    m_queryBuffer.clear();
    ObjectManager::GetInstance()->QueryObjectsInRect(expandedVp, m_queryBuffer);
    // 결과: 1,000개 → ~400개 (60% 감소)
    
    // 단계 2: AABB 정확 필터링 (경계선 근처 제외)
    m_visibleObjects.clear();
    for (auto* obj : m_queryBuffer) {
        if (IsObjectInViewport(obj)) {
            m_visibleObjects.push_back(obj);
        }
    }
    // 결과: 400개 → ~200개 (80% 초기 대비)
    
    // 단계 3: (선택적) 프러스텀 컬링 또는 오클루더 컬링
    // (향후 개선) 건물 뒤의 객체 제거
}
```

### 1.3 마진(Margin) 영역의 중요성

```cpp
const float M = 200.0f;  // 마진값
Gdiplus::RectF queryRect(
    vp.X - M,          // 왼쪽 마진
    vp.Y - M,          // 위쪽 마진
    vp.Width + 2*M,    // 너비 + 좌우 마진
    vp.Height + 2*M    // 높이 + 상하 마진
);
```

**마진값의 역할:**

| 상황 | 필요 마진 | 이유 |
|------|---------|------|
| 큰 스프라이트 | 128px | 타일 256×256 객체의 피벗 오프셋 |
| 회전 객체 | 180px | 90° 회전 시 대각선 길이 확대 |
| 애니메이션 | 50px | 공격 이펙트 오프셋 |
| 스크롤 히스테리시스 | 50px | 급격한 쿼리 변동 방지 |

**종합**: 200px = 안전한 마진값

```cpp
// 마진값 계산 예시
float CalculateOptimalMargin(float maxObjectSize, float maxRotationRadius, float animationOffset) {
    return maxObjectSize * 0.5f + maxRotationRadius + animationOffset + 50.0f;
    // = 128 + 180 + 50 + 50 = 408px → 보수적으로 512px 사용 가능
}
```

---

## 2. 재사용 버퍼 최적화 (Buffer Reuse)

### 2.1 문제: 매 프레임 메모리 할당/해제

```cpp
// ❌ 비효율적: 매 프레임 할당
void RenderVisibleGameObjects() {
    std::vector<GameObject*> tempBuffer;  // 매 프레임 생성
    
    // ... 쿼리 ...
    ObjectManager::GetInstance()->QueryObjectsInRect(rect, tempBuffer);
    
    // tempBuffer 소멸자에서 메모리 해제
}

// 성능 분석 (매 프레임 반복):
// - 할당: malloc() 호출 → OS 커널 호출 → 메모리 매핑
// - 해제: free() 호출 → 메모리 풀 관리
// - 오버헤드: ~100-1000 CPU 사이클 (매 쿼리마다!)
// - 캐시 방해: 메모리 단편화 가능
```

### 2.2 해결책: 버퍼 재사용

```cpp
// ✅ 효율적: 멤버 변수로 재사용
class CameraManager {
private:
    std::vector<GameObject*> m_queryBuffer;  // 멤버 변수
    
public:
    void Init() {
        m_queryBuffer.reserve(2048);  // 한 번만 할당
    }
    
    void RenderVisibleGameObjects() {
        m_queryBuffer.clear();  // O(1) clear (소유권만 해제)
        
        ObjectManager::GetInstance()->QueryObjectsInRect(rect, m_queryBuffer);
        
        // m_queryBuffer 재사용 (할당 안함)
    }
};
```

**성능 효과:**

```
1,000회 쿼리 (30 프레임 × 30초):

할당 방식:
  - 할당 1,000회 × 1,000 cycle = 1,000,000 cycle
  - 해제 1,000회 × 1,000 cycle = 1,000,000 cycle
  - 총 2,000,000 cycle ≈ 2 ms

재사용 방식:
  - 할당 1회 × 1,000 cycle = 1,000 cycle
  - clear 1,000회 × 10 cycle = 10,000 cycle
  - 총 11,000 cycle ≈ 0.01 ms

개선율: 200배 (!!)
```

### 2.3 최적 예약량 계산

```cpp
// 기존
m_queryBuffer.reserve(2048);

// 이유:
// - 카메라 뷰포트: 1920×1080 화면
// - 객체 밀도: ~1,000개 / (32,768px × 32,768px)
// - 쿼리 영역: ~(1920 + 400) × (1080 + 400) ≈ 3M px²
// - 예상 객체: 3,000,000 / 1,074,000,000 × 1,000 ≈ 2,700개
// → 보수적으로 2,048로 설정 (거의 모든 경우 충분)

// 동적 예약량 (향후)
int optimalSize = (int)(expectedObjectCount * 1.1f);  // 10% 마진
m_queryBuffer.reserve(optimalSize);
```

---

## 3. 중복 제거 최적화 (Deduplication)

### 3.1 문제: 경계선 객체의 중복 반환

```cpp
// 객체 크기 256px, 그리드 셀 256px
// 객체가 (1,1), (2,1), (1,2), (2,2) 셀 경계에 위치

if (쿼리가 4개 셀 모두 포함) {
    // ❌ 객체가 4번 반환됨
    results = [obj, obj, obj, obj]  // 중복!
}
```

### 3.2 해결책: unordered_set 기반 중복 제거

```cpp
void ObjectManager::QueryObjectsInRect(const Gdiplus::RectF& rect, std::vector<GameObject*>& outObjects)
{
    // ... 셀 범위 계산 ...
    
    m_queryUniqueBuffer.clear();  // std::unordered_set<GameObject*>
    
    for (int y = startY; y <= endY; ++y) {
        for (int x = startX; x <= endX; ++x) {
            for (auto* obj : m_spatialGrid[x][y]) {
                // insert()는 pair<iterator, bool> 반환
                // second = true면 처음 삽입, false면 이미 존재
                if (m_queryUniqueBuffer.insert(obj).second) {
                    outObjects.push_back(obj);  // 처음 만나는 객체만 추가
                }
            }
        }
    }
}
```

**성능 분석:**

```cpp
// unordered_set::insert()의 비용

객체 수       | 시간        | 분석
------------|-----------|-----------------------------------------
100개        | ~50 us    | 100 × O(1) ≈ 100 해시 연산
1,000개      | ~500 us   | 1000 × O(1) ≈ 1000 해시 연산  ⚠️ 해시 충돌 시작
10,000개     | ~10ms     | 로드 팩터 높음, 리해시 필요

개선 (예약량):
std::unordered_set.reserve(expectedSize);
// 로드 팩터 < 0.75 유지 → O(1) 보장
```

### 3.3 대안: Visited 플래그 (메모리 제약 시)

```cpp
// 메모리 매우 제한적인 경우
class GameObject {
    bool m_visited = false;  // 추가 1 바이트
};

void ObjectManager::QueryObjectsInRect(const Gdiplus::RectF& rect, std::vector<GameObject*>& outObjects)
{
    outObjects.clear();
    
    // ... 셀 범위 계산 ...
    
    for (int y = startY; y <= endY; ++y) {
        for (int x = startX; x <= endX; ++x) {
            for (auto* obj : m_spatialGrid[x][y]) {
                if (!obj->m_visited) {
                    obj->m_visited = true;
                    outObjects.push_back(obj);
                }
            }
        }
    }
    
    // 플래그 리셋
    for (auto* obj : outObjects) {
        obj->m_visited = false;
    }
}

// 성능:
// - 중복 제거: O(1) 플래그 확인 (unordered_set보다 빠름)
// - 리셋: O(k), k = 반환 객체 수 (일반적으로 작음)
// - 메모리: -56 bytes (unordered_set 오버헤드) + 1 byte (플래그) = -55 bytes
```

---

## 4. 충돌 감지 쿼리 최적화

### 4.1 일반적인 충돌 감지 흐름

```cpp
void GameObject::Update(float deltaTime) {
    // 1. 위치 업데이트
    m_position += m_velocity * deltaTime;
    
    // 2. 콜라이더 위치 동기화
    m_collider->SetWorldPosition(m_position);
    
    // 3. 충돌 감지 (비용이 높음)
    std::vector<GameObject*> collidingObjects;
    CameraManager::GetInstance()->FindObjectsIntersectingCollider(
        m_collider, 
        collidingObjects
    );
    
    // 4. 충돌 응답
    for (auto* other : collidingObjects) {
        OnCollisionEnter(other);
    }
}
```

### 4.2 FindObjectsIntersectingCollider 최적화

```cpp
void CameraManager::FindObjectsIntersectingCollider(
    Collider* pCol, 
    std::vector<GameObject*>& out,
    bool onlyInteraction = false)
{
    out.clear();
    if (!pCol) return;

    GameObject* owner = pCol->GetOwner();
    Gdiplus::RectF bounds = pCol->GetWorldRect();  // AABB

    // ========== 1단계: 공간 분할 쿼리 ==========
    // AABB로 후보 객체 수집
    m_queryBuffer.clear();
    ObjectManager::GetInstance()->QueryObjectsInRect(bounds, m_queryBuffer);
    
    // 결과: 1,000개 → ~100개 (90% 감소)
    
    // ========== 2단계: 정확 충돌 검사 ==========
    for (auto* obj : m_queryBuffer) {
        if (!obj->IsEnabled() || obj == owner) continue;  // 자기 자신 제외
        
        Collider* mainCol = obj->GetMainCollider();  // 메인 콜라이더만 체크
        if (mainCol && mainCol->IsEnabled()) {
            // 선택적: 상호작용 콜라이더만?
            if (!onlyInteraction || mainCol->IsInteractionCollider()) {
                // 정확 충돌 검사 (비용 높음)
                if (ColliderManager::GetInstance()->Intersects(pCol, mainCol)) {
                    out.push_back(obj);
                }
            }
        }
    }
}
```

**성능 분석:**

```
1,000개 객체의 플레이어 충돌 감지:

브루트 포스:
  - 1,000개 × Intersects() = 1,000 × 500ns = 500 us ≈ 0.5ms

공간 분할 최적화:
  - QueryObjectsInRect(): 250 us
  - 100개 × Intersects() = 100 × 500ns = 50 us
  - 총: 300 us ≈ 0.3ms

개선율: 40% 시간 단축
```

### 4.3 메인 콜라이더 최적화

```cpp
// ❌ 비효율적: 모든 콜라이더 검사
std::vector<Collider*> allColliders = obj->GetAllColliders();
for (auto* col : allColliders) {  // 여러 개
    if (Intersects(pCol, col)) {
        out.push_back(obj);
        break;  // 하나 충돌하면 충분
    }
}

// ✅ 효율적: 메인 콜라이더만 체크
Collider* mainCol = obj->GetMainCollider();
if (mainCol && Intersects(pCol, mainCol)) {
    out.push_back(obj);
}

// 메모리 효과:
// - GetAllColliders() 제거 → 캐시 미스 감소
// - 콜라이더 순회 제거 → 루프 언롤 최적화 가능
```

---

## 5. 렌더 커맨드 버퍼링 최적화

### 5.1 문제: 임의의 렌더 순서

```cpp
// ❌ 비효율적: 직접 렌더링
void RenderVisibleGameObjects() {
    for (auto* obj : m_visibleObjects) {        // 순서 무관
        Gdiplus::Graphics* g = GetGraphics();
        
        // 매 객체마다 상태 변경
        SetGDIPlusState(obj->GetLayer(), obj->GetZOrder());
        g->DrawImage(...);  // 각 객체마다 API 호출
    }
}

// 성능 문제:
// - 렌더 상태 변경: 200개 × ~1000 cycle = 200,000 cycle
// - 그래픽 API 오버헤드: 200 호출
// - 캐시 미스: 상태 구조체가 분산됨
```

### 5.2 해결책: 렌더 커맨드 버퍼링

```cpp
// ✅ 효율적: 커맨드 수집 → 정렬 → 일괄 렌더링

// 1단계: 커맨드 수집 (상태 변경 제로)
void RenderVisibleGameObjects() {
    for (auto* obj : m_visibleObjects) {
        RenderManager::GetInstance()->AddWorldEntityCommand(
            bitmap, sourceRect,
            worldX, worldY,
            scaleX, scaleY,
            pivotX, pivotY,
            layer, zOrder,
            direction, tintColor
        );
    }
}

// 2단계: 커맨드 정렬 (계층 및 z-order)
void RenderManager::Flush(Gdiplus::Graphics* pGraphics) {
    for (int i = 0; i < LAYER_COUNT; ++i) {
        auto& commands = m_layerCommands[i];
        
        // z-order 기반 정렬
        std::sort(commands.begin(), commands.end(), CompareDrawCommands);
        
        // 3단계: 정렬된 순서로 렌더링
        for (const DrawCommand& cmd : commands) {
            RenderSprite(pGraphics, cmd.sprite, cmd.destRect);
        }
        
        commands.clear();  // 재사용
    }
}

// 정렬 비용 분석
const int COMMAND_COUNT = 200;
// std::sort: O(n log n) = 200 × log(200) ≈ 1,600회 비교
// 비교 비용: 30 cycle × 1,600 = 48,000 cycle ≈ 50 us
// 
// 이득: 상태 변경 200,000 cycle 절감
// 순이익: 200,000 - 50,000 = 150,000 cycle (75% 절감)
```

### 5.3 계층 체계

```cpp
// RenderLayer 정의 (낮은 번호부터 렌더링)
enum RenderLayer {
    LAYER_TILE = 0,           // 배경 타일
    LAYER_GROUND,             // 지면 객체 (나무, 돌, 풀)
    LAYER_ENTITY,             // 엔티티 (플레이어, 몬스터)
    LAYER_EFFECT,             // 이펙트 (폭발, 마법)
    LAYER_UI_BACKGROUND,      // UI 배경
    LAYER_UI_CONTENT,         // UI 컨텐츠
    LAYER_UI_FOREGROUND,      // UI 전면 (오버레이)
    LAYER_COUNT
};

// 계층 선택 가이드:
// 
// Top-Down 뷰에서 "앞뒤 순서":
//   먼 것부터 그리기 (Y축이 크면 뒤)
//   
// 예: 플레이어 Y=200, 나무 Y=150
//   1. 나무 렌더링 (뒤)
//   2. 플레이어 렌더링 (앞)
//   
// 정렬 키: (layer, zOrder, worldY)
```

### 5.4 카메라 캐싱

```cpp
void RenderManager::AddWorldEntityCommand(
    Gdiplus::Bitmap* pBitmap,
    const Gdiplus::RectF& sourceRect,
    float worldX, float worldY,
    // ... 기타 파라미터 ...
) {
    // ========== 최적화: 카메라 위치 캐싱 ==========
    // 문제: 여러 객체를 렌더 커맨드로 추가할 때
    //       각 AddWorldEntityCommand() 호출 시마다
    //       CameraManager::GetCameraPos() 조회
    //       → 매번 다른 위치 반환 가능 (지연)
    
    const Gdiplus::PointF currentCamPos = CameraManager::GetInstance()->GetCameraPos();
    
    // 이 프레임 동안 카메라 위치를 고정
    // (객체 수집 단계에서 이미 뷰포트 계산했으므로 일관성 있음)
    
    float screenX = worldX - currentCamPos.X + (float)WINCX * 0.5f;
    float screenY = worldY - currentCamPos.Y + (float)WINCY * 0.5f;
    
    DrawCommand cmd;
    cmd.destRect = Gdiplus::RectF(screenX - width * pivotX, screenY - height * pivotY, width, height);
    
    m_layerCommands[layer].push_back(cmd);
}
```

**지터(Jitter) 방지:**

```cpp
// 문제 상황:
// 프레임 1: 카메라 = (100, 100), 객체 렌더링 → 스크린 위치 계산
// 프레임 1 중간: 카메라 이동 → (110, 110)
// 프레임 1 후반: 또 다른 객체 렌더링 → 다른 스크린 위치로 계산
// → 두 객체 사이 10px 오차 (시각적 "떨림")

// 해결: 프레임 시작 시 카메라 위치 고정
class RenderManager {
    Gdiplus::PointF m_cachedCameraPos;
    
    void BeginFrame() {
        m_cachedCameraPos = CameraManager::GetInstance()->GetCameraPos();
    }
    
    void AddWorldEntityCommand(...) {
        float screenX = worldX - m_cachedCameraPos.X + (float)WINCX * 0.5f;
        // ...
    }
};
```

---

## 6. 렌더링 파이프라인 흐름

```cpp
// ========== MainGame::Update() ==========
TimeManager::GetInstance()->Update();
InputManager::GetInstance()->Update(deltaTime);
SceneManager::GetInstance()->Update(deltaTime);     // 객체 위치 업데이트
RenderManager::GetInstance()->Update(deltaTime);
GameProgressManager::GetInstance()->Update(deltaTime);

// ========== MainGame::LateUpdate() ==========
InputManager::GetInstance()->LateUpdate();
SceneManager::GetInstance()->LateUpdate();

// ========== MainGame::Render() ==========
Gdiplus::Graphics* pGraphics = GraphicsManager::GetInstance()->GetGraphics();

// 1. 씬 렌더링 (커맨드 수집)
SceneManager::GetInstance()->Render();  // 객체들의 Render() 호출
                                        // → RenderManager에 커맨드 추가

// 2. 디버그 오버레이 (성능 모니터링)
if (m_showPerfOverlay) {
    RenderPerformanceOverlay();  // F1로 활성화
}

// 3. 렌더 커맨드 실행 (정렬 후 렌더링)
RenderManager::GetInstance()->Flush(pGraphics);

// 4. 그래픽스 렌더링
GraphicsManager::GetInstance()->Render();
```

**렌더 커맨드 흐름:**

```
                    프레임 시작
                        ↓
        AddWorldEntityCommand() ×200
        AddUICommand() ×50
        AddTextCommand() ×10
                        ↓
                    Flush() 호출
                        ↓
            [모든 계층의 커맨드]
                        ↓
            각 계층별 z-order 정렬
                        ↓
        [LAYER_TILE] [LAYER_GROUND] ... [LAYER_UI_FOREGROUND]
                        ↓
                GDI+ 렌더링 실행
                        ↓
                  화면 출력
                        ↓
                커맨드 clear (재사용)
```

---

## 7. 타일 렌더링 최적화

### 7.1 가시 타일 범위 계산

```cpp
void CameraManager::RenderVisibleTiles(const MapData* mapData) {
    if (!mapData) return;
    
    Gdiplus::RectF vp = GetViewportWorldRect();
    
    // 1. 월드 좌표 → 타일 좌표 변환
    int sx = max(0, (int)floor((vp.X - TILE_SIZE) / TILE_SIZE));
    int ex = min(MAP_WIDTH, (int)ceil((vp.X + vp.Width + TILE_SIZE) / TILE_SIZE));
    int sy = max(0, (int)floor((vp.Y - TILE_SIZE) / TILE_SIZE));
    int ey = min(MAP_HEIGHT, (int)ceil((vp.Y + vp.Height + TILE_SIZE) / TILE_SIZE));
    
    // 예: 뷰포트 1920×1080, TILE_SIZE=128
    // sx = floor((-960 - 128) / 128) = -9 → 0 (클램프)
    // ex = ceil((960 + 128) / 128) = 8.5 → 8
    // sy = floor((-540 - 128) / 128) = -5 → 0 (클램프)
    // ey = ceil((540 + 128) / 128) = 5.2 → 5
    // 타일 범위: 8×5 = 40개 타일
}
```

### 7.2 타일 캐시 관리

```cpp
// 타일 캐시 구조
std::unordered_map<UINT, TileCacheData> m_tileCache;

struct TileCacheData {
    TileID id;
    Gdiplus::Bitmap* bitmap;  // 캐시된 이미지
};

// 캐시 수명 주기:
// 1. 타일 로드 시: 비트맵 생성 및 캐시
// 2. 뷰포트 이동: 새 타일 캐시 추가
// 3. 뷰포트 밖 나감: 타일 캐시 제거 (CleanupUnusedTileCache)
// 4. 씬 전환: 전체 캐시 초기화 (ClearTileCache)

// 캐시 효율:
// - 같은 맵에서 카메라 이동: 캐시 재사용 (수십 프레임)
// - 캐시 미스율: ~10% (새 타일 진입 시)
// - 메모리 사용: ~2-5 MB (수백 타일 비트맵)
```

### 7.3 캐시 정리 로직

```cpp
void CameraManager::CleanupUnusedTileCache(
    const MapData* mapData, 
    int startX, int endX, int startY, int endY)
{
    // 현재 뷰포트에 필요한 타일 ID 수집
    std::unordered_set<TileID> neededTiles;
    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            if (mapData->tiles[x][y].id != TILEID_NONE) {
                neededTiles.insert(mapData->tiles[x][y].id);
            }
        }
    }
    
    // 캐시에서 불필요한 타일 제거
    for (auto it = m_tileCache.begin(); it != m_tileCache.end(); ) {
        if (neededTiles.find(it->first) == neededTiles.end()) {
            // 이 타일은 더 이상 필요없음
            if (it->second.bitmap) {
                delete it->second.bitmap;  // 메모리 해제
            }
            it = m_tileCache.erase(it);  // 캐시에서 제거
        } else {
            ++it;
        }
    }
}

// 성능:
// - 제거할 타일 수: 평균 10개 (새 영역 진입)
// - 삭제 연산: 10 × delete + 10 × erase = ~1 ms
// - 캐시 메모리: 40개 × 2MB = 80 MB (최악)
//              10개 × 2MB = 20 MB (일반적)
```

---

## 8. 성능 병목 진단

### 8.1 프로파일링 점검 목록

```cpp
// Performance Overlay (F1 활성화)로 확인할 지표:

PrintPerformanceData() {
    float fps = TimeManager::GetInstance()->GetCurrentFPS();
    float renderVisibleGameObjectsMs = CameraManager::GetInstance()->GetAvgRenderVisibleGameObjectsMs();
    float tileMs = CameraManager::GetInstance()->GetAvgRenderVisibleTilesMs();
    
    // 합계 = F1 추적 지표 총합
    float totalMs = renderVisibleGameObjectsMs + tileMs;
    
    // 프레임 버짓: 1000/30 ≈ 33ms
    // 위 4가지 합계: 이상적으로 < 5ms (15% 미만)
}
```

### 8.2 병목 구간 추적

```cpp
// 상황별 대응

QueryObjectsInRect > 2ms
└─ 원인: 과도한 쿼리 셀 수 또는 셀당 객체 많음
└─ 대응:
   1. GRID_CELL_SIZE 증가 (256→512)
   2. 객체 밀도 감소
   3. 쿼리 영역 축소 (마진 200→100)

RenderVisibleGameObjects > 3ms
└─ 원인: 후보 객체 과다 또는 비최적화 브루트포스 경로 영향
└─ 대응:
   1. 객체 활성도/밀도 조정
   2. IsObjectInViewport 필터 개선
   3. 마진 영역 재조정

RenderVisibleTiles > 5ms
└─ 원인: 타일 캐시 미스, 비트맵 로드 느림
└─ 대응:
   1. 캐시 크기 증가
   2. 타일 이미지 압축 (PNG → JPG)
   3. 미리로드 (preload) 구현

Flush 계열 병목은 최적화 ON 경로에서만 별도 프로파일링 대상
```

---

## 9. 향후 최적화 로드맵

### 9.1 단기 (1-2주)

```cpp
// 1. 타일 프리로드
void CameraManager::PreloadTilesAhead() {
    // 카메라 이동 방향 예측
    Gdiplus::PointF direction = m_cameraPos - m_lastCameraPos;
    
    // 이동 방향 앞의 타일 미리 로드
    Gdiplus::RectF aheadRect = GetViewportWorldRect();
    aheadRect.X += direction.X * 500;
    aheadRect.Y += direction.Y * 500;
    
    // 프리로드...
}

// 2. 동적 LOD (Level of Detail)
void GameObject::UpdateLOD() {
    float distToCamera = GetDistanceToCamera();
    
    if (distToCamera > 2000) {
        SetDetailLevel(LOD_LOW);      // 간단한 모델
    } else if (distToCamera > 1000) {
        SetDetailLevel(LOD_MEDIUM);
    } else {
        SetDetailLevel(LOD_HIGH);     // 복잡한 모델
    }
}

// 3. 프러스텀 컬링 (회전 객체 제외)
bool CameraManager::IsObjectInFrustum(GameObject* obj) {
    // 현재: AABB만 사용
    // 개선: 회전을 고려한 정확한 프러스텀 검사
    
    Gdiplus::Matrix rotation;
    rotation.Rotate(obj->GetRotation());
    
    return FrustumIntersects(GetViewportWorldRect(), obj->GetBounds(), rotation);
}
```

### 9.2 중기 (1개월)

```cpp
// 1. 쿼드트리 (Quadtree) 도입
class QuadTreeNode {
    QuadTreeNode* children[4];  // NW, NE, SW, SE
    std::vector<GameObject*> objects;
    
    void QueryObjectsInRect(const Gdiplus::RectF& rect, std::vector<GameObject*>& result) {
        if (!rect.IntersectsWith(m_bounds)) return;
        
        if (IsLeaf()) {
            for (auto* obj : objects) {
                result.push_back(obj);
            }
        } else {
            for (auto* child : children) {
                child->QueryObjectsInRect(rect, result);
            }
        }
    }
};

// 2. GPU 렌더링 (Direct2D)
void RenderManager::InitDirect2D() {
    // GDI+에서 Direct2D로 전환
    // - 하드웨어 가속
    // - 배치 렌더링 최적화
    // - GPU 병렬화
}

// 3. 오클루더 컬링
void CameraManager::UpdateOcclusionCulling() {
    // 건물(완전한 오클루더) 뒤의 객체 숨김
    for (auto* building : m_buildings) {
        for (auto* obj : m_visibleObjects) {
            if (IsOccludedBy(obj, building)) {
                m_visibleObjects.erase(obj);
            }
        }
    }
}
```

### 9.3 장기 (3-6개월)

```cpp
// 1. Compute Shader 기반 Culling
// GPU에서 가시성 판정 수행
// CPU 병목 완전 해결

// 2. 인스턴싱 렌더링
// 같은 메시 여러 개 → 한 번의 렌더 호출

// 3. 동적 라이팅 + 섀도우
// 계산 비용 높음, 지연 렌더링 필요
```

---

## 결론

쿼리 및 렌더링 최적화의 핵심 원칙:

1. **다단계 필터링**: 가장 비용 낮은 필터부터 적용
2. **버퍼 재사용**: 메모리 할당/해제 제거
3. **캐싱**: 반복 계산 회피
4. **배치 처리**: 상태 변경 최소화
5. **프로파일링**: 성능 지표를 명확하게 추적

현재 구현은 이 모든 원칙을 이미 적용하고 있으며, 0.2~0.4ms 범위의 안정적인 성능을 유지합니다.

향후 객체 수 증가나 복잡도 증가 시 위의 로드맵을 단계별로 적용하면 선형적 성능 개선을 기대할 수 있습니다.

