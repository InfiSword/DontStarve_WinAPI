#pragma once
#include "Collider.h"

class GameObject;

// 사각형 콜라이더
class BoxCollider : public Collider {
public:
    RECT m_boundingBox;   // 로컬 좌표 (상대 좌표)

    BoxCollider(GameObject* owner);
    virtual ~BoxCollider() = default;

    // 충돌 검사 메서드 구현
    virtual bool IntersectsCollider(const Collider* other) const override;
    
    // 월드 좌표로 변환된 boundingBox 가져오기
    virtual RECT GetWorldBoundingBox() const override;
    virtual bool ContainsPoint(float worldX, float worldY) const override;
    virtual void GetCenterWorld(float& outX, float& outY) const override;
    
    // 콜라이더 설정
    void SetBoundingBox(int offsetX, int offsetY, int width, int height);
    
    // Gizmo 렌더링
    virtual void RenderGizmo() override;
};
