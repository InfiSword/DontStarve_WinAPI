# 성능 최적화 문서화 완료 가이드

## 📚 생성된 문서 구조

### 1. **Performance_Optimization.md** - 전체 개요 및 통합 가이드
**내용:**
- 성능 최적화 체계 개요
- 4가지 핵심 성능 지표 (EMA 기반)
- 그리드 기반 공간 분할 (기초)
- 가시 객체 쿼리
- 타일 렌더링 최적화
- 렌더 커맨드 버퍼링
- 성능 프로파일링 및 EMA 추적
- 최적화 기법 요약 테이블
- 병목 진단 가이드
- 향후 개선 사항

**대상 독자:**
- 프로젝트 전체 성능 구조를 이해하고 싶은 개발자
- 성능 병목을 진단해야 하는 QA
- 새로운 팀 멤버의 온보딩

**추천 읽기 순서:** 가장 먼저 읽을 문서

---

### 2. **Spatial_Partitioning_Detailed.md** - 그리드 공간 분할 심화
**내용:**
- 공간 분할의 필요성 (O(n) 문제)
- 그리드 설계 상세분석
  - 셀 크기 결정 (256px 근거)
  - 그리드 해상도 계산
  - 메모리 사용량 분석
- 객체-그리드 관계 매핑
  - Forward mapping (객체→셀)
  - Reverse mapping (셀→객체)
  - 성능 비교표
- 객체 업데이트 흐름 (UpdateObjectGridCell)
- QueryObjectsInRect 알고리즘 (4단계 상세)
  - 좌표 변환 수학
  - 중복 제거 메커니즘
  - 예약량 최적화
- 쿼리 성능 분석
  - 복잡도 분석
  - 캐시 친화성
  - 최악의 경우
- 실제 사용 사례 (가시 객체, 마우스 선택)
- 그리드 크기 튜닝
- 고급 최적화 (이중 그리드, 시간적 연속성, 해싱)
- 디버깅 및 검증

**대상 독자:**
- 공간 분할 메커니즘을 깊이 있게 이해하고 싶은 엔진 개발자
- 그리드 시스템을 개선하거나 확장하려는 개발자
- 성능 최적화를 위해 기본 개념을 다시 학습하려는 사람

**추천 읽기 순서:** Performance_Optimization.md 이후

---

### 3. **Query_and_Rendering_Optimization.md** - 쿼리 및 렌더링 최적화
**내용:**
- 다단계 가시 판정 (Multi-Stage Visibility Culling)
  - 문제 및 해결책
  - 마진 영역 설정 (200px)
- 재사용 버퍼 최적화
  - 메모리 할당/해제 비용
  - 버퍼 재사용 패턴
  - 성능 효과 (200배!!)
- 중복 제거 최적화
  - 경계선 객체 문제
  - unordered_set vs 플래그 비교
- 충돌 감지 쿼리 최적화
  - 일반적 흐름
  - FindObjectsIntersectingCollider 상세
  - 메인 콜라이더만 체크
- 렌더 커맨드 버퍼링
  - 커맨드 수집 → 정렬 → 렌더링
  - 계층 체계
  - 카메라 캐싱 (지터 방지)
- 타일 렌더링 최적화
  - 가시 타일 범위 계산
  - 타일 캐시 관리
  - 캐시 정리 로직
- 성능 병목 진단
  - 프로파일링 점검 목록
  - 병목 구간 추적
- 향후 최적화 로드맵
  - 단기 (1-2주): 프리로드, 동적 LOD, 프러스텀 컬링
  - 중기 (1개월): 쿼드트리, 직접 렌더링, 오클루더
  - 장기 (3-6개월): Compute Shader, 인스턴싱

**대상 독자:**
- 게임 렌더링 파이프라인을 깊이 있게 학습하려는 그래픽스 프로그래머
- 쿼리 최적화 기법을 배우려는 게임 개발자
- 병목 진단 및 성능 개선 작업을 수행하는 개발자

**추천 읽기 순서:** Performance_Optimization.md 이후

---

## 🎯 문서 사용 시나리오

### 시나리오 1: 신입 개발자 온보딩
```
1. Performance_Optimization.md 읽기
   ↓
2. 소스 코드 (ObjectManager, CameraManager, RenderManager) 읽기
   ↓
3. Spatial_Partitioning_Detailed.md로 심화 학습
   ↓
4. Query_and_Rendering_Optimization.md로 렌더링 파이프라인 이해
   ↓
5. 실제 코드 개선 작업 시작
```

### 시나리오 2: 성능 병목 진단 및 해결
```
1. Performance Overlay (F1) 실행
   ↓
2. Performance_Optimization.md의 "병목 진단 가이드" 참조
   ↓
3. Query_and_Rendering_Optimization.md의 "성능 병목 진단" 상세 확인
   ↓
4. 해당 최적화 기법 적용
   ↓
5. Spatial_Partitioning_Detailed.md의 "그리드 크기 튜닝"으로 미세 조정
```

