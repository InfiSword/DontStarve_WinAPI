#pragma once
class GameObject;    
class CameraManager;

class Collider {
public:
    enum Type { RECTANGLE, CIRCLE };
    Type m_type;
    GameObject* m_pOwner; // 이 콜라이더를 소유한 GameObject
    RECT m_boundingBox;   // AABB 

    Collider(GameObject* owner, RECT boundingBox) : m_pOwner(owner), m_boundingBox(boundingBox), m_type(RECTANGLE) {}
    virtual ~Collider() = default;

    // 충돌 감지 메서드
    virtual bool Intersects(const POINT& point) {
        return PtInRect(&m_boundingBox, point); 
    }
};

class ColliderManager : public CSingleTon<ColliderManager>
{
    friend class CSingleTon<ColliderManager>;
public:
    ColliderManager();
    ~ColliderManager();

    void Init();
    void LateInit();                     
    void Update(float deltaTime);         // 동적 오브젝트 콜라이더 위치 갱신
    void LateUpdate();                    // 충돌 검사 수행 
    void Render(Gdiplus::Graphics* pGraphics); 
    // 디버그용 콜라이더 영역 그리기
    void Release();                    

    void AddCollider(Collider* pCollider);
    void RemoveCollider(Collider* pCollider);

    // 충돌 처리
    GameObject* CheckPointCollision(POINT screenPos); 
    // 화면 좌표로 클릭된 GameObject 찾기
    
    // GameObject* CheckRectCollision(RECT screenRect); 
    // 사각형 영역과 충돌하는 GameObject 찾기

private:
    std::vector<Collider*> m_colliders; // 관리할 콜라이더들
};