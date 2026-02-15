#pragma once

#include "Object.h"
#include "Component/Component.h"
#include <utility>

class Transform;
class SpriteRenderer;

class GameObject : public Object
{
protected:
	GameObjectID m_id;				// 오브젝트 아이디
	GameObjectType m_type;			// 오브젝트 타입

	std::wstring m_name;					// 해당 게임 오브젝트 이름
	bool m_isInteractive;			// 상호작용 가능 여부
	bool m_bReleased;				// Release() 호출 여부 (중복 호출 방지)
									
    // 컴포넌트 관리					
    std::vector<Component*> m_components;

public:
    
	GameObject(GameObjectType type, GameObjectID id, 
		const std::wstring& resourcePath = L"", const std::wstring& imageName = L"", 
		bool isActive = true, bool isInteractive = false);
 
	virtual ~GameObject();

	virtual void Init();
	virtual void LateInit();
	virtual void Update(float deltaTime); 
	virtual void LateUpdate();
	virtual void Release();

	// 상호작용 관련
	virtual void OnInteraction(GameObject* obj);
	virtual bool CanInteract() const { return m_isInteractive; }

    template <typename T, typename... Args>
    T* AddComponent(Args&&... args) {
        T* newComponent = new T(this, std::forward<Args>(args)...);
        m_components.push_back(newComponent);
        newComponent->Init();
        return newComponent;
    }

    template <typename T>
    T* GetComponent() const {
        // Release()된 GameObject의 컴포넌트에 접근하지 않음 (안전성)
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

	// inline 함수
	inline GameObjectID GetID() const { return m_id; }
	inline GameObjectType GetType() const { return m_type; }
	inline const std::wstring& GetName() const { return m_name; }
};
