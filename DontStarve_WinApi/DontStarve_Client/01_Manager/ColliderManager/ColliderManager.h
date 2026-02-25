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
    void RenderGizmos();                  // 디버그용 콜라이더 Gizmo 그리기 (RenderManager 명령 큐 사용)
    void Release();                    

    void AddCollider(Collider* pCollider);
    void RemoveCollider(Collider* pCollider);

    // 충돌 처리
    bool CheckCollision(GameObject* obj1, GameObject* obj2);

	// 콜라이더 쌍 전용 충돌 검사 헬퍼
	bool Intersects(Collider* a, Collider* b);

private:
    std::vector<Collider*> m_colliders; // 등록된 콜라이더들
};
