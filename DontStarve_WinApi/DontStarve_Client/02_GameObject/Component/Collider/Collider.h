#pragma once
#include "../Component.h"

class GameObject;    

// 콜라이더 비활성화(SetColliderEnabled(false)) 시 상호작용 클릭 감지·충돌 검사·Gizmo에서 제외됩니다.
class Collider : public Component {
protected:
    bool m_mapColliderDataValid;  
    int m_mapColliderOffsetX;
    int m_mapColliderOffsetY;
    int m_mapColliderWidth;
    int m_mapColliderHeight;

public:
    Collider(GameObject* owner);
    virtual ~Collider() = default;

    virtual void Init() override;
    virtual void Release() override;
    virtual void Update(float deltaTime) override;

    // 콜라이더 전용 활성화 제어 (상호작용 클릭 방지 시 SetColliderEnabled(false) 사용)
    void SetColliderEnabled(bool enabled) { SetActive(enabled); }
    bool IsColliderEnabled() const { return IsEnabled(); }

    virtual bool IntersectsCollider(const Collider* other) const = 0;  
    virtual RECT GetWorldBoundingBox() const = 0;
    // 월드 좌표 점이 콜라이더 내부에 있는지 (상호작용 범위 판정 등에 사용)
    virtual bool ContainsPoint(float worldX, float worldY) const = 0;
    // 콜라이더의 월드 좌표 중심 (이동 목표 등에 사용)
    virtual void GetCenterWorld(float& outX, float& outY) const = 0;
    
    // 맵 데이터의 콜라이더 정보 설정
    void SetMapColliderData(bool hasCollider, int offsetX, int offsetY, int width, int height);
    
    // 맵 데이터의 콜라이더 정보 가져오기
    bool HasMapColliderData() const { return m_mapColliderDataValid; }
    void GetMapColliderData(int& offsetX, int& offsetY, int& width, int& height) const;

    virtual void RenderGizmo() = 0;
};
