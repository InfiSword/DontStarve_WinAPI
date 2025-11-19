#pragma once

enum DrawCommandType {
	DRAW_COMMAND_IMAGE,
	DRAW_COMMAND_TEXT,
	DRAW_COMMAND_RECTANGLE,
	DRAW_COMMAND_FILL_RECTANGLE
};

// 화면에 그릴 모든 정보를 저장하는 명령 구조체
struct DrawCommand {
	DrawCommandType type;             // 명령 종류
	Gdiplus::Bitmap* pBitmap;         // 렌더링할 비트맵
	Gdiplus::RectF destRect;          // 화면 기준 목적 영역
	Gdiplus::RectF sourceRect;        // 소스 영역
	Gdiplus::Unit srcUnit;            // 소스 단위
	Gdiplus::PointF objectScreenPos;  // 객체의 화면상 중심 좌표
	RenderLayer layer;                // 렌더 레이어
	float sortKey;                    // Z 정렬용 키
	Direction direction;              // 방향

	// 텍스트 명령에만 사용하는 필드
	std::wstring text;                // 출력할 문자열
	Gdiplus::Font* pFont;             // 글꼴
	Gdiplus::Brush* pBrush;           // 브러시
	Gdiplus::StringFormat* pStringFormat; // 텍스트 정렬 정보

	Gdiplus::Color color;             // 색상
	float thickness;                  // 선 두께

	DrawCommand(Gdiplus::Bitmap* bmp, const Gdiplus::RectF& dest, const Gdiplus::RectF& src, Gdiplus::Unit unit, const Gdiplus::PointF& screenPos, RenderLayer l, float sk, Direction dir)
		: type(DRAW_COMMAND_IMAGE), pBitmap(bmp), destRect(dest), sourceRect(src), srcUnit(unit), objectScreenPos(screenPos), layer(l), sortKey(sk), direction(dir),
		text(L""), pFont(nullptr), pBrush(nullptr), pStringFormat(nullptr) {}

	DrawCommand(const std::wstring& t, Gdiplus::Font* f, Gdiplus::Brush* b, Gdiplus::StringFormat* sf, const Gdiplus::RectF& dest, RenderLayer l, float sk)
		: type(DRAW_COMMAND_TEXT), pBitmap(nullptr), destRect(dest), sourceRect(0, 0, 0, 0), srcUnit(Gdiplus::UnitPixel), objectScreenPos(dest.X, dest.Y), layer(l), sortKey(sk), direction(DIR_DOWN),
		text(t), pFont(f), pBrush(b), pStringFormat(sf) {}

	DrawCommand(const Gdiplus::RectF& rect, const Gdiplus::Color& c, float t, RenderLayer l, float sk)
		: type(DRAW_COMMAND_RECTANGLE), pBitmap(nullptr), destRect(rect), sourceRect(0, 0, 0, 0), srcUnit(Gdiplus::UnitPixel), objectScreenPos(rect.X, rect.Y), layer(l), sortKey(sk), direction(DIR_DOWN),
		text(L""), pFont(nullptr), pBrush(nullptr), pStringFormat(nullptr), color(c), thickness(t) {}

	DrawCommand(const Gdiplus::RectF& rect, const Gdiplus::Color& c, RenderLayer l, float sk, bool isFill = true)
		: type(DRAW_COMMAND_FILL_RECTANGLE), pBitmap(nullptr), destRect(rect), sourceRect(0, 0, 0, 0), srcUnit(Gdiplus::UnitPixel), objectScreenPos(rect.X, rect.Y), layer(l), sortKey(sk), direction(DIR_DOWN),
		text(L""), pFont(nullptr), pBrush(nullptr), pStringFormat(nullptr), color(c), thickness(0.0f) {}
};

// Forward declarations
class GameObject;
class Player;

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
	void Render();
	void Release();

	// 기본 렌더 명령 등록
	void AddDrawCommand(Gdiplus::Bitmap* pBitmap, const Gdiplus::RectF& destRect, const Gdiplus::RectF& sourceRect, Gdiplus::Unit srcUnit, const Gdiplus::PointF& objectScreenPos, RenderLayer layer, float sortKey, Direction direction);
	void AddTextCommand(const std::wstring& text, Gdiplus::Font* pFont, Gdiplus::Brush* pBrush, Gdiplus::StringFormat* pStringFormat, const Gdiplus::RectF& destRect, RenderLayer layer, float sortKey);
	void AddDrawCommand(const Gdiplus::RectF& rect, const Gdiplus::Color& color, float thickness, RenderLayer layer, float sortKey);
	void AddFillRectangleCommand(const Gdiplus::RectF& rect, const Gdiplus::Color& color, RenderLayer layer, float sortKey);  // 채워진 사각형 명령

	// UI 전용 렌더링 헬퍼
	// UI 이미지를 화면 좌표로 바로 렌더링
	void RenderUIImage(Gdiplus::Bitmap* bitmap, float x, float y, float width, float height,
		RenderLayer layer = LAYER_UI_BACKGROUND, float sortKey = 0.0f);

	// UI 텍스트 렌더링
	void RenderUIText(const std::wstring& text, Gdiplus::Font* font, Gdiplus::Brush* brush,
		float x, float y, float width, float height,
		RenderLayer layer = LAYER_UI_FOREGROUND, float sortKey = 0.0f);

	// 일반 GameObject 렌더링
	// GameObject 정보를 바탕으로 렌더 명령 큐에 추가
	void RenderGameObject(GameObject* pObject);

	// 타일 렌더링
	void RenderTile(Gdiplus::Bitmap* pTileBitmap, float worldX, float worldY, float width, float height);

	// 카메라에 보이는 객체만 렌더 (ViewportManager 활용)
	void RenderVisibleGameObjects();

	void Clear();

	void ApplyGdiTransform(Gdiplus::Graphics* pGraphics, const DrawCommand& command, float scaledWidth, float scaledHeight);

	void Flush(Gdiplus::Graphics* pGraphics);

private:
	std::vector<DrawCommand> m_drawCommands;

	// 정렬에서 레이어를 우선 비교하고 그 다음 sortKey 비교
	static bool CompareDrawCommands(const DrawCommand& a, const DrawCommand& b)
	{
		if (a.layer != b.layer) {
			return a.layer < b.layer;
		}
		return a.sortKey < b.sortKey;
	}
};