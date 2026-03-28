struct TextStyle {
	Gdiplus::Color color;
	RenderLayer layer;
	float sortKey;
	float width;
	float height;
};

struct TextRenderParams {
	const std::wstring* textPtr = nullptr;  
	Gdiplus::Font* font = nullptr;
	Gdiplus::Brush* brush = nullptr;
	Gdiplus::StringFormat* format = nullptr;
	Gdiplus::RectF destRect;
	RenderLayer layer = LAYER_UI_FOREGROUND;
	float sortKey = 0.0f;
};
#pragma once

#include "../Component.h"

class RectTransform;
class RenderManager;

class Text : public Component
{
public:
	Text(GameObject* owner,
		const std::wstring& text = L"",
		const Gdiplus::Color& color = Gdiplus::Color::Black,
		float width = 0.0f,
		float height = 0.0f,
		RenderLayer layer = LAYER_UI_FOREGROUND,
		float sortKey = 0.0f,
		const std::wstring& fontName = L"Arial",
		float fontSize = 16.0f,
		Gdiplus::FontStyle fontStyle = Gdiplus::FontStyleRegular,
		Gdiplus::StringAlignment hAlign = Gdiplus::StringAlignmentCenter,
		Gdiplus::StringAlignment vAlign = Gdiplus::StringAlignmentCenter);
		
	virtual ~Text();

	virtual void Init() override;
	virtual void Release() override;

	void Render();

	// 텍스트 내용은 런타임 변경 허용
	void SetText(const std::wstring& text);
	void SetColor(const Gdiplus::Color& color);
	void SetFontStyle(const std::wstring& fontName, float fontSize, Gdiplus::FontStyle fontStyle);
	void ApplyStyle(const TextStyle& style);

	// 렌더 파라미터 계산 (렌더 호출은 외부(UI)에서 수행)
	TextRenderParams BuildRenderParams(const RectTransform* rectTransform);

	// 피벗 제어 (텍스트는 스프라이트가 없으므로 자체 멤버로 관리)
	float GetPivotX() const { return m_pivotX; }
	float GetPivotY() const { return m_pivotY; }
	void SetPivot(float x, float y) { m_pivotX = x; m_pivotY = y; }

private:
	std::wstring m_text;
	std::unique_ptr<Gdiplus::Font> m_font;
	std::unique_ptr<Gdiplus::Brush> m_brush;
	std::unique_ptr<Gdiplus::StringFormat> m_format;

	RenderLayer m_layer;
	float m_sortKey;
	float m_width;
	float m_height;
	float m_pivotX;
	float m_pivotY;

	void EnsureDefaults();
};
