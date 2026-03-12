# 핵심 게임 시스템 (Core Game Systems)

## 1. 인벤토리 및 제작 시스템 (Inventory & Crafting)

### 1.1 인벤토리 구조
- 플레이어의 인벤토리는 `Inventory` 클래스에 의해 데이터가 관리되며, `InventoryUI`를 통해 화면에 출력됩니다.
- 아이템은 `Item` 객체로 존재하거나 인벤토리 내의 데이터 형태(ID, 수량 등)로 관리됩니다.

### 1.2 아이템 제작 (Crafting)
- `InventoryManager`는 `game_progress.txt`나 하드코딩된 레시피 맵을 로드합니다.
- 제작 로직:
  1. `CraftingUI`에서 제작 버튼 클릭.
  2. `InventoryManager::TryCraftItem()` 호출.
  3. `Inventory::HasEnoughMaterials()`로 재료 확인.
  4. 재료 소모 및 새로운 `Item` 생성 후 인벤토리에 추가.

---

## 2. 렌더링 및 뷰포트 최적화 (Rendering & Viewport Optimization)

### 2.1 뷰포트 컬링 (Viewport Culling)
- `CameraManager`는 매 프레임 현재 카메라 뷰포트 내에 들어오는 오브젝트만 선별하여 `m_visibleObjects` 목록에 추가합니다.
- `Update`와 `Render` 루프는 이 선별된 오브젝트들만을 대상으로 수행되어 성능을 최적화합니다.

### 2.2 Y-Sorting (Depth)
- `Don't Starve` 스타일의 쿼터뷰(Top-Down) 시점을 구현하기 위해 객체의 Y 좌표를 기반으로 렌더링 순서를 결정합니다.
- `RenderManager`는 등록된 `DrawCommand`를 `sortKey`(= Y좌표) 기준으로 정렬하여 바닥부터 머리 순으로 그려냅니다.

### 2.3 타일 렌더링 (Tile Rendering)
- 맵 데이터는 거대한 2D 타일 배열로 구성됩니다.
- 화면에 보이는 타일 범위만을 계산하여 그리는 `RenderVisibleTiles()` 기능을 통해 맵 크기에 관계없이 일정한 렌더링 성능을 유지합니다.

---

## 4. 애니메이션 시스템 (Animation System)

### 4.1 SpriteSheet 관리
- 하나의 큰 이미지 파일에 여러 프레임이 담긴 `SpriteSheet`를 `ResourceManager`를 통해 관리합니다.
- `LoadSpriteSheet()`는 동일한 리소스를 여러 객체가 공유할 수 있도록 스마트 포인터 캐싱을 사용합니다.

### 4.2 상태 기반 애니메이션 (State-based Animation)
- `Animator` 컴포넌트는 `Entity`의 상태(`Idle`, `Walk`, `Attack`, `Death`)에 따라 애니메이션 클립을 자동으로 전환합니다.
- 특정 프레임에서 이벤트를 발생시켜(예: 공격 판정 프레임) 로직과 연동할 수 있습니다.

---

## 5. 데이터 저장 및 로드 (Data Persistence)

### 5.1 맵 데이터 (.dsm)
- 맵 에디터를 통해 생성된 타일 정보, 오브젝트 배치 정보가 `.dsm` 바이너리 또는 텍스트 파일로 저장됩니다.
- `SceneManager`는 게임 시작 시 모든 맵 데이터를 로드하여 빠른 씬 전환을 지원합니다.

### 5.2 게임 진행도 (Game Progress)
- 플레이어의 인벤토리 상태, 잠금 해제된 캐릭터, 게임 클리어 조건 등은 `game_progress.txt`에 저장됩니다.
- `GameProgressManager`가 이를 전담하여 처리합니다.
