#pragma once
#include "../GameObject/GameObject.h"
#include <functional>

enum class ButtonState
{
    NORMAL,
    HOVER,
    CLICKED,
    DISABLED
};

class UIButton : public GameObject
{
private:
    ButtonState m_buttonState;
    Gdiplus::Bitmap* m_normalBitmap;
    Gdiplus::Bitmap* m_hoverBitmap;
    bool m_isMouseOver;
    bool m_wasClicked;
    bool m_isDisabled;
    std::function<void()> m_onClickCallback;
    
    // 텍스트 관련 멤버 변수
    std::wstring m_buttonText;
    Gdiplus::Font* m_font;
    Gdiplus::Brush* m_textBrush;
    Gdiplus::StringFormat* m_stringFormat;

public:
    UIButton(GameObjectID id, float x, float y, float width, float height,
             const std::wstring& normalImagePath, const std::wstring& hoverImagePath, 
             const std::wstring buttonText = L"");
    virtual ~UIButton();

    // GameObject 인터페이스 구현
    virtual void Init() override {}
    virtual void LateInit() override {}
    virtual void Update(float deltaTime) override;
    virtual void LateUpdate() override {}
    virtual void Render(Gdiplus::Graphics* pGraphics) override;
    virtual void Release() override;

    // UIButton 전용 메소드
    void LoadBitmaps(const std::wstring& normalImagePath, const std::wstring& hoverImagePath);
    void InitializeText();
    void CheckMouseInteraction();
    bool IsPointInside(float x, float y) const;
    Gdiplus::Bitmap* GetBitmap() const;
    void SetOnClickCallback(std::function<void()> callback);
    ButtonState GetButtonState() const;
    
    // 비활성화 관련 메소드
    void SetDisabled(bool disabled);
    bool IsDisabled() const { return m_isDisabled; }
    void RenderDisabled(Gdiplus::Graphics* pGraphics);
}; 