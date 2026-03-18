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
	void AddDrawCommand(Gdiplus::Bitmap* pBitmap, const Gdiplus::RectF& destRect, const Gdiplus::RectF& sourceRect, Gdiplus::Unit srcUnit, const Gdiplus::PointF& objectScreenPos, RenderLayer layer, float sortKey, Direction direction, const Gdiplus::Color& tintColor = Gdiplus::Color(255, 255, 255, 255), bool hasTint = false, bool preFlipped = false);
	void AddTextCommand(const std::wstring* text, Gdiplus::Font* pFont, Gdiplus::Brush* pBrush, Gdiplus::StringFormat* pStringFormat, const Gdiplus::RectF& destRect, RenderLayer layer, float sortKey);
	void AddDrawRectCommand(const Gdiplus::RectF& rect, const Gdiplus::Color& color, float thickness, RenderLayer layer, float sortKey);
	void AddFillRectangleCommand(const Gdiplus::RectF& rect, const Gdiplus::Color& color, RenderLayer layer, float sortKey);  // 채워진 사각형 명령
	void AddDrawEllipseCommand(const Gdiplus::RectF& rect, const Gdiplus::Color& color, float thickness, RenderLayer layer, float sortKey);  // 원/타원 외곽선
	void AddUIImageCommand(Gdiplus::Bitmap* bitmap, float x, float y, float width, float height,
		float pivotX, float pivotY, RenderLayer layer = LAYER_UI_BACKGROUND, float sortKey = 0.0f, const Gdiplus::Color& tintColor = Gdiplus::Color(255, 255, 255, 255), bool hasTint = false);
	
	void RenderGameObject(GameObject* pObject);
	void RenderEntity(Transform* pTransform, SpriteRenderer* pSpriteRenderer, Animator* pAnimator);
	void RenderUI(RectTransform* pRectTransform, ComponentElement::Image* pImage);
	void RenderTile(Gdiplus::Bitmap* pTileBitmap, float worldX, float worldY, float width, float height);

	void Clear();

	void Flush(Gdiplus::Graphics* pGraphics);

private:
	std::vector<DrawCommand> m_layerCommands[LAYER_COUNT];

	// 캐싱된 GDI+ 객체 (성능 최적화)
	Gdiplus::Pen* m_pCachedPen = nullptr;
	Gdiplus::SolidBrush* m_pCachedBrush = nullptr;
	Gdiplus::ImageAttributes* m_pCachedAttr = nullptr;

	// 레이어별로 이미 분할되어 있으므로 sortKey만 비교하여 정렬 비용 감소
	static bool CompareDrawCommands(const DrawCommand& a, const DrawCommand& b)
	{
		return a.sortKey < b.sortKey;
	}
};
