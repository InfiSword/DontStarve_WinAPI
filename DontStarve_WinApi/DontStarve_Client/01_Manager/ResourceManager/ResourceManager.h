#pragma once

#include "../../02_GameObject/Component/Sprite/Sprite.h"

class ResourceManager : public CSingleTon<ResourceManager>
{
    friend class CSingleTon<ResourceManager>;
private:
    ResourceManager();
    ~ResourceManager();

public:
    void Init();
    void Release();
    
    // resources.txt 파일 로드
    void LoadResourcesFromFile(const std::wstring& filePath);
    
    // 리소스 정보 가져오기
    const GameObjectData* GetObjectResourceInfo(GameObjectID id) const;
    const TileData* GetTileResourceInfo(TileID id) const;
    
    // 전체 리소스 맵 가져오기
    const std::map<GameObjectID, GameObjectData>& GetAllObjectResources() const { return m_objectResources; }
    const std::map<TileID, TileData>& GetAllTileResources() const { return m_tileResources; }

	// 스프라이트 로드/캐시
	std::shared_ptr<Sprite> LoadSprite(const std::wstring& fullPath);
	std::shared_ptr<Sprite> LoadSpriteFromAtlas(const std::wstring& atlasPath, const Gdiplus::RectF& srcRect, float pivotX = 0.5f, float pivotY = 0.5f);
    
    // 경로 빌드 헬퍼 함수들
    std::wstring BuildObjectResourcePath(GameObjectID id, const std::wstring& subFolder, const std::wstring& filename) const;
    std::wstring BuildTileResourcePath(TileID id, const std::wstring& subFolder, const std::wstring& filename) const;
    std::wstring BuildUIResourcePath(const std::wstring& subFolder, const std::wstring& filename) const;

private:
    std::wstring BuildResourcePath(const std::wstring& basePath, const std::wstring& subFolder, const std::wstring& filename) const;
    std::map<GameObjectID, GameObjectData> m_objectResources;
    std::map<TileID, TileData> m_tileResources;
	std::unordered_map<std::wstring, std::weak_ptr<Sprite>> m_spriteCache;
}; 
