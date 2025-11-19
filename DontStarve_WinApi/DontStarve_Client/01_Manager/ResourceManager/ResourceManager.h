#pragma once

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
    
    // 경로 빌드 헬퍼 함수들
    std::wstring BuildResourcePath(const std::wstring& basePath, const std::wstring& subFolder, const std::wstring& filename) const;
    std::wstring BuildObjectResourcePath(GameObjectID id, const std::wstring& subFolder, const std::wstring& filename) const;
    std::wstring BuildTileResourcePath(TileID id, const std::wstring& subFolder, const std::wstring& filename) const;

private:
    std::map<GameObjectID, GameObjectData> m_objectResources;
    std::map<TileID, TileData> m_tileResources;
}; 