#pragma once
#include "../Component.h"

class GameObject;    

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

    virtual bool IntersectsCollider(const Collider* other) const = 0;  
    virtual RECT GetWorldBoundingBox() const = 0;
    
    // 맵 데이터의 콜라이더 정보 설정
    void SetMapColliderData(bool hasCollider, int offsetX, int offsetY, int width, int height);
    
    // 맵 데이터의 콜라이더 정보 가져오기
    bool HasMapColliderData() const { return m_mapColliderDataValid; }
    void GetMapColliderData(int& offsetX, int& offsetY, int& width, int& height) const;

    virtual void RenderGizmo() = 0;
};
