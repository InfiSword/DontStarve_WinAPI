#pragma once

class DontStarve_EditorMain
{
public:
	DontStarve_EditorMain();
	~DontStarve_EditorMain();

public:
	void Initialize();
	void Update();
	void Render();
	void Release();

	LRESULT HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	// 맵 저장/로드/새 맵 함수
	void NewMap();                                  // 새 맵 생성 (초기화)
	bool SaveMap(const WCHAR* filename);
	bool LoadMap(const WCHAR* filename);

	// 파일 다이얼로그 함수
	bool ShowSaveFileDialog(WCHAR* fileName, DWORD fileNameSize);
	bool ShowOpenFileDialog(WCHAR* fileName, DWORD fileNameSize);

	// 오브젝트 추가/제거/위치 관련 함수 (m_objectsDirty 플래그 관리)
	void AddObject(const GameObjectData& obj);
	void RemoveObject(size_t idx); // 인덱스 삭제
	void RemoveObject(GameObjectData* objToRemove); // 포인터로 삭제 (오버로드)
	void UpdateObjectPosition(GameObjectData* obj, int newX, int newY); // 오브젝트 위치 업데이트

	// 성능 정보 설정/조회 (Editor.cpp에서 계산된 값 받기)
	void SetCurrentFPS(float fps) { m_currentFPS = fps; }
	float GetCurrentFPS() const { return m_currentFPS; }
	float GetLayerMemoryUsageMB() const;

	// 디버그 설정
	void SetDebugInfoVisible(bool visible) { m_showDebugInfo = visible; }
	bool IsDebugInfoVisible() const { return m_showDebugInfo; }

private:
	// GDI+ 렌더링 관련 멤버 변수
	Gdiplus::Graphics* m_pGraphics;
	Gdiplus::Bitmap* m_pDoubleBufferBitmap;

	// 에디터 데이터 관련 멤버 변수
	TileData m_tileMap[MAP_WIDTH][MAP_HEIGHT]; // 타일맵 데이터
	std::vector<GameObjectData> m_gameObjects;

	// 플레이어 스폰 포인트
	bool m_hasPlayerSpawn = false;				// 플레이어 스폰 포인트가 설정되었는지 여부
	Gdiplus::PointF m_playerSpawnPoint;			// 플레이어 스폰 월드 좌표 (발 밑 중심)
	bool m_isPlayerSpawnMode = false;			// 플레이어 스폰 포인트 설정 모드

	// 팔레트 관련 멤버 변수
	std::vector<PaletteItem> m_paletteItems;    // 팔레트 아이템 리스트
	RECT m_paletteRect;                         // 팔레트 영역
	int m_selectedPaletteIndex;                 // 현재 선택된 팔레트 아이템 인덱스 (-1은 선택 없음)

	// 배치 및 프리뷰 관련 멤버 변수
	bool m_isPlacingMode;                       // 현재 아이템 배치 모드인지 여부 
	bool m_is3x3Mode;                           // 3x3 타일 설치 모드 여부
	POINT m_rawMousePos;                        // 현재 마우스 커서의 픽셀 좌표
	Gdiplus::PointF m_snappedPreviewPos;        // 프리뷰의 스냅된 화면 좌표 (float 정밀도)

	// Pivot 설정 관련 멤버 변수
	bool m_isPivotEditMode;                     // Pivot 편집 모드 여부
	POINT m_pivotEditPos;                       // Pivot 편집 위치
	float m_currentPivotX;                      // 현재 설정된 Pivot X
	float m_currentPivotY;                      // 현재 설정된 Pivot Y
	GameObjectData* m_editingObject;            // 편집 중인 오브젝트

