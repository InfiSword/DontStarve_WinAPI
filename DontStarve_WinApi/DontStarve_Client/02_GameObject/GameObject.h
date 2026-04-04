#pragma once

#include "Object.h"
#include "Component/Component.h"
#include <utility>
#include <type_traits>

class Transform;
class SpriteRenderer;
class Collider;

// 코루틴 핸들: deltaTime을 받아 실행 중이면 true, 완료되면 false를 반환하는 함수
using CoroutineHandle = std::function<bool(float)>;

enum GameObjectType
{
	GO_TYPE_NONE = 0,
	GO_TYPE_NATURAL_ENVIRONMENT,
	GO_TYPE_MONSTER,
	GO_TYPE_BUILDING,
	GO_TYPE_ITEM,
	GO_TYPE_PLAYER,
	GO_TYPE_UI,
};

class GameObject : public Object
{
protected:
	GameObjectID m_id;				// 오브젝트 아이디

	std::wstring m_name;					// 해당 게임 오브젝트 이름
	bool m_isInteractive;			// 상호작용 가능 여부
	GameObjectType m_type;					// 게임 오브젝트 타입
    // 컴포넌트 관리					
    std::vector<Component*> m_components;

	// 메인(몸통) 콜라이더 캐싱 (매 프레임 GetComponents 방지)
	Collider* m_mainCollider = nullptr;

	// 캐싱된 바운딩 박스 (성능 최적화용)
	Gdiplus::RectF m_cachedBounds = { 0, 0, 0, 0 };
	bool m_isBoundsDirty = true;

	// 공간 분할용 그리드 좌표
	int m_gridCellX = -1;
	int m_gridCellY = -1;

	bool m_isDead = false; // 삭제 여부 플래그

private:
	std::vector<CoroutineHandle> m_coroutines;

public:
    
	GameObject(GameObjectID id, 
		const std::wstring& resourcePath = L"", const std::wstring& imageName = L"", 
		bool isActive = true, bool isInteractive = false);
 
	virtual ~GameObject() override;

	virtual void Init();
	virtual void LateInit();
	virtual void Update(float deltaTime); 
	virtual void LateUpdate();
	virtual void Render();
	virtual void Release();

	// 삭제 관련
	bool IsDead() const { return m_isDead; }
	void SetDead(bool dead) { m_isDead = dead; }

	// UI 여부 반환 (dynamic_cast 대체용)
	virtual bool IsUI() const { return false; }

	// 공간 분할용 접근자
	int GetGridCellX() const { return m_gridCellX; }
	int GetGridCellY() const { return m_gridCellY; }
	void SetGridCell(int x, int y) { m_gridCellX = x; m_gridCellY = y; }

	// 메인 콜라이더 접근자
	virtual Collider* GetMainCollider() const { return m_mainCollider; }
	void SetMainCollider(Collider* col) { if(!m_mainCollider) m_mainCollider = col; }

	// 코루틴 시스템
	void StartCoroutine(CoroutineHandle coroutine);
	void StopAllCoroutines();

	virtual void RenderDebugOverlay() {}
	virtual void MainColliderGizmo();

	// 상호작용 관련
	virtual bool OnInteraction(GameObject* obj);
	virtual void OnCollision(GameObject* other) {}
	virtual void Damaged(int damage) {}
	virtual bool CanInteract() const { return m_isInteractive; }
	virtual void SetInteractive(bool interactive) { m_isInteractive = interactive; }

	// 바운딩 박스 관련
	Gdiplus::RectF GetBounds();
	void SetBoundsDirty() { m_isBoundsDirty = true; }

	// 디버그/시각화 오버레이 
	static bool g_bRenderDebugOverlay;	

    template <typename T, typename... Args>
    T* AddComponent(Args&&... args) {
        T* newComponent = new T(this, std::forward<Args>(args)...);
        m_components.push_back(newComponent);

        newComponent->Init();
        return newComponent;
    }

    template <typename T>
    T* GetComponent() const 
	{
        for (Component* component : m_components) {
            if (!component) continue; 
            T* target = dynamic_cast<T*>(component);
            if (target) {
                return target;
            }
        }
        return nullptr;
    }

	template <typename T>
	std::vector<T*> GetComponents() const {
		std::vector<T*> result;
		for (Component* component : m_components) {
			if (!component) continue;
			T* target = dynamic_cast<T*>(component);
			if (target) result.push_back(target);
		}
		return result;
	}

	// inline 함수
	inline GameObjectID GetID() const { return m_id; }
	inline GameObjectType GetType() const { return m_type; }
	inline const std::wstring& GetName() const { return m_name; }

private:
	void UpdateCoroutines(float deltaTime);
};
