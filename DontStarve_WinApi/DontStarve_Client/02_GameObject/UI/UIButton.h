#pragma once
#include "../GameObject.h"

class RectTransform;
class Text;
class Button;

namespace ComponentElement { class Image; }

class UIButton : public GameObject
{
private:
    RectTransform* m_rectTransform;
    ComponentElement::Image* m_image;
    class Button* m_buttonComp;
    class Text* m_textComp;
    std::shared_ptr<Sprite> m_normalSprite;  // normal 상태 스프라이트 저장
    std::shared_ptr<Sprite> m_hoverSprite;   // hover 상태 스프라이트 저장

public:
    UIButton(GameObjectID id, float x, float y, float width, float height,
             const std::wstring& normalImagePath, const std::wstring& hoverImagePath, 
             const std::wstring buttonText = L"");
    virtual ~UIButton();
    
    virtual void Init() override {}
    virtual void LateInit() override {}
    virtual void Update(float deltaTime) override;
    virtual void LateUpdate() override {}
    virtual void Render();
    virtual void Release() override;

    Gdiplus::Bitmap* GetBitmap() const;

    // UIButton 전용 메서드
    void LoadBitmaps(const std::wstring& normalImagePath, const std::wstring& hoverImagePath);
    ButtonState GetButtonState() const;    

    // 버튼 이벤트
    void SetOnClickCallback(std::function<void()> callback);
    
    // 비활성화 관련 메서드
    void SetDisabled(bool disabled);
    bool IsDisabled() const;
    void RenderDisabled();
}; 
