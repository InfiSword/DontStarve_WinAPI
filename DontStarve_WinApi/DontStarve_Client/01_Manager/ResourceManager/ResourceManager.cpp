#include "../../99_Default/pch.h"
#include "ResourceManager.h"

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
	Release();
}

void ResourceManager::Init()
{
	// 기본 리소스 로드
	LoadResourcesFromFile(L"../Resource/resources.txt");
}

void ResourceManager::Release()
{
	m_objectResources.clear();
	m_tileResources.clear();
}

// 파일 로드 함수 (기본 구현)
void ResourceManager::LoadResourcesFromFile(const std::wstring& filePath)
{
	std::wifstream file(filePath);

	if (!file.is_open()) {
		return;
	}

	std::wstring line;
	while (std::getline(file, line)) {
		if (line.empty() || line[0] == L'#') continue;

		std::wstringstream ss(line);
		std::wstring type, id, resourcePath, imageName;

		if (std::getline(ss, type, L',') &&
			std::getline(ss, id, L',') &&
			std::getline(ss, resourcePath, L',')) {

			// 이미지 파일명을 선택적으로 읽기 (플레이어와 같은 경우에는 없을 수 있음)
			std::getline(ss, imageName, L',');

			// 공백 제거
			type.erase(0, type.find_first_not_of(L" \t"));
			type.erase(type.find_last_not_of(L" \t") + 1);
			id.erase(0, id.find_first_not_of(L" \t"));
			id.erase(id.find_last_not_of(L" \t") + 1);
			resourcePath.erase(0, resourcePath.find_first_not_of(L" \t"));
			resourcePath.erase(resourcePath.find_last_not_of(L" \t") + 1);
			imageName.erase(0, imageName.find_first_not_of(L" \t"));
			imageName.erase(imageName.find_last_not_of(L" \t") + 1);

			if (type.find(L"TILE_") == 0) {
				// 타일 리소스
				TileID tileID = EnumUtils::GetEnumValue<TileID>(id.c_str(), TILEID_NONE);
				if (tileID != TILEID_NONE) {
					TileData tileData;
					tileData.id = tileID;
					tileData.tileAssetBaseDirectory = resourcePath;
					tileData.tileImageName = imageName;
					m_tileResources[tileID] = tileData;
				}
			}
			else {
				// 게임오브젝트 리소스
				GameObjectID objID = EnumUtils::GetEnumValue<GameObjectID>(id.c_str(), GOID_NONE);
				GameObjectData objData;
				objData.id = objID;
				objData.type = EnumUtils::GetEnumValue<GameObjectType>(type.c_str(), GOBJ_NONE);
				objData.objectAssetBaseDirectory = resourcePath;
				objData.assetImageName = imageName;
				objData.pivotX = 0.5f;
				objData.pivotY = 1.0f;
				m_objectResources[objID] = objData;

			}
		}
	}

	file.close();
}

const GameObjectData* ResourceManager::GetObjectResourceInfo(GameObjectID id) const
{
	auto it = m_objectResources.find(id);
	if (it != m_objectResources.end()) {
		return &(it->second);
	}
	return nullptr;
}

const TileData* ResourceManager::GetTileResourceInfo(TileID id) const
{
	auto it = m_tileResources.find(id);
	if (it != m_tileResources.end()) {
		return &(it->second);
	}
	return nullptr;
}

std::wstring ResourceManager::BuildResourcePath(const std::wstring& basePath, const std::wstring& subFolder, const std::wstring& filename) const
{
	std::wstring path = basePath;
	if (!subFolder.empty()) {
		path += L"/" + subFolder;
	}
	if (!filename.empty()) {
		path += L"/" + filename;
	}

	std::wstring relativePath = L"../" + path;

	// 경로 구분자를 백슬래시로 변환 (Windows 표준, 파일 시스템 호환)
	for (size_t i = 0; i < relativePath.length(); ++i) {
		if (relativePath[i] == L'/') {
			relativePath[i] = L'\\';
		}
	}

	// 상대 경로 파일 존재 확인
	DWORD fileAttributes = GetFileAttributesW(relativePath.c_str());
	if (fileAttributes != INVALID_FILE_ATTRIBUTES) {
		return relativePath;
	}

	// 상대경로가 실패하면 절대 경로 변환 시도
	wchar_t fullPath[MAX_PATH];
	if (GetFullPathNameW(relativePath.c_str(), MAX_PATH, fullPath, nullptr) > 0) {
		std::wstring absolutePath = std::wstring(fullPath);

		// 절대 경로도 파일 존재 여부 확인
		fileAttributes = GetFileAttributesW(absolutePath.c_str());
		if (fileAttributes != INVALID_FILE_ATTRIBUTES) {
			return absolutePath;
		}
	}

	return relativePath;
}

std::wstring ResourceManager::BuildObjectResourcePath(GameObjectID id, const std::wstring& subFolder, const std::wstring& filename) const
{
	const GameObjectData* resourceData = GetObjectResourceInfo(id);
	if (!resourceData) {
		return L"";
	}

	return BuildResourcePath(resourceData->objectAssetBaseDirectory, subFolder, filename);
}

std::wstring ResourceManager::BuildTileResourcePath(TileID id, const std::wstring& subFolder, const std::wstring& filename) const
{
	const TileData* resourceData = GetTileResourceInfo(id);
	if (!resourceData) {
		return L"";
	}

	return BuildResourcePath(resourceData->tileAssetBaseDirectory, subFolder, filename);
}