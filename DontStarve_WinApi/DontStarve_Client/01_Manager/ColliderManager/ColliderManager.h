#pragma once

class GameObject;    
class Collider;

class ColliderManager : public CSingleTon<ColliderManager>
{
    friend class CSingleTon<ColliderManager>;
public:
    ColliderManager();
    ~ColliderManager();

    void Init();
    void LateInit();                     
    void Update(float deltaTime);         // 모든 게임오브젝트의 콜라이더 위치 업데이트
    void LateUpdate();                    // 충돌 검사 처리 
    void Release();                    

    void AddCollider(Collider* pCollider);
    void RemoveCollider(Collider* pCollider);

    // 충돌 처리
    bool CheckCollision(GameObject* obj1, GameObject* obj2);

	// 콜라이더 쌍 전용 충돌 검사 헬퍼
	bool Intersects(Collider* a, Collider* b);

private:
    std::vector<Collider*> m_colliders; // 등록된 콜라이더들
    std::vector<GameObject*> m_queryBuffer; // 공간 분할 쿼리용 재사용 버퍼 (성능 최적화용)
};