	const int MIN_COLLIDER_SIZE = 4;
	const float MIN_COLLIDER_RADIUS = 2.0f;    // CircleCollider 최소 반지름
	bool m_isColliderEditMode = false;          // 콜라이더 편집 모드 여부
	GameObjectData* m_editingColliderObject = nullptr; // 현재 콜라이더를 편집 중인 오브젝트
	bool m_isDraggingCollider = false;          // 콜라이더 드래그 중인지 여부
	POINT m_colliderEditStartMousePos;          // 콜라이더 크기/위치 조절 시작 마우스 위치 (화면 좌표)
	RECT m_initialColliderRect;                 // 콜라이더 편집 시작 시 콜라이더의 초기 위치/크기 (오브젝트 로컬 좌표계) - BoxCollider용
	float m_initialColliderCenterX;            // 콜라이더 편집 시작 시 CircleCollider의 초기 중심 X
	float m_initialColliderCenterY;            // 콜라이더 편집 시작 시 CircleCollider의 초기 중심 Y
	float m_initialColliderRadius;             // 콜라이더 편집 시작 시 CircleCollider의 초기 반지름
	int m_draggingHandle = -1;                  // 드래그 중인 핸들 인덱스 (0:좌상단, 1:우상단, 2:좌하단, 3:우하단, 4:이동, 5:반지름조절)

	// 맵 스크롤 오프셋 관련 멤버 변수
	POINT m_mapOffset;                          // 맵 렌더링 오프셋 (스크롤 위치)

	// 하위 팔레트 데이터 구조체
	struct SubPaletteData {
		bool isOpen = false;
		ItemCategory category = CATEGORY_NONE;
		int targetCategoryId = -1;
		// TileType 또는 GameObjectType (메인 팔레트에서 선택된 category ID)

		RECT rect = { 0 };			 // 하위 팔레트 UI 영역
		std::vector<RECT> itemRects; // 하위 팔레트 아이템별 UI Rect

		std::vector<std::pair<TileID, const TileVariant*>> currentTileVariantDefs;
		std::vector<std::pair<GameObjectID, const ObjectVariant*>> currentObjectVariantDefs;

		int selectedTileVariantIndex = -1;
		int selectedObjectVariantIndex = -1;

		const TileVariant* getSelectedTileVariant() const {
			if (selectedTileVariantIndex != -1 && category == CATEGORY_TILE && selectedTileVariantIndex < currentTileVariantDefs.size())
			{
				return currentTileVariantDefs[selectedTileVariantIndex].second;
			}
			return nullptr;
		}
		const ObjectVariant* getSelectedObjectVariant() const {
			if (selectedObjectVariantIndex != -1 && category == CATEGORY_OBJECT && selectedObjectVariantIndex < currentObjectVariantDefs.size()) {
				return currentObjectVariantDefs[selectedObjectVariantIndex].second;
			}
			return nullptr;
		}

		TileID getSelectedTileID() const {
			if (selectedTileVariantIndex != -1 &&
				selectedTileVariantIndex < currentTileVariantDefs.size())
			{
				return currentTileVariantDefs[selectedTileVariantIndex].first;
			}
			return TILEID_NONE;
		}

		GameObjectID getSelectedGameObjectID() const {
			if (selectedObjectVariantIndex != -1 && selectedObjectVariantIndex < currentObjectVariantDefs.size()) {
				return currentObjectVariantDefs[selectedObjectVariantIndex].first;
			}
			return GOID_NONE;
		}
	} m_subPalette;


	// 리소스 관리 멤버 변수	
	std::map<TileType, std::map<TileID, TileVariant>> m_tileVariants;
	std::map<GameObjectType, std::map<GameObjectID, ObjectVariant>> m_objectVariants;

	// 아틀라스 비트맵 소유권
	std::unique_ptr<Gdiplus::Bitmap> m_tileAtlasBitmapOwner;
	std::unique_ptr<Gdiplus::Bitmap> m_objectAtlasBitmapOwner;

	// 줌 관련 멤버 변수
	float m_zoomFactor;
	const float m_minZoom;
	const float m_maxZoom;
	const float m_zoomStep;

	// 오브젝트 정렬 최적화용 멤버
	std::vector<const GameObjectData*> m_sortedObjects;
	bool m_objectsDirty = true;
	GameObjectData* m_selectedObjectPtr;

