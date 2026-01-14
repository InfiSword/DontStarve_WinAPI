#include "99_Default/pch.h"
#include "Text.h"
#include "../Transform/RectTransform.h"
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
	Gdiplus::StringAlignment hAlign,
	Gdiplus::StringAlignment vAlign)
	: Component(owner),
	m_text(text),
	m_layer(layer),
	m_sortKey(sortKey),
	m_width(width),
	m_height(height)
{
	m_brush = std::make_unique<Gdiplus::SolidBrush>(color);
	
	// 한글 지원을 위해 FontFamily를 명시적으로 생성
	// GDI+ Font 생성자는 FontFamily를 복사하므로 스택 변수 사용 가능
	Gdiplus::FontFamily fontFamily(fontName.c_str());
	if (fontFamily.GetLastStatus() == Gdiplus::Ok) {
		m_font = std::make_unique<Gdiplus::Font>(&fontFamily, fontSize, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint);
	}
	else {
		Gdiplus::FontFamily defaultFamily(L"맑은 고딕");
		if (defaultFamily.GetLastStatus() == Gdiplus::Ok) {
			m_font = std::make_unique<Gdiplus::Font>(&defaultFamily, fontSize, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint);
		}
	}
	
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

void Text::SetText(const std::wstring& text)
{
	m_text = text;
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
	float pivotX = rectTransform->GetPivotX();
	float pivotY = rectTransform->GetPivotY();

	params.text = m_text;
	params.font = m_font.get();
	params.brush = m_brush.get();
	params.format = m_format.get();
	params.destRect = Gdiplus::RectF(
		x - (pivotX * m_width),
		y - (pivotY * m_height),
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
		// 한글 지원을 위해 FontFamily를 명시적으로 생성
		Gdiplus::FontFamily defaultFamily(L"맑은 고딕");
		if (defaultFamily.GetLastStatus() == Gdiplus::Ok) {
			m_font = std::make_unique<Gdiplus::Font>(&defaultFamily, 16.0f, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint);
		}
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
