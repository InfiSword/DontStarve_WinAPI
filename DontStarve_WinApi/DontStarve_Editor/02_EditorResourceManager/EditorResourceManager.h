#pragma once

#include <map>
#include <memory>
#include "Struct.h"

// 에디터 내부용 카테고리 구분
enum class ObjectCategory {
	Natural = 0,
	Monster,
	Building,
	Item,
	Player,
	Count
};

class EditorResourceManager
{
public:
	EditorResourceManager() = default;
	~EditorResourceManager() = default;

	void LoadResources();
	void ReleaseResources();

	const ResourcePathUtils::TileResourceDef* GetTileVariant(TileType type, TileID id) const;
	const ResourcePathUtils::ObjectResourceDef* GetObjectVariant(GameObjectID id) const;

	const std::map<TileType, std::map<TileID, ResourcePathUtils::TileResourceDef>>& GetTileVariants() const { return m_tileVariants; }
	const std::map<GameObjectID, ResourcePathUtils::ObjectResourceDef>& GetObjectVariants() const { return m_objectVariants; }

	// 비트맵 캐시 관리
	std::shared_ptr<Gdiplus::Bitmap> GetCachedBitmap(const std::wstring& fullPath);
	void ClearBitmapCache();
	size_t GetBitmapCacheSize() const { return m_bitmapCache.size(); }

	static std::wstring GetGameDataPath();
	static bool ObjectResourceOverridesFileExists();
	void LoadObjectResourceOverrides();
	void ApplyInitialObjectValuesToAll();
	bool SaveObjectResourceOverride(GameObjectID id, const ResourcePathUtils::ObjectResourceDef& def);
	bool SaveAllObjectResourceOverrides();

	// ID 기반 카테고리 판별 헬퍼 (에디터 내부용)
	static ObjectCategory GetCategoryFromID(GameObjectID id);

private:
	static std::wstring GetResourceRoot();

	std::map<TileType, std::map<TileID, ResourcePathUtils::TileResourceDef>> m_tileVariants;
	std::map<GameObjectID, ResourcePathUtils::ObjectResourceDef> m_objectVariants;
	
	std::map<std::wstring, std::shared_ptr<Gdiplus::Bitmap>> m_bitmapCache;
};
