#pragma once

class Animator;
class AnimationClip;
class Player;

class GameObject
{
protected:
    float m_x, m_y;
    float m_width, m_height;	    // 비트맵(스프라이트) 크기
	GameObjectID m_id;
	GameObjectType m_type;
	Direction m_direction;

	std::wstring m_name;		// 순수한 게임 오브젝트 이름
    std::wstring resourcePath;	// 해당 리소스 경로
    std::wstring imageName;		// ~~~.png
	std::wstring m_description;	// 해당 오브젝트 설명 (필요하면)
	Gdiplus::Bitmap* m_bitmap;

    Animator* m_animator;
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
	virtual void Render(Gdiplus::Graphics* pGraphics); // RenderManager에서 처리하므로 개별 호출 불필요
	virtual void Release();

	virtual void OnInteraction(GameObject* obj);
	virtual Gdiplus::RectF GetWorldBoundingBox();
	
	// 비트맵 반환 - 애니메이션 있는 클래스는 Animator에서, 정적 이미지는 직접 인덱
	virtual Gdiplus::Bitmap* GetBitmap() const;
	
	// 비트맵 로드 - 하위 클래스에서 override 가능
	virtual void LoadBitmap();

	// Unity Animator 스타일 애니메이션 관련 virtual 메소드들 (필요한 클래스에서 구현)
	virtual void UpdateAnimation(float deltaTime) {}
	
	// Animator 접근자
	Animator* GetAnimator() const { return m_animator; }

	// Getters
	std::wstring GetImageName() const;
	GameObjectID GetID() const;  
	GameObjectType GetType() const;
	const std::wstring& GetName() const;
	const std::wstring& GetDescription() const;
	bool GetActive() const;
	float GetX() const;
	float GetY() const;
	float GetWidth() const ;
	float GetHeight() const ;
	float GetPivotX() const;
	float GetPivotY() const;
	Direction GetDir() const;

	// Setters
	void SetPivot(float pivotX, float pivotY);
	void SetActive(bool active);
	void SetPosition(float x, float y);
	
	// 상호작용 관련 메서드들
	void SetInteractive(bool interactive) { m_isInteractive = interactive; }
	bool IsInteractive() const { return m_isInteractive; }
	virtual bool CanInteract() const { return m_isActive && m_isInteractive; }
};