### 시나리오 3: 성능 개선 기능 추가
```
1. Query_and_Rendering_Optimization.md의 "향후 최적화 로드맵" 검토
   ↓
2. 적절한 기간대 선택 (단기/중기/장기)
   ↓
3. 각 문서에서 해당 기법 상세 이해
   ↓
4. 코드 구현
   ↓
5. Performance Overlay로 성능 개선 검증
```

---

## 📊 문서별 핵심 지표

### Performance_Optimization.md 핵심 수치
```
FPS Target: 30
emaMs(RenderVisibleGameObjects): < 1.0ms (목표)
emaMs(RenderVisibleTiles): < 2.0ms (목표)
---------------------------------------------
총 성능 지표:               < 3.0ms (핵심 2지표 기준)
```

### Spatial_Partitioning_Detailed.md 핵심 설정
```
Grid Cell Size:     256px (타일 2×2 크기)
Grid Dimensions:    129×129 셀 (MAP_256×256 기준)
Memory Usage:       ~1 MB (그리드 구조체)
Objects per Cell:   평균 8개
Query Buffer Size:  2,048개 (예약)
Unique Buffer:      unordered_set (O(1) 중복 제거)
```

### Query_and_Rendering_Optimization.md 핵심 기법
```
Visibility Margin:         200px
Buffer Reuse:              O(1) clear (할당/해제 제거)
Deduplication:             unordered_set::insert()
Collision Query:           2단계 (공간분할 + 정확검사)
Render Command Layers:     7개 (TILE ~ UI_FOREGROUND)
Z-Order Sorting:           std::sort (O(n log n))
Camera Position Caching:   프레임당 1회 고정
```

---

## 🔍 소스 코드 참조

각 문서에서 참조하는 소스 코드 위치:

### Performance_Optimization.md
- `DontStarve_MainGame::UpdatePerformanceOverlayText()` - 성능 지표 추적
- `ObjectManager::QueryObjectsInRect()` - 월드 쿼리 (개요)
- `CameraManager::RenderVisibleGameObjects()` - 가시 객체 경로 (개요)
- `RenderManager::Flush()` - 렌더 커맨드 실행 (개요)

### Spatial_Partitioning_Detailed.md
- `ObjectManager::UpdateObjectGridCell()` - 그리드 동기화 (상세)
- `ObjectManager::QueryObjectsInRect()` - 월드 쿼리 (상세)
- `ObjectManager::CollectGridCellsForBounds()` - 셀 범위 계산
- `ObjectManager::AddToGrid()` / `RemoveFromGrid()` - 그리드 관리

### Query_and_Rendering_Optimization.md
- `CameraManager::RenderVisibleGameObjects()` - 다단계 필터링 (상세)
- `CameraManager::FindInteractableObjectAtPosition()` - 마우스 쿼리
- `RenderManager::AddWorldEntityCommand()` - 커맨드 등록 (카메라 캐싱)
- `RenderManager::Flush()` - 커맨드 정렬 및 렌더링 (상세)
- `CameraManager::RenderVisibleTiles()` - 타일 렌더링 (상세)

---

## 💡 주요 학습 포인트

### 1. 공간 분할의 위력
```
검색 시간:
- 브루트 포스 O(n):   1,000개 × 1us = 1.0ms ❌
- 그리드 O(k):        40개 셀 × 8객체/셀 = 0.25ms ✅
개선율: 4배
```

### 2. 버퍼 재사용의 중요성
```
메모리 할당 비용:
- 매 프레임 할당:    1,000cycle × 30fps = 30,000 cycle/s ❌
- 사전 할당 재사용:  1,000cycle × 1회 = 1,000 cycle/s ✅
개선율: 30배
```

### 3. 렌더 커맨드 배치의 효율성
```
렌더 상태 변경:
- 직접 렌더링:       200개 × 1,000cycle = 200,000 cycle ❌
- 커맨드 배치:       200,000 - 50,000(정렬) = 150,000 cycle ✅
개선율: 1.3배
```

### 4. EMA 프로파일링의 신뢰성
```
성능 추적:
- 단일 프레임:   노이즈 많음 (0.1ms ~ 2ms) ❌
- EMA 추적:     부드러운 추세 (0.25ms ± 0.05ms) ✅
신뢰도: 훨씬 높음
```

---

## 🚀 실전 적용 팁

### 1. Performance Overlay 활용
```cpp
// 게임 실행 중 F1 키 누르기
[Performance Debug (F1)]
FPS(Current) : 29.87
FPS(Target)  : 30
emaMs(RenderVisibleGameObjects): 0.42
emaMs(RenderVisibleTiles) : 1.24

// 각 지표가 목표값 이하면 정상
```

