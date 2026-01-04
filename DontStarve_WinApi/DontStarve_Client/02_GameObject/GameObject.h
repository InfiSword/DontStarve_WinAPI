#pragma once

#include "Object.h"
#include "Component/Component.h"

class Transform;
class SpriteRenderer;

class GameObject : public Object
{
protected:
	GameObjectID m_id;				// 오브젝트 아이디
	GameObjectType m_type;			// 오브젝트 타입

	std::wstring m_name;					// 해당 게임 오브젝트 이름
    std::wstring m_resourcePath;			// 해당 리소스 경로
    std::wstring m_spriteResourceName;		// ~~~.png
	std::wstring m_description;				// 해당 오브젝트 설명 (필요시)
									
    // 컴포넌트 관리					
    std::vector<Component*> m_components;

	bool m_isInteractive;			// 상호작용 가능 여부 (Object의 m_enabled와는 독립)

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

	virtual void OnInteraction(GameObject* obj);
	
    template <typename T>
    T* AddComponent() {
        T* newComponent = new T(this);
        m_components.push_back(newComponent);
        newComponent->Init();
        return newComponent;
    }

    template <typename T>
    T* GetComponent() const {
        for (Component* component : m_components) {
            T* target = dynamic_cast<T*>(component);
            if (target) {
                return target;
            }
        }
        return nullptr;
    }

	// inline 함수
	inline std::wstring GetSpriteResourceName() const { return m_spriteResourceName; }
	inline GameObjectID GetID() const { return m_id; }
	inline GameObjectType GetType() const { return m_type; }
	inline const std::wstring& GetName() const { return m_name; }
	inline const std::wstring& GetDescription() const { return m_description; }
	inline bool IsInteractive() const { return m_isInteractive; }
};
