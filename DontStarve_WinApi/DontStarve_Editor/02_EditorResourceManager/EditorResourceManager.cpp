#include "../pch.h"
#include "EditorResourceManager.h"
#include "../../Header/Function.h"
#include "../../Header/Enum.h"
#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>

ObjectCategory EditorResourceManager::GetCategoryFromID(GameObjectID id)
{
	if (id >= 1 && id < 100) return ObjectCategory::Natural;
	if (id >= 100 && id < 200) return ObjectCategory::Monster;
	if (id >= 200 && id < 300) return ObjectCategory::Building;
	if (id >= 300 && id < 500) return ObjectCategory::Item;
	if (id >= 1000 && id < 2000) return ObjectCategory::Player;
	return ObjectCategory::Count;
}

void EditorResourceManager::LoadResources()
{
	ReleaseResources();

	OutputDebugStringW(L"=== Editor: Loading resources from code ===\n");

	// 타일 리소스 로드
	for (size_t i = 0; i < DataTable::TileResourceCount; ++i) {
		const auto& def = DataTable::TileResourceTable[i];
		m_tileVariants[def.type][def.id] = ResourcePathUtils::TileResourceDef(def.type, def.id, def.baseDir, def.imageName);
	}

	// 오브젝트 리소스 로드 (flat map 구조)
	for (size_t i = 0; i < DataTable::ObjectResourceCount; ++i) {
		const auto& def = DataTable::ObjectResourceTable[i];
		// Wilson 플레이어만 포함하고 나머지 플레이어는 제외
		if (def.id >= 1000 && def.id < 2000 && def.id != GOID_PLAYER_WILSON) continue; 

		m_objectVariants[def.id] = ResourcePathUtils::ObjectResourceDef(def.id, 0, 0, def.baseDir, def.imageName, 0.5f, 1.0f);
	}

	ApplyInitialObjectValuesToAll();

	if (ObjectResourceOverridesFileExists())
		LoadObjectResourceOverrides();
	else
		SaveAllObjectResourceOverrides();

	OutputDebugStringW((L"Final Tile Variants Map Size: " + std::to_wstring(m_tileVariants.size()) + L"\n").c_str());
	OutputDebugStringW((L"Final Object Variants Map Size: " + std::to_wstring(m_objectVariants.size()) + L"\n").c_str());
}

void EditorResourceManager::ReleaseResources()
{
	m_tileVariants.clear();
	m_objectVariants.clear();
	ClearBitmapCache();
}

const ResourcePathUtils::TileResourceDef* EditorResourceManager::GetTileVariant(TileType type, TileID id) const
{
	auto type_it = m_tileVariants.find(type);
	if (type_it != m_tileVariants.end()) {
		auto id_it = type_it->second.find(id);
		if (id_it != type_it->second.end()) {
			return &(id_it->second);
		}
	}
	return nullptr;
}

const ResourcePathUtils::ObjectResourceDef* EditorResourceManager::GetObjectVariant(GameObjectID id) const
{
	auto it = m_objectVariants.find(id);
	if (it != m_objectVariants.end()) {
		return &(it->second);
	}
	return nullptr;
}

std::wstring EditorResourceManager::GetResourceRoot()
{
	return L".";
}

std::shared_ptr<Gdiplus::Bitmap> EditorResourceManager::GetCachedBitmap(const std::wstring& fullPath)
{
	std::wstring pathToUse = fullPath;
	
	// 상대 경로인 경우 현재 디렉터리(프로젝트 루트)를 기준으로 처리
	// EnsureResourceWorkingDirectory()에 의해 이미 작업 디렉터리가 루트로 설정되어 있음

	auto it = m_bitmapCache.find(pathToUse);
	if (it != m_bitmapCache.end()) {
		return it->second;
	}

	Gdiplus::Bitmap* pBitmap = Gdiplus::Bitmap::FromFile(pathToUse.c_str());
	if (pBitmap && pBitmap->GetLastStatus() == Gdiplus::Ok) {
		auto sharedBitmap = std::shared_ptr<Gdiplus::Bitmap>(pBitmap);
		m_bitmapCache[pathToUse] = sharedBitmap;
		return sharedBitmap;
	}
	return nullptr;
}

void EditorResourceManager::ClearBitmapCache()
{
	m_bitmapCache.clear();
}

std::wstring EditorResourceManager::GetGameDataPath()
{
	return L"GameData";
}

bool EditorResourceManager::ObjectResourceOverridesFileExists()
{
	std::wstring path = GetGameDataPath();
	if (path.empty()) return false;
	if (path.back() != L'\\') path += L"\\";
	path += L"object_resource_overrides.txt";
	DWORD attrs = GetFileAttributesW(path.c_str());
	return (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY));
}

void EditorResourceManager::ApplyInitialObjectValuesToAll()
{
	const float initPivotX = 0.5f;
	const float initPivotY = 1.0f;
	for (auto& pair : m_objectVariants) {
		ResourcePathUtils::ObjectResourceDef& def = pair.second;
		def.pivotX = initPivotX;
		def.pivotY = initPivotY;
		std::wstring fullPath = def.baseDir;
		if (!fullPath.empty() && fullPath.back() != L'\\' && fullPath.back() != L'/') fullPath += L"\\";
		fullPath += def.imageName;
		std::shared_ptr<Gdiplus::Bitmap> pBitmap = GetCachedBitmap(fullPath);
		if (pBitmap) {
			int iw = pBitmap->GetWidth();
			int ih = pBitmap->GetHeight();
			def.hasCollider = true;
			def.colliderType = COLLIDER_BOX;
			def.colliderOffsetX = -(int)(initPivotX * iw);
			def.colliderOffsetY = -(int)(initPivotY * ih);
			def.colliderWidth = iw;
			def.colliderHeight = ih;
			def.colliderCenterX = iw * (0.5f - initPivotX);
			def.colliderCenterY = ih * (0.5f - initPivotY);
			def.colliderRadius = (float)((iw < ih) ? iw : ih) * 0.5f;
		}
	}
}

