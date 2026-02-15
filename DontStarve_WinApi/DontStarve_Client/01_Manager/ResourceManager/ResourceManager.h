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

    // 리소스 등록 (클래스별 RegisterResources에서 호출)
    void RegisterObjectResource(GameObjectID id, const GameObjectData& data);

    // 리소스 정보 가져오기
    const GameObjectData* GetObjectResourceInfo(GameObjectID id) const;
    
    // 전체 오브젝트 리소스 맵 가져오기
    const std::map<GameObjectID, GameObjectData>& GetAllObjectResources() const { return m_objectResources; }

	// 스프라이트 로드/캐시
	std::shared_ptr<Sprite> LoadSprite(const std::wstring& fullPath);

	// 경로 빌드 (basePath + subFolder + filename → 상대/절대 경로)
	std::wstring BuildResourcePath(const std::wstring& basePath, const std::wstring& subFolder, const std::wstring& filename) const;

	// 맵 데이터 로드/캐시 (파싱 후 반환, 동일 파일은 캐시에서 반환)
	const MapData* LoadMapData(const std::wstring& mapFileName);

private:
    void ParseMapFileInto(const std::wstring& mapFileName, MapData& outMapData);

    std::map<GameObjectID, GameObjectData> m_objectResources;
    std::map<std::wstring, MapData> m_mapDataCache;
	std::unordered_map<std::wstring, std::weak_ptr<Sprite>> m_spriteCache;
}; 
