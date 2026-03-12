# 매니저 클래스 (Managers) 명세

모든 매니저 클래스는 `CSingleTon<T>`을 상속받아 전역적으로 접근 가능한 싱글톤 패턴으로 구현되어 있습니다.

## 1. SceneManager (씬 매니저)
- **역할**: 현재 활성화된 씬을 관리하고 씬 전환 로직을 담당합니다.
- **특징**: Unity 스타일의 지연 씬 로딩(`PendingSceneType`)을 사용하여 프레임 끝에 안전하게 전환합니다.
- **주요 기능**:
  - `LoadTitleScene()`, `LoadCharacterSelectScene()`, `LoadGameScene()`
  - `ProcessPendingSceneLoad()`: 씬 전환 요청을 실제로 실행

## 2. ObjectManager (오브젝트 매니저)
- **역할**: 활성화된 모든 `GameObject`의 수명주기(Update, LateUpdate, Render, Release)를 통합 관리합니다.
- **특징**: 팩토리 패턴을 사용하여 `GameObjectID`를 기반으로 객체를 생성하며, 삭제 지연 큐를 통해 순회 중 안전한 삭제를 지원합니다.
- **주요 기능**:
  - `CreateGameObject()`: ID별 팩토리 함수 호출을 통해 객체 생성
  - `AddGameObject()`, `RemoveGameObject()`: 리스트 관리
  - `GetPlayer()`: 플레이어 객체 캐싱 및 접근 제공

## 3. RenderManager (렌더 매니저)
- **역할**: 렌더링 명령(`DrawCommand`)을 수집하고 레이어별 정렬 후 화면에 출력합니다.
- **특징**: 커맨드 패턴 기반의 렌더링 시스템을 사용하여 복잡한 스프라이트 겹침 문제를 해결합니다.
- **주요 기능**:
  - `AddDrawCommand()`: 스프라이트 그리기 명령 등록
  - `Flush()`: 수집된 모든 레이어의 명령을 정렬 후 GDI+로 출력
  - `AddUIImageCommand()`: UI 전용 렌더링 명령 제공

## 4. CameraManager (카메라 매니저)
- **역할**: 월드 좌표와 화면 좌표 간의 변환을 처리하며 뷰포트 컬링(Culling)을 수행합니다.
- **특징**: 타겟 팔로우 모드와 맵 경계 제한(Clamping) 기능을 포함합니다.
- **주요 기능**:
  - `WorldToScreen()`, `ScreenToWorld()`: 좌표 변환
  - `UpdateVisibleObjects()`: 뷰포트 내 오브젝트만 선별하여 갱신 성능 최적화
  - `RenderVisibleTiles()`: 타일 캐싱 및 보이는 타일 렌더링

## 5. ResourceManager (리소스 매니저)
- **역할**: 이미지(Sprite) 및 애니메이션(SpriteSheet) 리소스를 로드하고 캐싱하여 메모리 낭비를 방지합니다.
- **특징**: `std::shared_ptr`와 `std::weak_ptr`를 사용한 스마트 포인터 캐싱 시스템을 갖추고 있습니다.
- **주요 기능**:
  - `LoadSprite()`, `LoadSpriteSheet()`: 캐싱 기반 리소스 로드
  - `GetObjectResourceInfo()`: ID별 데이터 정보(경로, 피벗 등) 조회

## 6. UIManager (UI 매니저)
- **역할**: 화면에 표시되는 UI 요소들을 관리하며 사용자 입력을 처리합니다.
- **특징**: UI 요소에 의한 마우스 클릭 차단(`IsScreenPointBlockedByUI`) 기능을 제공합니다.
- **주요 기능**:
  - `AddUIImage()`, `AddUIButton()`, `AddUIText()`: UI 요소 등록
  - `SetUIVisibility()`: 전체 UI 가시성 제어

## 7. InventoryManager (인벤토리 매니저)
- **역할**: 플레이어의 아이템 소유 상태, 제작 레시피, 아이템 사용 로직을 관리합니다.
- **특징**: 파일 입출력을 통한 인벤토리 저장/로드 기능과 제작 레시피 파싱을 담당합니다.
- **주요 기능**:
  - `TryGainItemFromWorldObject()`: 월드 객체에서 아이템 획득
  - `TryCraftItem()`: 제작 레시피 검사 및 아이템 생성
  - `SaveInventoryToFile()`, `LoadInventoryFromFile()`

## 8. InputManager / TimeManager / ColliderManager
- **InputManager**: 키보드 및 마우스 입력을 상태별(Down, Up, Pressed)로 관리
- **TimeManager**: 고정 델타 타임 및 실제 프레임 타임 계산
- **ColliderManager**: 레이어별 충돌 체크 및 `OnCollisionEnter/Stay/Exit` 이벤트 발생