	// 레이어 캐싱 및 최적화 관련 변수
	Gdiplus::Bitmap* m_tileLayerBitmap;
	bool m_tileLayerDirty = true;
	Gdiplus::Bitmap* m_objectLayerBitmap;
	bool m_objectLayerDirty = true;
	Gdiplus::Bitmap* m_paletteLayerBitmap;
	bool m_paletteLayerDirty = true;
	Gdiplus::Bitmap* m_gridLayerBitmap;
	bool m_gridLayerDirty = true;

	// 성능 모니터링 관련 변수 (Editor.cpp에서 설정됨)
	float m_currentFPS = 0.0f;		// 현재 FPS (외부에서 설정)

	// 디버그 설정
	bool m_showDebugInfo = true;			// 디버그 정보 표시 여부

	// Walkable Area 편집 관련 변수
	bool m_isWalkableEditMode = false;		// Walkable 영역 편집 모드 여부
	bool m_isDraggingWalkable = false;		// Walkable 영역 드래그 중인지 여부
	POINT m_walkableDragStart;				// 드래그 시작 지점 (화면 좌표)
	POINT m_walkableDragEnd;				// 드래그 끝 지점 (화면 좌표)
	bool m_walkableAreaMap[MAP_HEIGHT][MAP_WIDTH];	// 각 타일별 walkable 여부

	// 카메라 드래그 관련 변수
	bool m_isDraggingCamera = false;		// 카메라 드래그 중인지 여부
	POINT m_cameraDragStart;				// 카메라 드래그 시작 지점 (화면 좌표)
	POINT m_initialMapOffset;				// 드래그 시작 시 맵 오프셋

private:
	// 내부 헬퍼 함수들
	void InitPalette();                             // 팔레트 초기화
	void LoadResources();                           // 이미지 리소스 로드
	void ReleaseResources();                        // 이미지 리소스 해제

	// 그리기 함수들
	void ComposeTileLayer();
	void ComposeObjectLayer();
	void ComposePaletteLayer();
	void ComposeGridLayer();

	// 성능 모니터링 함수 (메모리 사용량만)

	void DrawGrid(Gdiplus::Graphics* pGraphics);
	void DrawTileMap(Gdiplus::Graphics* pGraphics);
	void DrawObjects(Gdiplus::Graphics* pGraphics);
	void DrawPalette(Gdiplus::Graphics* pGraphics);
	void DrawPreview(Gdiplus::Graphics* pGraphics);
	void DrawSubPalette(Gdiplus::Graphics* pGraphics);
	void DrawPivotEditor(Gdiplus::Graphics* pGraphics); // Pivot 에디터 그리기 (현재 코드에 있다고 가정)
	void DrawColliders(Gdiplus::Graphics* pGraphics);
	void DrawPlayerSpawn(Gdiplus::Graphics* pGraphics);	// 플레이어 스폰 포인트 그리기
	void DrawWalkableAreas(Gdiplus::Graphics* pGraphics);	// Walkable 영역 그리기
	void DrawDebugInfo(Gdiplus::Graphics* pGraphics);

	const TileVariant* GetTileVariant(TileType type, TileID id) const;
	const ObjectVariant* GetObjectVariant(GameObjectType type, GameObjectID id)const;

	// Pivot 편집 관련 함수들 (기존 코드에 있다고 가정)
	void UpdatePivotEdit(POINT clickPoint);
	void StartPivotEdit(GameObjectData* obj);
	void EndPivotEdit();

	// 콜라이더 편집
	void StartColliderEdit(GameObjectData* obj);
	void EndColliderEdit();
	int GetColliderHandleAt(POINT screenPos); // 마우스 위치에 콜라이더 핸들이 있는지 확인

	// 현재 화면에 보이는 월드 좌표 범위 (뷰 프러스텀)를 반환합니다.
	Gdiplus::RectF GetViewWorldRect(float cullingMargin = 0.0f) const;
	Gdiplus::PointF WorldToScreen(Gdiplus::PointF worldPos) const;
	Gdiplus::RectF WorldToScreen(Gdiplus::RectF worldRect) const;
	Gdiplus::PointF ScreenToWorld(Gdiplus::PointF screenPos) const;
};