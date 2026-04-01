#include "99_Default/pch.h"
#include "Text.h"
#include "../Transform/RectTransform.h"
#include  "../../GameObject.h"
#include "../../../01_Manager/RenderManager/RenderManager.h"

Text::Text(GameObject* owner,
	const std::wstring& text,
	const Gdiplus::Color& color,
	float width,
	float height,
	RenderLayer layer,
	float sortKey,
	const std::wstring& fontName,
	float fontSize,
	Gdiplus::FontStyle fontStyle,
	Gdiplus::StringAlignment hAlign,
	Gdiplus::StringAlignment vAlign)
	: Component(owner),
	m_text(text),
	m_layer(layer),
	m_sortKey(sortKey),
	m_width(width),
	m_height(height),
	m_pivotX(0.5f),
	m_pivotY(0.5f)
{
	m_brush = std::make_unique<Gdiplus::SolidBrush>(color);
	
	// 폰트 생성
	m_font = std::make_unique<Gdiplus::Font>(fontName.c_str(), fontSize, fontStyle, Gdiplus::UnitPoint);
	
	m_format = std::make_unique<Gdiplus::StringFormat>();
	m_format->SetAlignment(hAlign);
	m_format->SetLineAlignment(vAlign);
}

Text::~Text()
{
	Release();
}

void Text::Init()
{
}

void Text::Release()
{
	m_font.reset();
	m_brush.reset();
	m_format.reset();
}

void Text::Render()
{
	RectTransform* rt = GetOwner()->GetComponent<RectTransform>();
	if (!rt) return;

	TextRenderParams params = BuildRenderParams(rt);
	if (params.textPtr && !params.textPtr->empty())
	{
		RenderManager::GetInstance()->AddTextCommand(
			params.textPtr,
			params.font,
			params.brush,
			params.format,
			params.destRect,
			params.layer,
			params.sortKey,
			rt->GetRotation(),
			Gdiplus::PointF(rt->GetX(), rt->GetY())
		);
	}
}

void Text::SetText(const std::wstring& text)
{
	m_text = text;
}

void Text::SetColor(const Gdiplus::Color& color)
{
	m_brush = std::make_unique<Gdiplus::SolidBrush>(color);
}

void Text::SetFontStyle(const std::wstring& fontName, float fontSize, Gdiplus::FontStyle fontStyle)
{
	m_font = std::make_unique<Gdiplus::Font>(fontName.c_str(), fontSize, fontStyle, Gdiplus::UnitPoint);
}

void Text::ApplyStyle(const TextStyle& style)
{
	m_brush = std::make_unique<Gdiplus::SolidBrush>(style.color);
	m_layer = style.layer;
	m_sortKey = style.sortKey;
	m_width = style.width;
	m_height = style.height;
}

TextRenderParams Text::BuildRenderParams(const RectTransform* rectTransform)
{
	TextRenderParams params{};
	if (!rectTransform) return params;
	if (m_text.empty()) return params;
	if (m_width <= 0.0f || m_height <= 0.0f) return params;

	EnsureDefaults();

	float x = rectTransform->GetX();
	float y = rectTransform->GetY();

	params.textPtr = &m_text;
	params.font = m_font.get();
	params.brush = m_brush.get();
	params.format = m_format.get();
	params.destRect = Gdiplus::RectF(
		x - (m_pivotX * m_width),
		y - (m_pivotY * m_height),
		m_width,
		m_height
	);
	params.layer = m_layer;
	params.sortKey = m_sortKey;

	return params;
}

void Text::EnsureDefaults()
{
	if (!m_font)
	{
		m_font = std::make_unique<Gdiplus::Font>(L"Arial", 16.0f, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint);
	}
	if (!m_brush)
	{
		m_brush = std::make_unique<Gdiplus::SolidBrush>(Gdiplus::Color::Black);
	}
	if (!m_format)
	{
		m_format = std::make_unique<Gdiplus::StringFormat>();
		m_format->SetAlignment(Gdiplus::StringAlignmentCenter);
		m_format->SetLineAlignment(Gdiplus::StringAlignmentCenter);
	}
}
