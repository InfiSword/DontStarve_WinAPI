#pragma once

#include "Object.h"
#include "Component/Component.h"
#include <utility>

class Transform;
class SpriteRenderer;

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
	bool m_bReleased;				// Release() 호출 여부 (중복 호출 방지)
	GameObjectType m_type;					// 게임 오브젝트 타입
    // 컴포넌트 관리					
    std::vector<Component*> m_components;

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
	virtual void Render() {}
	virtual void Release();

	// UI 여부 반환 (dynamic_cast 대체용)
	virtual bool IsUI() const { return false; }

	// 코루틴 시스템
	void StartCoroutine(CoroutineHandle coroutine);
	void StopAllCoroutines();

	virtual void RenderDebugOverlay() {}

	// 상호작용 관련
	virtual bool OnInteraction(GameObject* obj);
	virtual void Damaged(int damage) {}
	virtual bool CanInteract() const { return m_isInteractive; }
	virtual void SetInteractive(bool interactive) { m_isInteractive = interactive; }

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
        if (m_bReleased) {
            return nullptr;
        }
        for (Component* component : m_components) {
            if (!component) continue; // nullptr 체크
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
		if (m_bReleased) return result;
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
