# 마우스 상호작용 로직 아키텍처 가이드

## 1. 개요
Don't Starve 클론 프로젝트에서 플레이어가 마우스로 월드 내 오브젝트(나무, 돌, 적 등)를 클릭하여 상호작용하는 전체적인 흐름에 대한 가이드입니다. 

입력 감지부터 실제 상호작용 애니메이션과 판정이 일어나기까지 크게 **4단계의 과정**을 거치며, 중간에 공간 분할 알고리즘을 활용한 최적화가 적용되어 있습니다.

---

## 2. 1단계: 마우스 입력 감지 (`InputManager`)

마우스 클릭은 운영체제에서 발생하는 이벤트를 기반으로 프레임 단위로 감지됩니다.

*   **메시지 기반 감지**: 
    `InputManager::ProcessMouseMessage`가 `WM_LBUTTONDOWN` / `WM_RBUTTONDOWN` 윈도우 메시지를 수신하면, `s_lButtonClickedThisFrame`과 같은 플래그를 `true`로 설정하여 **즉각적인 클릭**을 감지합니다. 폴백으로 `GetAsyncKeyState`도 사용합니다.
*   **좌표 변환**: 
    획득한 화면 상의 마우스 좌표(`Screen 좌표`)는 `CameraManager::ScreenToWorld` 함수를 통해 카메라 위치를 반영한 실제 게임 내 `World 좌표`로 변환됩니다.

---

## 3. 2단계: 대상 탐색 및 상호작용 준비 (`Player::TryStartInteraction`)

`Player::Update()`에서 `InputManager::IsLButtonClicked()`가 `true`일 때 실행되는 핵심 로직입니다.

### 3.1. UI 블로킹 체크
```cpp
if (ObjectManager::GetInstance()->IsScreenPointBlockedByUI(sx, sy)) return;
```
클릭한 좌표가 인벤토리나 기타 UI 위에 있다면, 월드 상호작용 로직을 즉시 취소합니다.

### 3.2. 공간 분할 기반 대상 쿼리 (최적화)
```cpp
float range = 100.0f;
Gdiplus::RectF queryRect(worldX - range, worldY - range, range * 2, range * 2);
cameraManager->QueryObjectsInteractive(queryRect, queryResults, true);
```
전체 월드 객체를 순회하는 대신, **클릭한 위치 주변(반경 100px)의 객체만 제한적으로 쿼리**합니다. 이때 내부적으로 `ObjectManager`의 **그리드 공간 분할(Spatial Partitioning)** 시스템이 사용되어 탐색 비용(O(1)~O(k))을 극적으로 낮춥니다.

### 3.3. 정밀 충돌 검사 및 Z-Order 판정
```cpp
if (mainCol && mainCol->ContainsPoint(worldX, worldY)) {
    float curY = obj->GetComponent<Transform>()->GetY();
    if (!target || curY > maxY) {
        target = obj;
        maxY = curY;
    }
}
```
*   가져온 후보 객체 중, 마우스 좌표가 **실제 `Collider` 내부에 있는지(`ContainsPoint`) 정밀 검사**를 수행합니다.
*   여러 객체가 겹쳐 있다면, **Y좌표가 가장 큰(화면상 가장 앞쪽에 그려지는) 객체**를 최종 상호작용 대상(`target`)으로 선정합니다.

### 3.4. 상호작용 가능 여부 판별
선택된 타겟이 어떤 종류(`GameObjectType`)인지, 그리고 플레이어가 현재 장착한 도구(`m_equippedItemID`)가 무엇인지 검사합니다.
*   **나무(NORMAL_TREE)**: 도끼(`GOID_TOOL_RED_AXE` 등) 필요
*   **돌(NORMAL_ROCK)**: 곡괭이(`GOID_TOOL_PICKAXE` 등) 필요
*   조건이 충족되면 `m_pendingInteractionTarget`에 대상을 임시 저장하고, 해당 객체의 위치로 이동을 시작합니다.

---

## 4. 3단계: 이동 및 상태 전이 (`Player::Update` -> `Player::OnInteraction`)

대상을 클릭한 후, 플레이어는 해당 대상에게 접근해야 실제로 행동을 시작할 수 있습니다.

1.  **목표 지점 이동**: `m_pendingInteractionTarget`의 위치를 향해 캐릭터가 이동합니다.
2.  **도착 판정**: 목표물과의 거리가 상호작용 사거리 내로 좁혀지면 이동을 멈춥니다.
3.  **타겟 확정**: `m_pendingInteractionTarget`을 비우고, 이를 `m_activeInteractionTarget`으로 승격시킵니다. 캐릭터의 시선(방향)을 타겟 쪽으로 맞춥니다.
4.  **상태(FSM) 전이**: 
    ```cpp
    bool Player::OnInteraction(GameObject* obj)
    ```
    대상의 종류에 따라 플레이어의 상태를 `PlayerState::CHOP`(벌목), `MINE`(채광), `PICKUP`(줍기), `ATTACK`(공격) 등으로 변경합니다.

---

## 5. 4단계: 결과 처리 (Animation Event)

상태가 전이되었다고 해서 곧바로 자원이 캐지거나 몬스터가 피해를 입는 것은 아닙니다. 애니메이션과 시각적 피드백을 동기화하기 위해 **애니메이션 이벤트**를 활용합니다.

1.  **애니메이션 재생**: 상태에 맞는 애니메이션(예: 곡괭이로 내려찍기)이 시작됩니다.
2.  **타격점(Hit) 처리**: 애니메이션 프레임 중 도구가 실제로 물체에 닿는 순간, 델리게이트를 통해 `Player::OnMineHit()`, `Player::OnChopHit()` 등이 호출됩니다.
    *   이때 대상 객체의 `Damaged()` 함수가 호출되어 내구도가 감소하거나 부서집니다.
3.  **동작 종료**: 애니메이션이 끝나는 프레임에 `Player::OnMineEnd()` 등이 호출되어 `m_activeInteractionTarget`을 초기화하고 플레이어를 `IDLE` 상태로 복귀시킵니다.

---

## 6. 요약

마우스 상호작용 로직은 아래와 같이 유기적으로 연결되어 있습니다.

> 마우스 클릭 (`InputManager`) ➔ 반경 내 후보 선별 및 정밀 충돌 검사 (`CameraManager` & `Collider`) ➔ 조건 판별 후 이동 시작 (`TryStartInteraction`) ➔ 도착 시 상태 전이 (`OnInteraction`) ➔ 애니메이션 타격 프레임에서 실제 결과 적용 (`OnMineHit` 등)