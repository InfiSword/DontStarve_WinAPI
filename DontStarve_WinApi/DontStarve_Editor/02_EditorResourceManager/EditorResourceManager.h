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

	// 비트맵 캐시 관리 (fullPath: 절대경로 또는 리소스 기준 상대경로, e.g. Resource\\Tiles\\...)
	std::shared_ptr<Gdiplus::Bitmap> GetCachedBitmap(const std::wstring& fullPath);
	void ClearBitmapCache();
	size_t GetBitmapCacheSize() const { return m_bitmapCache.size(); }

	// GameData: 오브젝트 에디터에서 설정한 pivot/콜라이더를 저장·로드 (리소스 템플릿에 반영)
	static std::wstring GetGameDataPath();
	/// object_resource_overrides.txt 파일이 GameData 경로에 존재하는지 여부
	static bool ObjectResourceOverridesFileExists();
	void LoadObjectResourceOverrides();  // LoadResources() 내부에서 호출
	/// object_resource_overrides.txt 가 없을 때 사용: 모든 오브젝트에 pivot 0.5/1.0, box 콜라이더=이미지 크기 적용
	void ApplyInitialObjectValuesToAll();
	bool SaveObjectResourceOverride(GameObjectType type, GameObjectID id, const ResourcePathUtils::ObjectResourceDef& def);
	/// 오브젝트 에디터에서 "저장" 시 현재 모든 오브젝트 variant의 pivot/콜라이더를 한 번에 파일로 저장
	bool SaveAllObjectResourceOverrides();

private:
	static std::wstring GetResourceRoot();

	std::map<TileType, std::map<TileID, ResourcePathUtils::TileResourceDef>> m_tileVariants;
	std::map<GameObjectType, std::map<GameObjectID, ResourcePathUtils::ObjectResourceDef>> m_objectVariants;
	
	// 경로 기반 비트맵 캐시 (shared_ptr로 메모리 관리)
	std::map<std::wstring, std::shared_ptr<Gdiplus::Bitmap>> m_bitmapCache;
};
