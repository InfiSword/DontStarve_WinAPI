#include "99_Default/pch.h"
#include "RenderManager.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../02_GameObject/Component/Transform/Transform.h"
#include "../../02_GameObject/Component/Transform/RectTransform.h"
#include "../../02_GameObject/Component/Sprite/SpriteRenderer.h"
#include "../../02_GameObject/Component/Sprite/Image.h"
#include "../../02_GameObject/Component/Text/Text.h"
#include "../../03_Animation/Animator.h"
#include "../../02_GameObject/Component/Sprite/SpriteSheet.h"
#include "../../02_GameObject/GameObject.h"

RenderManager::RenderManager()
{
	// 생성자에서 레이어별 커맨드 벡터의 초기 공간을 미리 확보 (싱글톤 생성 시 1회 수행)
	for (int i = 0; i < LAYER_COUNT; ++i) {
		m_layerCommands[i].reserve(512);
	}
}

RenderManager::~RenderManager()
{
	Release();
}

void RenderManager::Init()
{
	// GDI+ 객체 캐싱 초기화
	m_pCachedPen = new Gdiplus::Pen(Gdiplus::Color(0, 0, 0, 0));

	m_pCachedBrush = new Gdiplus::SolidBrush(Gdiplus::Color(0, 0, 0, 0));

	m_pCachedAttr = new Gdiplus::ImageAttributes();
}

void RenderManager::LateInit()
{
}

void RenderManager::Update(float deltaTime)
{
}

void RenderManager::LateUpdate()
{
}

void RenderManager::Release()
{
	Clear();

	// 캐싱된 GDI+ 객체 해제 및 nullptr 설정
	if (m_pCachedPen) { delete m_pCachedPen; m_pCachedPen = nullptr; }
	if (m_pCachedBrush) { delete m_pCachedBrush; m_pCachedBrush = nullptr; }
	if (m_pCachedAttr) { delete m_pCachedAttr; m_pCachedAttr = nullptr; }
}

void RenderManager::AddDrawCommand(Gdiplus::Bitmap* pBitmap, const Gdiplus::RectF& destRect, const Gdiplus::RectF& sourceRect, Gdiplus::Unit srcUnit, const Gdiplus::PointF& objectScreenPos, RenderLayer layer, float zOrder, Direction direction, const Gdiplus::Color& tintColor, bool hasTint, bool preFlipped, float rotation)
{
	DrawCommand cmd;
	cmd.type = DRAW_COMMAND_IMAGE;
	cmd.pBitmap = pBitmap;
	cmd.destRect = destRect;
	cmd.sourceRect = sourceRect;
	cmd.srcUnit = srcUnit;
	cmd.objectScreenPos = objectScreenPos;
	cmd.layer = layer;
	cmd.zOrder = zOrder;
	cmd.direction = direction;
	cmd.tintColor = tintColor;
	cmd.hasTint = hasTint;
	cmd.preFlipped = preFlipped;
	cmd.rotation = rotation;

	m_layerCommands[layer].push_back(cmd);
}

void RenderManager::AddTextCommand(const std::wstring* text, Gdiplus::Font* pFont, Gdiplus::Brush* pBrush, Gdiplus::StringFormat* pStringFormat, const Gdiplus::RectF& destRect, RenderLayer layer, float zOrder, float rotation, const Gdiplus::PointF& rotationPivot)
{
	DrawCommand cmd;
	cmd.type = DRAW_COMMAND_TEXT;
	cmd.textPtr = text;
	cmd.pFont = pFont;
	cmd.pBrush = pBrush;
	cmd.pStringFormat = pStringFormat;
	cmd.destRect = destRect;
	cmd.layer = layer;
	cmd.zOrder = zOrder;
	cmd.rotation = rotation;
	cmd.objectScreenPos = rotationPivot;

	m_layerCommands[layer].push_back(cmd);
}

void RenderManager::AddDrawRectCommand(const Gdiplus::RectF& rect, const Gdiplus::Color& color, float thickness, RenderLayer layer, float zOrder)
{
	DrawCommand cmd;
	cmd.type = DRAW_COMMAND_RECTANGLE;
	cmd.destRect = rect;
	cmd.color = color;
	cmd.thickness = thickness;
	cmd.layer = layer;
	cmd.zOrder = zOrder;

	m_layerCommands[layer].push_back(cmd);
}

void RenderManager::AddFillRectangleCommand(const Gdiplus::RectF& rect, const Gdiplus::Color& color, RenderLayer layer, float zOrder)
{
	DrawCommand cmd;
	cmd.type = DRAW_COMMAND_FILL_RECTANGLE;
	cmd.destRect = rect;
	cmd.color = color;
	cmd.layer = layer;
	cmd.zOrder = zOrder;

	m_layerCommands[layer].push_back(cmd);
}

