#include "99_Default/pch.h"
#include "DataManager.h"

DataManager::DataManager()
{
}

DataManager::~DataManager()
{
	Release();
}

void DataManager::Init()
{
	// 오브젝트 리소스 등록 (Function.h의 정적 테이블에서 직접 가져와서 캐싱)
	for (size_t i = 0; i < DataTable::ObjectResourceCount; ++i) {
		const auto& entry = DataTable::ObjectResourceTable[i];
		ResourcePathUtils::ObjectResourceDef def;
		def.id = entry.id;
		def.baseDir = entry.baseDir;
		def.imageName = entry.imageName;
		RegisterObjectResource(entry.id, def);
	}

	// GameData/object_resource_overrides.txt 로드 후 id별 pivot/콜라이더 덮어쓰기 (Function.h 공통 파서 사용)
	ResourcePathUtils::ParseObjectResourceOverridesFile(L"GameData/object_resource_overrides.txt",
		[this](GameObjectID id, const ResourcePathUtils::ObjectResourceDef& overrideDef) {
			auto it = m_objectResources.find(id);
			if (it != m_objectResources.end()) {
				it->second.pivotX = overrideDef.pivotX;
				it->second.pivotY = overrideDef.pivotY;
				it->second.hasCollider = overrideDef.hasCollider;
				it->second.colliderType = overrideDef.colliderType;
				it->second.colliderOffsetX = overrideDef.colliderOffsetX;
				it->second.colliderOffsetY = overrideDef.colliderOffsetY;
				it->second.colliderWidth = overrideDef.colliderWidth;
				it->second.colliderHeight = overrideDef.colliderHeight;
				it->second.colliderCenterX = overrideDef.colliderCenterX;
				it->second.colliderCenterY = overrideDef.colliderCenterY;
				it->second.colliderRadius = overrideDef.colliderRadius;
			}
		});
}

void DataManager::Release()
{
	m_objectResources.clear();
}

void DataManager::RegisterObjectResource(GameObjectID id, const ResourcePathUtils::ObjectResourceDef& data)
{
	m_objectResources[id] = data;
}

const ResourcePathUtils::ObjectResourceDef* DataManager::GetObjectResourceInfo(GameObjectID id) const
{
	auto it = m_objectResources.find(id);
	if (it != m_objectResources.end()) {
		return &(it->second);
	}
	return nullptr;
}
