#pragma once
#include "../GameObject/GameObject.h"

class UIImage : public GameObject
{

public:
    UIImage(GameObjectID id, float x, float y, float width, float height, RenderLayer layer, const std::wstring& imagePath, float sortKey);
    virtual ~UIImage();

    // GameObject 인터페이스 구현
    virtual void Init() override {}
    virtual void LateInit() override {}
    virtual void Update(float deltaTime) override;
    virtual void LateUpdate() override {}
    virtual void Render(Gdiplus::Graphics* pGraphics) override;
    virtual void Release() override;

    // UIImage 특화 메소드
    void LoadBitmap(const std::wstring& imagePath);
    Gdiplus::Bitmap* GetBitmap() const;

private:
    RenderLayer m_layer;
    float m_sortKey;
}; 