#pragma once
#include "../../../Header/SingleTon.h"

class GameObject;
class Transform;
class RectTransform;
class SpriteRenderer;
class Animator;

namespace ComponentElement {
	class Image;
}

class RenderManager : public CSingleTon<RenderManager>
{
	friend class CSingleTon<RenderManager>;
public:
	RenderManager();
	~RenderManager();

	void Init();
	void LateInit();
	void Update(float deltaTime);
	void LateUpdate();
	void Release();

	// 기본 렌더 명령 등록
	void AddWorldEntityCommand(Gdiplus::Bitmap* pBitmap, const Gdiplus::RectF& sourceRect, float worldX, float worldY, float scaleX, float scaleY, float pivotX, float pivotY, RenderLayer layer, float zOrder, Direction direction = DIR_DOWN, const Gdiplus::Color& tintColor = Gdiplus::Color(255, 255, 255, 255), bool hasTint = false, bool preFlipped = false, float rotation = 0.0f);

	void AddUICommand(Gdiplus::Bitmap* pBitmap, const Gdiplus::RectF& sourceRect, float screenX, float screenY, float scaleX, float scaleY, float pivotX, float pivotY, RenderLayer layer, float zOrder, const Gdiplus::Color& tintColor = Gdiplus::Color(255, 255, 255, 255), bool hasTint = false, float rotation = 0.0f);

	void AddTextCommand(const std::wstring* text, Gdiplus::Font* pFont, Gdiplus::Brush* pBrush, Gdiplus::StringFormat* pStringFormat, const Gdiplus::RectF& destRect, RenderLayer layer, float zOrder = 0.0f, float rotation = 0.0f, const Gdiplus::PointF& rotationPivot = Gdiplus::PointF(0, 0));
	
	void AddDrawRectCommand(const Gdiplus::RectF& rect, const Gdiplus::Color& color, float thickness, RenderLayer layer, float zOrder = 0.0f);

	void AddFillRectangleCommand(const Gdiplus::RectF& rect, const Gdiplus::Color& color, RenderLayer layer, float zOrder = 0.0f); 

	void Clear();

	void Flush(Gdiplus::Graphics* pGraphics);

private:
	std::vector<DrawCommand> m_layerCommands[LAYER_COUNT];

	Gdiplus::Pen* m_pCachedPen = nullptr;
	Gdiplus::SolidBrush* m_pCachedBrush = nullptr;
	Gdiplus::ImageAttributes* m_pCachedAttr = nullptr;

	Gdiplus::PointF m_cameraPos = { 0, 0 };

	void RenderSprite(Gdiplus::Graphics* pGraphics, const DrawCommand::SpriteData& data, const Gdiplus::RectF& destRect);

	static bool CompareDrawCommands(const DrawCommand& a, const DrawCommand& b)
	{
		return a.zOrder < b.zOrder;
	}
};
