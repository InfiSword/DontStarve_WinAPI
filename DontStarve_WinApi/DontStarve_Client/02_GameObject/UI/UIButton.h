#pragma once
#include "../GameObject.h"

class UIButton : public GameObject
{
private:
    ButtonState m_buttonState;
    Gdiplus::Bitmap* m_hoverBitmap;
    bool m_isMouseOver;
    bool m_wasClicked;
    bool m_isDisabled;
    std::function<void()> m_onClickCallback;
    
    // 텍스트 렌더링 관련 멤버
    std::wstring m_buttonText;
    Gdiplus::Font* m_font;
    Gdiplus::Brush* m_textBrush;
    Gdiplus::StringFormat* m_stringFormat;

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
    void InitializeText();
    void CheckMouseInteraction();
    bool IsPointInside(float x, float y) const;
    ButtonState GetButtonState() const;    

    // 버튼 이벤트
    void SetOnClickCallback(std::function<void()> callback);
    
    // 비활성화 관련 메서드
    void SetDisabled(bool disabled);
    bool IsDisabled() const { return m_isDisabled; }
    void RenderDisabled();
}; 