void EditorResourceManager::LoadObjectResourceOverrides()
{
	std::wstring dir = GetGameDataPath();
	if (dir.empty()) return;
	if (dir.back() != L'\\') dir += L"\\";
	std::wstring path = dir + L"object_resource_overrides.txt";
	ResourcePathUtils::ParseObjectResourceOverridesFile(path,
		[this](GameObjectID id, const ResourcePathUtils::ObjectResourceDef& overrideDef) {
			auto it = m_objectVariants.find(id);
			if (it == m_objectVariants.end()) return;
			ResourcePathUtils::ObjectResourceDef& def = it->second;
			def.pivotX = overrideDef.pivotX;
			def.pivotY = overrideDef.pivotY;
			def.hasCollider = overrideDef.hasCollider;
			def.colliderType = overrideDef.colliderType;
			def.colliderOffsetX = overrideDef.colliderOffsetX;
			def.colliderOffsetY = overrideDef.colliderOffsetY;
			def.colliderWidth = overrideDef.colliderWidth;
			def.colliderHeight = overrideDef.colliderHeight;
			def.colliderCenterX = overrideDef.colliderCenterX;
			def.colliderCenterY = overrideDef.colliderCenterY;
			def.colliderRadius = overrideDef.colliderRadius;
		});
}

bool EditorResourceManager::SaveObjectResourceOverride(GameObjectID id, const ResourcePathUtils::ObjectResourceDef& def)
{
	auto it = m_objectVariants.find(id);
	if (it == m_objectVariants.end()) return false;

	it->second.pivotX = def.pivotX;
	it->second.pivotY = def.pivotY;
	it->second.hasCollider = def.hasCollider;
	it->second.colliderType = def.colliderType;
	it->second.colliderOffsetX = def.colliderOffsetX;
	it->second.colliderOffsetY = def.colliderOffsetY;
	it->second.colliderWidth = def.colliderWidth;
	it->second.colliderHeight = def.colliderHeight;
	it->second.colliderCenterX = def.colliderCenterX;
	it->second.colliderCenterY = def.colliderCenterY;
	it->second.colliderRadius = def.colliderRadius;

	std::wstring dir = GetGameDataPath();
	std::wstring path = dir + L"\\object_resource_overrides.txt";

	std::vector<std::wstring> lines;
	std::map<GameObjectID, std::wstring> overrideLines;
	{
		std::wifstream ifs(path);
		if (ifs.is_open()) {
			std::wstring line;
			while (std::getline(ifs, line)) {
				lines.push_back(line);
				if (line.empty() || line[0] == L'#') continue;
				std::wistringstream iss(line);
				std::wstring idName;
				if (iss >> idName) {
					GameObjectID i = EnumTables::GetGameObjectID(idName.c_str());
					if (i != GOID_NONE)
						overrideLines[i] = line;
				}
			}
		}
	}

	std::wostringstream oss;
	oss << EnumTables::GetEnumName(id)
		<< L" " << def.pivotX << L" " << def.pivotY
		<< L" " << (def.hasCollider ? 1 : 0) << L" " << (int)def.colliderType
		<< L" " << def.colliderOffsetX << L" " << def.colliderOffsetY
		<< L" " << def.colliderWidth << L" " << def.colliderHeight
		<< L" " << def.colliderCenterX << L" " << def.colliderCenterY << L" " << def.colliderRadius;
	overrideLines[id] = oss.str();

	CreateDirectoryW(dir.c_str(), nullptr);

	std::wofstream ofs(path);
	if (!ofs.is_open()) return false;
	ofs << L"# object_resource_overrides.txt - pivot and collider from Object Editor\n";
	ofs << L"# IDName pivotX pivotY hasCollider colliderType offsetX offsetY width height centerX centerY radius\n";
	for (const auto& kv : overrideLines)
		ofs << kv.second << L"\n";
	return true;
}

bool EditorResourceManager::SaveAllObjectResourceOverrides()
{
	std::wstring dir = GetGameDataPath();
	std::wstring path = dir + L"\\object_resource_overrides.txt";
	CreateDirectoryW(dir.c_str(), nullptr);

	std::wofstream ofs(path);
	if (!ofs.is_open()) return false;
	ofs << L"# object_resource_overrides.txt - pivot and collider from Object Editor\n";
	ofs << L"# IDName pivotX pivotY hasCollider colliderType offsetX offsetY width height centerX centerY radius\n";
	for (const auto& pair : m_objectVariants) {
		GameObjectID id = pair.first;
		const ResourcePathUtils::ObjectResourceDef& def = pair.second;
		ofs << EnumTables::GetEnumName(id)
			<< L" " << def.pivotX << L" " << def.pivotY
			<< L" " << (def.hasCollider ? 1 : 0) << L" " << (int)def.colliderType
			<< L" " << def.colliderOffsetX << L" " << def.colliderOffsetY
			<< L" " << def.colliderWidth << L" " << def.colliderHeight
			<< L" " << def.colliderCenterX << L" " << def.colliderCenterY << L" " << def.colliderRadius << L"\n";
	}
	return true;
}
