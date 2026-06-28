# 코루틴 시스템과 Tree 객체의 쉐이킹 이펙트 분석

## 1. 개요
Don't Starve 클론 프로젝트의 `GameObject` 클래스는 `std::function`과 C++ 람다(Lambda)를 활용한 **경량 코루틴(Coroutine) 시스템**을 구현하고 있습니다. 이 코루틴은 매 프레임 `deltaTime`을 전달받아 비동기적인 시간 흐름이나 애니메이션, 타이머 동작을 별도의 상태 변수 없이 지역 변수(Capture)만으로 깔끔하게 처리할 수 있도록 돕습니다.

대표적인 사용 사례로, `Tree` 객체(나무)가 타격을 받았을 때 발생하는 **쉐이킹(Shaking) 효과**가 이 코루틴 시스템을 통해 어떻게 구현되어 있는지 살펴봅니다.

---

## 2. GameObject의 코루틴 시스템 동작 원리

### 2.1. 코루틴 타입 정의
```cpp
// DontStarve_Client/02_GameObject/GameObject.h
using CoroutineHandle = std::function<bool(float)>;
```
코루틴은 **"매 프레임 `deltaTime`(float)을 인자로 받고, 계속 실행되어야 하는지(bool)를 반환하는 함수"**로 정의됩니다. 반환값이 `true`이면 다음 프레임에 또 호출되고, `false`를 반환하면 코루틴 목록에서 제거됩니다.

### 2.2. 코루틴의 등록 및 관리
```cpp
// DontStarve_Client/02_GameObject/GameObject.h
std::vector<CoroutineHandle> m_coroutines;

void GameObject::StartCoroutine(CoroutineHandle coroutine);
void GameObject::StopAllCoroutines();
void GameObject::UpdateCoroutines(float deltaTime);
```
*   `m_coroutines`: 해당 게임 오브젝트에서 실행 중인 코루틴들을 저장하는 벡터입니다.
*   `StartCoroutine`: 새로운 코루틴(람다)을 벡터에 추가합니다.
*   `StopAllCoroutines`: 벡터를 비워 실행 중이던 모든 코루틴을 강제로 중단시킵니다.
*   `UpdateCoroutines`: 매 프레임 `GameObject::Update()` 등에서 호출되며, 리스트의 코루틴들을 실행합니다.

### 2.3. 코루틴 업데이트 로직 (Swap-and-Pop 최적화)
```cpp
// DontStarve_Client/02_GameObject/GameObject.cpp
void GameObject::UpdateCoroutines(float deltaTime)
{
    if (m_coroutines.empty()) return;

    size_t i = 0;
    while (i < m_coroutines.size()) {
        bool stillRunning = m_coroutines[i](deltaTime); // 코루틴 실행
        
        if (!stillRunning) {
            // 코루틴이 종료(false 반환)되면 O(1) 방식으로 제거
            if (i == m_coroutines.size() - 1) {
                m_coroutines.pop_back();
            }
            else {
                m_coroutines[i] = std::move(m_coroutines.back());
                m_coroutines.pop_back();
            }
        }
        else {
            ++i; // 계속 실행 상태면 다음 코루틴으로
        }
    }
}
```
중간에 코루틴이 끝났을 때 벡터를 당기는 `erase` 연산(O(N)) 대신, 맨 마지막 원소를 현재 위치로 가져오고 `pop_back()`하는 **Swap-and-Pop 패턴(O(1))**을 사용하여 성능을 최적화했습니다.

---

## 3. Tree 객체의 쉐이킹 이펙트 구현

플레이어가 도끼로 나무를 찍었을 때(`Damaged`), 나무가 좌우로 흔들리는 시각적 피드백(Shaking)을 코루틴을 이용해 구현합니다. 별도의 `Update()` 내부에 복잡한 타이머 분기문을 작성할 필요 없이, 타격 시점에 직관적으로 로직을 구성할 수 있습니다.

### 3.1. 타격 이벤트 발생 (`Damaged`)
```cpp
// DontStarve_Client/02_GameObject/Entity/Enviorment/Tree.cpp
void Tree::Damaged(int damage)
{
    // ... 체력 감소 처리 ...
    
    StopAllCoroutines(); // 기존에 진행 중이던 흔들림 중단
    m_isShaking = true;

    // 쉐이킹에 필요한 기준 좌표 및 설정값 복사 (람다 캡처용)
    float baseX = m_baseX;
    float baseY = m_baseY;
    float elapsed = 0.0f;
    float duration = m_shakeDuration; // 예: 0.2초
    float amount = m_shakeAmount;     // 흔들리는 진폭
    float speed = m_shakeSpeed;       // 흔들리는 속도
    Transform* tr = m_transform;

    // 코루틴 시작
    StartCoroutine([=](float dt) mutable -> bool {
        // 하단 3.2 람다 본문 참고
    });
}
```
*   `mutable` 키워드: 람다 내에서 캡처한 값(예: `elapsed`)을 변경하며 상태를 유지할 수 있게 해줍니다. 캡처된 변수들이 하나의 독립적인 상태계(State) 역할을 합니다.

### 3.2. 쉐이킹 로직 (람다 내부)
```cpp
StartCoroutine([=](float dt) mutable -> bool {
    elapsed += dt;

    // 1. 종료 조건
    if (elapsed >= duration) {
        if (tr) tr->SetPosition(baseX, baseY); // 원래 위치로 복구
        m_isShaking = false;
        return false; // false를 반환하여 코루틴 종료
    }

    // 2. 흔들림 연산
    if (tr) {
        // 시간에 따라 감쇄하는 쉐이킹 효과 (갈수록 진폭이 작아짐)
        float currentAmount = amount * (1.0f - (elapsed / duration));
        
        // sin 함수를 이용한 좌우 진동
        float offsetX = sinf(elapsed * speed) * currentAmount;
        
        tr->SetPosition(baseX + offsetX, baseY);
    }
    
    return true; // 계속 진행
});
```

*   **감쇄 진동(Damped Oscillation)**: `(1.0f - (elapsed / duration))`을 통해 흔들림의 세기(`currentAmount`)가 시간이 지날수록 선형적으로 줄어들게 만들어 자연스러운 물리적 피드백을 제공합니다.
*   **사인파 진동(`sinf`)**: `elapsed * speed`를 라디안으로 하여 부드러운 좌우 진동(`offsetX`)을 만들어냅니다.

---

## 4. 코루틴 시스템의 장점

이러한 코루틴 방식(`std::function` + `mutable` 람다)은 다음과 같은 장점이 있습니다.

1.  **가독성**: `Update()` 함수 내부에 복잡한 상태 변수(`float m_shakeTimer`, `bool m_isShaking`, `float m_shakeAmount` 등)를 클래스 멤버로 선언하고 분기(if-else)를 만들 필요 없이, 행위가 발생하는 지점(`Damaged()`)에 로직을 응집시킬 수 있습니다.
2.  **캡슐화**: 쉐이킹에만 필요한 임시 변수(`elapsed`, `baseX`)가 람다 내부에 캡처되어 외부에서 접근할 수 없는 안전한 상태를 가집니다.
3.  **유연성**: Tree 쉐이킹뿐만 아니라 "1초 뒤에 오브젝트 파괴", "N초간 색상 깜빡임" 등 시간 기반의 일회성 이벤트를 매우 쉽게 작성할 수 있습니다. (예: `SpiderEgg`나 `IceProjectile` 등에서도 활용)