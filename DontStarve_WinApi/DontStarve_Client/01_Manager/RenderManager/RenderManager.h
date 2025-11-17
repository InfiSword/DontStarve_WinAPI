#pragma once

enum DrawCommandType {
    DRAW_COMMAND_IMAGE,
    DRAW_COMMAND_TEXT,
    DRAW_COMMAND_RECTANGLE,
    DRAW_COMMAND_FILL_RECTANGLE  // 새로운 타입 추가
};

// 단일 드로우 명령 정보를 담는 구조체
struct DrawCommand {
    DrawCommandType type;             // 커맨드 타입
    Gdiplus::Bitmap* pBitmap;         // 이미지용 (텍스트용은 nullptr)
    Gdiplus::RectF destRect;          // 이미지 위치/크기 (텍스트용)
    Gdiplus::RectF sourceRect;        // 이미지용 (텍스트용은 무시)
    Gdiplus::Unit srcUnit;            // 이미지용 (텍스트용은 무시)
    Gdiplus::PointF objectScreenPos;  // 변환 중심
    RenderLayer layer;                // 렌더링 레이어
    float sortKey;                    // Z-정렬 키
    Direction direction;              // 방향 

    // 텍스트 커맨드용 필드들
    std::wstring text;                // 표시할 텍스트 내용
    Gdiplus::Font* pFont;             // 사용할 폰트
    Gdiplus::Brush* pBrush;           // 사용할 브러시 
    Gdiplus::StringFormat* pStringFormat; //  텍스트 포맷

    Gdiplus::Color color;             // 색상
    float thickness;                  // 두께

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

    // 기본 드로우 커맨드 추가
    void AddDrawCommand(Gdiplus::Bitmap* pBitmap, const Gdiplus::RectF& destRect, const Gdiplus::RectF& sourceRect, Gdiplus::Unit srcUnit, const Gdiplus::PointF& objectScreenPos, RenderLayer layer, float sortKey, Direction direction);
    void AddTextCommand(const std::wstring& text, Gdiplus::Font* pFont, Gdiplus::Brush* pBrush, Gdiplus::StringFormat* pStringFormat, const Gdiplus::RectF& destRect, RenderLayer layer, float sortKey);
    void AddDrawCommand(const Gdiplus::RectF& rect, const Gdiplus::Color& color, float thickness, RenderLayer layer, float sortKey);
    void AddFillRectangleCommand(const Gdiplus::RectF& rect, const Gdiplus::Color& color, RenderLayer layer, float sortKey);  // 새로운 메서드 추가

    // === UI 렌더링 전용 메소드들 ===
    // UI 이미지 렌더링 (화면 좌표 직접 사용)
    void RenderUIImage(Gdiplus::Bitmap* bitmap, float x, float y, float width, float height, 
                      RenderLayer layer = LAYER_UI_BACKGROUND, float sortKey = 0.0f);
    
    // UI 텍스트 렌더링
    void RenderUIText(const std::wstring& text, Gdiplus::Font* font, Gdiplus::Brush* brush, 
                     float x, float y, float width, float height, 
                     RenderLayer layer = LAYER_UI_FOREGROUND, float sortKey = 0.0f);

    // === 기존 GameObject 렌더링 메소드들 ===
    // GameObject 렌더링 (비트맵만 가져와서 렌더링 큐에 추가)
    void RenderGameObject(GameObject* pObject);
    
    // 타일 렌더링
    void RenderTile(Gdiplus::Bitmap* pTileBitmap, float worldX, float worldY, float width, float height);
    
    // 모든 게임 오브젝트 렌더링 (ObjectManager에서 호출)
    void RenderAllGameObjects(const std::vector<GameObject*>& gameObjects);
    
    // 화면에 보이는 오브젝트만 렌더링 (ViewportManager 사용)
    void RenderVisibleGameObjects();

    void Clear();

    void ApplyGdiTransform(Gdiplus::Graphics* pGraphics, const DrawCommand& command, float scaledWidth, float scaledHeight);

    void Flush(Gdiplus::Graphics* pGraphics);

private:
    std::vector<DrawCommand> m_drawCommands;
    
    // GameObject 타입별 렌더 레이어 결정
    RenderLayer GetRenderLayerForObject(GameObject* pObject);

    // CompareDrawCommands 시그니처 및 로직 변경 (RenderLayer를 1순위로)
    static bool CompareDrawCommands(const DrawCommand& a, const DrawCommand& b)
    {
      
        if (a.layer != b.layer) {
            return a.layer < b.layer;
        }

        if (a.sortKey != b.sortKey) {
            return a.sortKey < b.sortKey;
        }
        return reinterpret_cast<uintptr_t>(a.pBitmap) < reinterpret_cast<uintptr_t>(b.pBitmap);
    }
};