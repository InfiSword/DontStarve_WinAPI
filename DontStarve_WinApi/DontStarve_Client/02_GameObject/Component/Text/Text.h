struct TextStyle {
	Gdiplus::Color color;
	RenderLayer layer;
	float sortKey;
	float width;
	float height;
};

struct TextRenderParams {
	std::wstring text;
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
		Gdiplus::StringAlignment hAlign = Gdiplus::StringAlignmentCenter,
		Gdiplus::StringAlignment vAlign = Gdiplus::StringAlignmentCenter);
		
	virtual ~Text();

	virtual void Init() override;
	virtual void Release() override;

	// 텍스트 내용은 런타임 변경 허용
	void SetText(const std::wstring& text);
	void ApplyStyle(const TextStyle& style);

	// 렌더 파라미터 계산 (렌더 호출은 외부(UI)에서 수행)
	TextRenderParams BuildRenderParams(const RectTransform* rectTransform);

private:
	std::wstring m_text;
	std::unique_ptr<Gdiplus::Font> m_font;
	std::unique_ptr<Gdiplus::Brush> m_brush;
	std::unique_ptr<Gdiplus::StringFormat> m_format;

	RenderLayer m_layer;
	float m_sortKey;
	float m_width;
	float m_height;

	void EnsureDefaults();
};