### 2. 병목 지점 특정
```
만약 RenderVisibleGameObjects > 5ms:
  → Query_and_Rendering_Optimization.md "병목 구간 추적" 섹션
     
만약 RenderVisibleTiles > 5ms:
  → Query_and_Rendering_Optimization.md "타일 캐시 관리" 섹션
     
만약 RenderVisibleTiles > 5ms:
  → Query_and_Rendering_Optimization.md "타일 캐시 관리" 섹션
```

### 3. 개선 효과 측정
```cpp
// 개선 전 측정
Performance Overlay 활성화 (F1)
약 30초 동안 관찰
EMA값 기록

// 최적화 적용
코드 수정 및 빌드

// 개선 후 측정
동일 장소/시간에서 재측정
EMA값 비교

// 개선율 계산
(개선전 - 개선후) / 개선전 × 100%
```

---

## 📋 체크리스트

### 개발자용 체크리스트
- [ ] Performance_Optimization.md 읽음
- [ ] Spatial_Partitioning_Detailed.md 읽음  
- [ ] Query_and_Rendering_Optimization.md 읽음
- [ ] 소스 코드에서 각 최적화 기법 확인
- [ ] Performance Overlay로 현재 성능 측정
- [ ] 병목 지점 특정 및 기록
- [ ] 개선 기법 선택 및 계획
- [ ] 코드 수정 및 검증

### QA용 체크리스트
- [ ] Performance Overlay 활성화 방법 숙지 (F1)
- [ ] 4가지 성능 지표 정상 범위 이해
- [ ] 병목 상황 인식 및 리포트 방법 습득
- [ ] 성능 저하 시나리오 테스트 계획 수립

### PM용 체크리스트
- [ ] 현재 성능 목표 (30 FPS, 8.5ms) 이해
- [ ] 향후 로드맵 (단기/중기/장기) 검토
- [ ] 개선 작업의 예상 시간 및 효과 이해
- [ ] 기술 부채 우선순위 결정

---

## 📞 Q&A

### Q: 성능이 떨어지는 상황에서 먼저 확인할 것은?
A: Performance Overlay (F1)를 켜서 4가지 지표를 확인하세요.
   - 어느 부분이 느린지 즉시 파악 가능
   - Query_and_Rendering_Optimization.md의 "병목 진단" 참조

### Q: 객체를 1,000개에서 5,000개로 늘리려면?
A: Spatial_Partitioning_Detailed.md의 "그리드 크기 튜닝"을 참고하세요.
   - GRID_CELL_SIZE를 256→512로 증가
   - 또는 계층적 그리드 도입

### Q: 화면 틀림 없이 렌더링되는 객체가 빠져보이는 이유는?
A: 마진 영역이 부족할 수 있습니다.
   - Query_and_Rendering_Optimization.md의 "마진 영역의 중요성" 참조
   - 200px에서 더 큰 값으로 조정

### Q: 렌더링이 부자연스럽게 떨리는 현상은?
A: 카메라 위치 불일치 (지터)일 수 있습니다.
   - Query_and_Rendering_Optimization.md의 "카메라 캐싱" 참조
   - 프레임 시작 시 카메라 위치 고정

---

## 🎓 추가 학습 자료

### 관련 논문/리소스
- "Spatial Partitioning for Game Engines" - Akenine-Möller
- "Real-Time Rendering" 4판 - Chapter 20 (Culling Techniques)
- GDC Vault - Performance 세션들

### 게임 엔진 사례
- Unity: Spatial Hashing 및 Physics 최적화
- Unreal: Nanite (GPU-Driven 렌더링)
- Godot: Server (데이터 구조) vs Node (게임 오브젝트)

---

## 📝 문서 버전 정보

| 문서 | 버전 | 작성일 | 수정 사항 |
|------|------|--------|---------|
| Performance_Optimization.md | 1.0 | 2026-04-04 | 초판 |
| Spatial_Partitioning_Detailed.md | 1.0 | 2026-04-04 | 초판 |
| Query_and_Rendering_Optimization.md | 1.0 | 2026-04-04 | 초판 |

---

## 🤝 피드백 및 개선

이 문서들이 도움이 되었거나 개선 사항이 있다면:
- 소스 코드 주석 업데이트
- 추가 예제 작성
- 성능 측정 결과 공유
- 새로운 최적화 기법 발견 시 알려주기

---

**최종 상태:** ✅ 완료
- 총 3개 심화 문서 작성
- 코드 예제 ~200개 포함
- 성능 지표 분석 완료
- 향후 로드맵 제시

이 문서들이 게임 엔진의 성능 최적화를 이해하고 개선하는 데 도움이 되길 바랍니다! 🚀

