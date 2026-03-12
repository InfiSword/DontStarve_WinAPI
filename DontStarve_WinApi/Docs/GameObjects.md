# 게임 오브젝트 및 컴포넌트 (GameObjects & Components)

## 1. GameObject 계층 구조

프로젝트의 모든 게임 내 객체는 `GameObject`를 최상위로 하여 다음과 같은 계층을 형성합니다.

### 1.1 GameObject (Base)
- **설명**: 모든 객체의 기반 클래스입니다. 컴포넌트 관리, 코루틴 시스템, 기본 생명주기를 정의합니다.
- **주요 속성**: `GameObjectID`, `GameObjectType`, `Transform`, `Components List`.
- **핵심 메서드**: `AddComponent<T>()`, `GetComponent<T>()`, `StartCoroutine()`, `OnInteraction()`.

### 1.2 Entity (Living Objects)
- **설명**: 살아있거나 상호작용 가능한 월드 내 객체입니다. 체력(HP)과 상태 관리 시스템을 포함합니다.
- **주요 속성**: `HP`, `MaxHP`, `State`, `Animator`, `Collider`.
- **하위 클래스**:
  - **Combatant**: 전투가 가능한 객체(`Player`, `Monster`)입니다. 공격 범위, 쿨다운, 공격 콜라이더 관리 로직을 추가합니다.
  - **NaturalEnvironment**: 나무, 돌 등 자원 채집이 가능한 환경 객체입니다.

### 1.3 UIElement (UI Objects)
- **설명**: 화면(Screen Space)에 배치되는 UI 객체입니다.
- **특징**: 일반적인 `Transform` 대신 화면 정렬 및 크기 조절을 위한 `RectTransform`을 사용합니다.
- **하위 클래스**: `UIImage`, `UIButton`, `UIText`, `InventoryUI`, `CraftingUI`.

### 1.4 Item (World Items)
- **설명**: 월드에 드롭된 아이템 객체입니다. 획득 시 인벤토리 데이터로 변환됩니다.
- **특징**: 엔티티보다는 가볍고, 시각적 정보와 이름, 설명 정보를 포함합니다.

---

## 2. 컴포넌트 시스템 (Component System)

각 `GameObject`는 필요한 기능을 컴포넌트 형태로 부착하여 사용합니다.

### 2.1 Transform (변환)
- 모든 `GameObject`에 기본적으로 포함됩니다 (UI는 `RectTransform`).
- 위치(`x, y`), 크기(`scaleX, scaleY`), 회전(`rotation`), 피벗(`pivotX, pivotY`) 정보를 관리합니다.

### 2.2 SpriteRenderer (렌더링)
- 스프라이트를 화면에 그리는 역할을 담당합니다.
- `RenderManager`와 협력하여 `DrawCommand`를 생성합니다.
- `SortingLayer`, `SortKey`(Y-Sorting)를 지정할 수 있습니다.

### 2.3 Animator (애니메이션)
- `SpriteSheet`를 사용하여 프레임 기반 애니메이션을 수행합니다.
- 상태 기반 애니메이션 전환 및 이벤트 처리를 지원합니다.

### 2.4 Collider (충돌체)
- 객체의 충돌 영역을 정의합니다.
- `BoxCollider`, `CircleCollider` 타입이 존재하며, `ColliderManager`에 의해 관리됩니다.

---

## 3. 상호작용 시스템 (Interaction System)

`GameObject::OnInteraction(GameObject* target)` 메서드를 오버라이드하여 객체 간의 상호작용을 정의합니다.

- **Player -> Resource**: 자원을 채집하거나 아이템을 획득합니다.
- **Player -> Monster**: 공격을 하거나 데미지를 받습니다.
- **Player -> Item**: 아이템을 인벤토리에 추가합니다.
- **Player -> Building**: 제작 메뉴를 열거나 특정 기능을 활성화합니다.
