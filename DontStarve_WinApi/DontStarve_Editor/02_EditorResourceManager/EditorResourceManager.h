#pragma once

#include <map>
#include <memory>
#include "Struct.h"

class EditorResourceManager
{
public:
	EditorResourceManager() = default;
	~EditorResourceManager() = default;

	void LoadResources();
	void ReleaseResources();

	const ResourcePathUtils::TileResourceDef* GetTileVariant(TileType type, TileID id) const;
	const ResourcePathUtils::ObjectResourceDef* GetObjectVariant(GameObjectType type, GameObjectID id) const;

	const std::map<TileType, std::map<TileID, ResourcePathUtils::TileResourceDef>>& GetTileVariants() const { return m_tileVariants; }
	const std::map<GameObjectType, std::map<GameObjectID, ResourcePathUtils::ObjectResourceDef>>& GetObjectVariants() const { return m_objectVariants; }

	// 비트맵 캐시 관리
	std::shared_ptr<Gdiplus::Bitmap> GetCachedBitmap(const std::wstring& fullPath);
	void ClearBitmapCache();
	size_t GetBitmapCacheSize() const { return m_bitmapCache.size(); }

private:
	std::map<TileType, std::map<TileID, ResourcePathUtils::TileResourceDef>> m_tileVariants;
	std::map<GameObjectType, std::map<GameObjectID, ResourcePathUtils::ObjectResourceDef>> m_objectVariants;
	
	// 경로 기반 비트맵 캐시 (shared_ptr로 메모리 관리)
	std::map<std::wstring, std::shared_ptr<Gdiplus::Bitmap>> m_bitmapCache;
};
