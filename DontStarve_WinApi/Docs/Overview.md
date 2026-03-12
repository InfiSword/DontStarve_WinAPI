# DontStarve_WinApi 프로젝트 개요

본 프로젝트는 WinAPI와 GDI+를 사용하여 제작된 'Don't Starve' 스타일의 서바이벌 액션 게임 엔진입니다. Unity와 유사한 **Manager-GameObject-Component** 아키텍처를 채택하여 확장성과 유지보수성을 높였습니다.

## 1. 핵심 아키텍처 (Core Architecture)

### 1.1 싱글톤 매니저 (Singleton Managers)
시스템의 주요 기능(렌더링, 입력, 리소스, 씬 관리 등)은 `CSingleTon<T>`을 상속받은 매니저 클래스들이 담당합니다. 이들은 전역적인 상태를 관리하며 게임 루프의 각 단계(Update, LateUpdate, Render)에서 호출됩니다.

### 1.2 게임 오브젝트 및 컴포넌트 시스템 (GameObject & Component)
- **GameObject**: 모든 게임 내 객체의 최상위 클래스입니다. `AddComponent<T>()`, `GetComponent<T>()`를 통해 기능을 동적으로 확장할 수 있습니다.
- **Component**: `Transform`, `SpriteRenderer`, `Animator`, `Collider` 등 실제 기능을 수행하는 단위입니다.

### 1.3 코루틴 시스템 (Coroutine System)
`GameObject`는 간단한 코루틴 시스템을 지원하여, 시간 지연이 필요한 로직(예: 공격 딜레이, 상태 지속 시간)을 비동기 스타일로 작성할 수 있습니다.

---

## 2. 주요 시스템 흐름 (System Flow)

### 2.1 게임 루프 (Main Loop)
1. **InputManager**: 사용자 입력을 처리하고 상태를 갱신합니다.
2. **TimeManager**: 프레임 간 델타 타임(DeltaTime)을 계산합니다.
3. **SceneManager**: 현재 활성화된 씬의 `Update`를 호출합니다.
4. **ObjectManager**: 모든 활성화된 `GameObject`의 `Update` 및 `LateUpdate`를 호출합니다.
5. **CollisionManager**: 콜라이더 간의 충돌을 검사하고 이벤트를 발생시킵니다.
6. **RenderManager**: 모든 렌더링 명령을 레이어별로 수집, 정렬 후 화면에 출력합니다.

### 2.2 렌더링 파이프라인 (Rendering Pipeline)
`RenderManager`는 즉시 그리는 방식이 아닌 **커맨드 기반 렌더링**을 사용합니다.
1. 각 오브젝트가 `SpriteRenderer`를 통해 `DrawCommand`를 `RenderManager`에 등록합니다.
2. `RenderLayer`별로 명령이 분류됩니다 (Background, World, UI 등).
3. `sortKey`(주로 Y 좌표)를 기준으로 정렬하여 **Y-Sorting(Depth)**을 처리합니다.
4. 프레임 끝에 `Flush()`를 통해 일괄적으로 렌더링합니다.

---

## 3. 디렉토리 구조 (Directory Structure)
- `00_MainGame`: 엔트리 포인트 및 메인 게임 루프
- `01_Manager`: 시스템 전반을 관리하는 싱글톤 클래스들
- `02_GameObject`: 게임 객체 계층 구조 (Entity, UI, Item, Building)
- `03_Animation`: 스프라이트 시트 및 애니메이션 제어 로직
- `Header`: 전역 상수, 열거형, 구조체 정의
- `Resource`: 이미지 및 데이터 파일
