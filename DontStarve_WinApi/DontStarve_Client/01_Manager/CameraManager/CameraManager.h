#pragma once

// Ÿ�� ĳ�� ������ ����ü
struct TileCacheData {
	TileID id;
	Gdiplus::Bitmap* bitmap;
	Gdiplus::RectF sourceRect;
	bool isAtlasBased;
	
	TileCacheData() : id(TILEID_NONE), bitmap(nullptr), isAtlasBased(false) {}
	TileCacheData(TileID _id, Gdiplus::Bitmap* _bitmap, const Gdiplus::RectF& _rect, bool _isAtlas) 
		: id(_id), bitmap(_bitmap), sourceRect(_rect), isAtlasBased(_isAtlas) {}
};

class GameObject;
class RenderManager;
class Player;

class CameraManager : public CSingleTon<CameraManager>
{
	friend class CSingleTon<CameraManager>;
public:
	CameraManager();
	~CameraManager();

	void Init();
	void LateInit();
	void Update(float deltaTime);
	void LateUpdate();
	void Render();
	void Release();

	// ���� ��ǥ <-> ȭ�� ��ǥ ��ȯ �Լ�
	Gdiplus::PointF WorldToScreen(float worldX, float worldY);
	Gdiplus::PointF ScreenToWorld(float screenX, float screenY);
	
	// ����Ʈ ���� ��� �Լ�
	Gdiplus::RectF GetViewportWorldRect() const;
	
	Gdiplus::PointF GetCameraPos();

	// �÷��̾� ���� ���
	void SetTarget(GameObject* target);
	const GameObject* GetTarget();
	void FollowTarget(float deltaTime);
	void SetFollowMode(bool enabled) { m_followMode = enabled; }
	bool IsFollowMode() const { return m_followMode; }
	
	// ī�޶� ��ġ ���� ����
	void SetCameraPosition(float x, float y);

	// === ȭ�鿡 ���̴� ������Ʈ ���� ��� (ViewportManager ����) ===
	// ȭ�鿡 ���̴� ������Ʈ ������Ʈ (ObjectManager�� ������Ʈ���� �������)
	void UpdateVisibleObjects();
	const std::vector<GameObject*>& GetVisibleObjects() const { return m_visibleObjects; }
	
	// Ư�� ��ġ�� ������Ʈ ã�� (ȭ�鿡 ���̴� ������Ʈ�� �˻�)
	GameObject* FindObjectAtPosition(float worldX, float worldY);
	
	// ȭ�鿡 ���̴� ������Ʈ���� Ȯ��
	bool IsObjectVisible(GameObject* obj) const;
	
	// ����Ʈ ���� ����
	bool HasViewportChanged() const { return m_viewportChanged; }
	void ClearViewportChanged() { m_viewportChanged = false; }	

	// === Ÿ�� ������ ���� ��� ===
	// ȭ�鿡 ���̴� Ÿ�� ������ (�� �����͸� �Ű������� ����)
	void RenderVisibleTiles(RenderManager* renderManager, const MapData* mapData);
	
	// Ÿ�� ĳ�� ����
	void ClearTileCache();

private:
    GameObject* m_target;
	Gdiplus::PointF m_cameraPos;

	float m_zoomFactor;             // ���� �� ����
	
	bool m_followMode;              // �÷��̾� ���� ���
	
	// === ȭ�鿡 ���̴� ������Ʈ ���� (ViewportManager ����) ===
	std::vector<GameObject*> m_visibleObjects;
	std::unordered_set<GameObject*> m_visibleObjectSet; // ���� �˻��� ���� �ؽü�
	
	// ����Ʈ ����
	Gdiplus::RectF m_lastViewportRect;
	bool m_viewportChanged;
	
	// ȭ�鿡 ���̴� ������Ʈ���� Ȯ���ϴ� ���� �Լ�
	bool IsObjectInViewport(GameObject* obj) const;
	
	// ����Ʈ ���� ����
	void CheckViewportChanged();
	
	// === Ÿ�� ������ ���� ===
	std::map<UINT, TileCacheData> m_tileCache;
	std::vector<std::pair<int, int>> m_visibleTileIndices;
	
	// Ÿ�� ������ ����ȭ ���� ������
	int m_lastStartTileX, m_lastStartTileY, m_lastEndTileX, m_lastEndTileY;
	bool m_tileViewportChanged;
	
	// Ÿ�� ������ ���� �Լ���
	void LoadTileBitmap(TileID tileID, TileCacheData& cacheData);
	void RenderSingleTile(RenderManager* renderManager, const MapData* mapData, int x, int y, float worldY);
	// bool IsTileInViewport(int tileX, int tileY) const; // ���� �̻��
};