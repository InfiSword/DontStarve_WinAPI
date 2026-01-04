#pragma once
#include "../GameObject.h"

// Forward declarations
class RectTransform;

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
    virtual void Render();
    virtual void Release() override;

    // UIImage 전용 메서드
    void LoadBitmap(const std::wstring& imagePath);
    Gdiplus::Bitmap* GetBitmap() const;

private:
    RectTransform* m_rectTransform;
    ::Image* m_image;
}; 
