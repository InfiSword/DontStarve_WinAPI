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

    // 리소스 정보 가져오기
    const ResourcePathUtils::ObjectResourceDef* GetObjectResourceInfo(GameObjectID id) const;
    
    // 전체 오브젝트 리소스 맵 가져오기
    const std::map<GameObjectID, ResourcePathUtils::ObjectResourceDef>& GetAllObjectResources() const { return m_objectResources; }

	// 스프라이트 로드/캐시
	std::shared_ptr<Sprite> LoadSprite(const std::wstring& fullPath);

private:
    // 리소스 등록 (내부 사용)
    void RegisterObjectResource(GameObjectID id, const ResourcePathUtils::ObjectResourceDef& data);

    std::map<GameObjectID, ResourcePathUtils::ObjectResourceDef> m_objectResources;
	std::unordered_map<std::wstring, std::weak_ptr<Sprite>> m_spriteCache;
}; 
