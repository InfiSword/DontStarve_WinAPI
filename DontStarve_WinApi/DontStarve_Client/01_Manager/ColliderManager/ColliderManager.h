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
      void Update(float deltaTime);         // 위치 갱신 단계(현재는 개별 콜라이더가 자기 위치를 관리)
      void LateUpdate();                    // 브로드페이즈(공간 쿼리) + 내로우페이즈(실교차) 처리
    void Release();                    

    void AddCollider(Collider* pCollider);
    void RemoveCollider(Collider* pCollider);

      // GameObject 단위 편의 검사: 메인 콜라이더만 사용
    bool CheckCollision(GameObject* obj1, GameObject* obj2);

      // 콜라이더 쌍 전용 검사: Box/ Circle/ 혼합 조합을 단일 진입점으로 처리
	bool Intersects(Collider* a, Collider* b);

private:
      std::vector<Collider*> m_colliders; // 등록된 콜라이더들
      std::vector<GameObject*> m_queryBuffer; // 매 프레임 재사용 버퍼(할당/해제 비용 절감)
};
