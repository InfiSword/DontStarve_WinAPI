#pragma once

#include "Component/Component.h"

class GameObject
{
protected:
    float m_x, m_y;
    float m_width, m_height;	    // 비트맵(스프라이트시트) 크기
	GameObjectID m_id;
	GameObjectType m_type;
	RenderLayer m_layer;
	Direction m_direction;

	std::wstring m_name;		// 해당 게임 오브젝트 이름
    std::wstring resourcePath;	// 해당 리소스 경로
    std::wstring imageName;		// ~~~.png
	std::wstring m_description;	// 해당 오브젝트 설명 (필요시)
	Gdiplus::Bitmap* m_bitmap;

    // 컴포넌트 관리
    std::vector<Component*> m_components;

	float m_pivotX;
	float m_pivotY;

	bool m_isActive;
	bool m_isInteractive;

public:
    GameObject(GameObjectType type, GameObjectID id, float x, float y, float pivotX, float pivotY, Direction dir, const std::wstring& resourcePath = L"", const std::wstring& imageName = L"");
 
	virtual ~GameObject();

	virtual void Init();
	virtual void LateInit();
	virtual void Update(float deltaTime); // 이동 등
	virtual void LateUpdate();
	virtual void Release();

	virtual void OnInteraction(GameObject* obj);
	
	// 비트맵 로드 - 하위 클래스에서 override 가능
	virtual void LoadBitmap();
	
    template <typename T>
    T* AddComponent() {
        // Unity 스타일: 컴포넌트를 동적으로 추가
        // 중복 허용 (Unity와 동일한 방식)
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

	// Getters
	Gdiplus::Bitmap* GetBitmap() const { return m_bitmap; }

	virtual Gdiplus::RectF GetWorldBoundingBox() const { return Gdiplus::RectF(m_x - m_width * m_pivotX, m_y - m_height * m_pivotY, m_width, m_height); }

	RenderLayer GetRenderLayer() const { return m_layer; }

	virtual float GetSortKey(RenderLayer layer) const { return static_cast<float>(layer) + m_y; }

	std::wstring GetImageName() const { return imageName; }
	GameObjectID GetID() const { return m_id; }
	GameObjectType GetType() const { return m_type; }
	const std::wstring& GetName() const { return m_name; }
	const std::wstring& GetDescription() const { return m_description; }
	bool GetActive() const { return m_isActive; }
	float GetX() const { return m_x; }
	float GetY() const { return m_y; }
	float GetWidth() const { return m_width; }
	float GetHeight() const { return m_height; }
	float GetPivotX() const { return m_pivotX; }
	float GetPivotY() const { return m_pivotY; }
	Direction GetDir() const { return m_direction; }

	// Setters
	void SetPivot(float pivotX, float pivotY) { m_pivotX = pivotX; m_pivotY = pivotY; }
	void SetActive(bool active) { m_isActive = active; }
	void SetPosition(float x, float y) { m_x = x; m_y = y; }
	
	// 상호작용 관련 메서드들
	void SetInteractive(bool interactive) { m_isInteractive = interactive; }
	bool IsInteractive() const { return m_isInteractive; }
	virtual bool CanInteract() const { return m_isActive && m_isInteractive; }
};
