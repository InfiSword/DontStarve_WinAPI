#pragma once
#include "UIElement.h"

class RectTransform;
class Text;

class UIText : public UIElement
{
private:
    Text* m_text;

public:
    UIText(GameObjectID id, float width, float height,
           const std::wstring& text, const Gdiplus::Color& color,
           RenderLayer layer, float sortKey, 
           const std::wstring& fontName = L"Arial", float fontSize = 16.0f,
           Gdiplus::StringAlignment hAlign = Gdiplus::StringAlignmentCenter,
           Gdiplus::StringAlignment vAlign = Gdiplus::StringAlignmentCenter,
           float anchorMinX = 0.5f, float anchorMinY = 0.5f,
           float anchorMaxX = 0.5f, float anchorMaxY = 0.5f,
           float anchoredPosX = 0.0f, float anchoredPosY = 0.0f);
    virtual ~UIText();

    virtual void Init() override {}
    virtual void LateInit() override {}
    virtual void Update(float deltaTime) override;
    virtual void LateUpdate() override {}
    virtual void Render() override;
    virtual void Release() override;

    void SetText(const std::wstring& text);
    void SetColor(const Gdiplus::Color& color);
    Text* GetTextComponent() const { return m_text; }
};