void RenderManager::RenderImage(RectTransform* pRectTransform, ComponentElement::Image* pImage)
{
	if (!pRectTransform || !pImage) return;

	auto spriteHandle = pImage->GetSpriteHandle();
	if (!spriteHandle || !spriteHandle->bitmap) return;

	// UI는 화면 좌표계에서 직접 계산
	float width = spriteHandle->sourceRect.Width * pRectTransform->GetScaleX();
	float height = spriteHandle->sourceRect.Height * pRectTransform->GetScaleY();
	float x = pRectTransform->GetX();
	float y = pRectTransform->GetY();

	float renderX = x - (pImage->GetPivotX() * width);
	float renderY = y - (pImage->GetPivotY() * height);

	// Image 컴포넌트의 틴트 색상 사용 (UI 버튼 등에서 상태 표현에 사용됨)
	Gdiplus::Color tintColor = pImage->GetTintColor();
	bool hasTint = (tintColor.GetValue() != Gdiplus::Color::MakeARGB(255, 255, 255, 255));

	AddDrawCommand(spriteHandle->bitmap.get(), Gdiplus::RectF(renderX, renderY, width, height),
		spriteHandle->sourceRect, Gdiplus::UnitPixel, Gdiplus::PointF(x, y),
		pImage->GetLayer(), pImage->GetSortKey(), DIR_DOWN,
		tintColor, hasTint);
}

void RenderManager::RenderText(RectTransform* pRectTransform, Text* pText)
{
	if (!pRectTransform || !pText) return;

	auto params = pText->BuildRenderParams(pRectTransform);
	if (params.textPtr && !params.textPtr->empty())
	{
		AddTextCommand(
			params.textPtr,
			params.font,
			params.brush,
			params.format,
			params.destRect,
			params.layer,
			params.sortKey,
			pRectTransform->GetRotation(),
			Gdiplus::PointF(pRectTransform->GetX(), pRectTransform->GetY())
		);
	}
}

void RenderManager::RenderTile(Gdiplus::Bitmap* pTileBitmap, float worldX, float worldY, float width, float height)
{
	if (!pTileBitmap) return;

	Gdiplus::PointF screenPos = CameraManager::GetInstance()->WorldToScreen(worldX, worldY);
	float renderX = screenPos.X - width * 0.5f;
	float renderY = screenPos.Y - height * 0.5f;

	AddDrawCommand(pTileBitmap, Gdiplus::RectF(renderX, renderY, width, height), Gdiplus::RectF(0, 0, (float)pTileBitmap->GetWidth(), (float)pTileBitmap->GetHeight()), Gdiplus::UnitPixel, screenPos, LAYER_TILE_BACKGROUND, worldY, DIR_DOWN);
}

void RenderManager::Clear()
{
	for (int i = 0; i < LAYER_COUNT; ++i) {
		m_layerCommands[i].clear();
	}
}

void RenderManager::Flush(Gdiplus::Graphics* pGraphics)
{
	if (!pGraphics) return;

	for (int i = 0; i < LAYER_COUNT; ++i) {
		if (m_layerCommands[i].empty()) continue;
		
		std::sort(m_layerCommands[i].begin(), m_layerCommands[i].end(), CompareDrawCommands);
		
		for (const auto& cmd : m_layerCommands[i]) {
			Gdiplus::GraphicsState state;
			bool rotated = (cmd.rotation != 0.0f);
			
			if (rotated) {
				state = pGraphics->Save();
				pGraphics->TranslateTransform(cmd.objectScreenPos.X, cmd.objectScreenPos.Y);
				pGraphics->RotateTransform(cmd.rotation);
				pGraphics->TranslateTransform(-cmd.objectScreenPos.X, -cmd.objectScreenPos.Y);
			}

			switch (cmd.type) {
			case DRAW_COMMAND_IMAGE:
				if (cmd.pBitmap) {
					if (cmd.hasTint && m_pCachedAttr) {
						float r = cmd.tintColor.GetR() / 255.0f;
						float g = cmd.tintColor.GetG() / 255.0f;
						float b = cmd.tintColor.GetB() / 255.0f;
						float a = cmd.tintColor.GetA() / 255.0f;
						Gdiplus::ColorMatrix matrix = { r,0,0,0,0, 0,g,0,0,0, 0,0,b,0,0, 0,0,0,a,0, 0,0,0,0,1 };
						m_pCachedAttr->SetColorMatrix(&matrix);
						pGraphics->DrawImage(cmd.pBitmap, cmd.destRect, cmd.sourceRect.X, cmd.sourceRect.Y, cmd.sourceRect.Width, cmd.sourceRect.Height, cmd.srcUnit, m_pCachedAttr);
					}
					else {
						pGraphics->DrawImage(cmd.pBitmap, cmd.destRect, cmd.sourceRect.X, cmd.sourceRect.Y, cmd.sourceRect.Width, cmd.sourceRect.Height, cmd.srcUnit);
					}
				}
				break;
			case DRAW_COMMAND_TEXT:
				if (cmd.textPtr && cmd.pBrush) {
					pGraphics->DrawString(cmd.textPtr->c_str(), -1, cmd.pFont, cmd.destRect, cmd.pStringFormat, cmd.pBrush);
				}
				break;
			case DRAW_COMMAND_RECTANGLE:
				if (m_pCachedPen) {
					m_pCachedPen->SetColor(cmd.color);
					m_pCachedPen->SetWidth(cmd.thickness);
					pGraphics->DrawRectangle(m_pCachedPen, cmd.destRect);
				}
				break;
			case DRAW_COMMAND_FILL_RECTANGLE:
				if (m_pCachedBrush) {
					m_pCachedBrush->SetColor(cmd.color);
					pGraphics->FillRectangle(m_pCachedBrush, cmd.destRect);
				}
				break;			
			}

			if (rotated) {
				pGraphics->Restore(state);
			}
		}
		m_layerCommands[i].clear();
	}
}
