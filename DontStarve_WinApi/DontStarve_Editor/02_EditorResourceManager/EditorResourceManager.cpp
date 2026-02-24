#include "../pch.h"
#include "EditorResourceManager.h"
#include "../../Header/Function.h"
#include "../../Header/Enum.h"
#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>

void EditorResourceManager::LoadResources()
{
	ReleaseResources();


	OutputDebugStringW(L"=== Loading resources from code ===\n");

	// 정적 테이블에서 직접 로드
	for (size_t i = 0; i < ResourcePathUtils::TileResourceCount; ++i) {
		const auto& def = ResourcePathUtils::TileResourceTable[i];
		// 절대경로로 직접 저장 (아틀라스 사용 안 함)
		m_tileVariants[def.type][def.id] = ResourcePathUtils::TileResourceDef(def.type, def.id, def.baseDir, def.imageName);
	}

	// 정적 테이블에서 직접 로드
	for (size_t i = 0; i < ResourcePathUtils::ObjectResourceCount; ++i) {
		const auto& def = ResourcePathUtils::ObjectResourceTable[i];
		m_objectVariants[def.type][def.id] = ResourcePathUtils::ObjectResourceDef(def.type, def.id, 0, 0, def.baseDir, def.imageName, 0.5f, 1.0f);
	}

	// ObjectEditor 진입 시점: 먼저 모든 오브젝트를 초기값으로 셋팅 (pivot 0.5/1.0, box = 이미지 크기)
	ApplyInitialObjectValuesToAll();

	// 해당 경로에 파일이 있으면 로드하여 덮어씀. 없으면 GameData 폴더와 파일을 초기값으로 생성
	if (ObjectResourceOverridesFileExists())
		LoadObjectResourceOverrides();
	else
		SaveAllObjectResourceOverrides();  // GameData + object_resource_overrides.txt 생성 (현재 초기값 기록)

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

const ResourcePathUtils::ObjectResourceDef* EditorResourceManager::GetObjectVariant(GameObjectType type, GameObjectID id) const
{
	auto type_it = m_objectVariants.find(type);
	if (type_it != m_objectVariants.end()) {
		auto id_it = type_it->second.find(id);
		if (id_it != type_it->second.end()) {
			return &(id_it->second);
		}
	}
	return nullptr;
}

std::wstring EditorResourceManager::GetResourceRoot()
{
	wchar_t modulePath[MAX_PATH] = {};
	GetModuleFileNameW(NULL, modulePath, MAX_PATH);
	wchar_t* winApiPos = wcsstr(modulePath, L"DontStarve_WinApi");
	if (winApiPos) {
		size_t len = wcslen(L"DontStarve_WinApi");
		std::wstring root(modulePath, winApiPos - modulePath + len);
		return root;
	}
	// 폴백: exe 디렉터리의 상위(..)를 프로젝트 루트로 간주
	wchar_t* lastSlash = wcsrchr(modulePath, L'\\');
	if (lastSlash) { *lastSlash = L'\0'; lastSlash = wcsrchr(modulePath, L'\\'); if (lastSlash) *lastSlash = L'\0'; }
	return std::wstring(modulePath);
}

std::shared_ptr<Gdiplus::Bitmap> EditorResourceManager::GetCachedBitmap(const std::wstring& fullPath)
{
	// 상대 경로면 프로젝트 루트 기준으로 절대 경로 생성 (Resource\... 형태)
	std::wstring pathToUse = fullPath;
	if (fullPath.size() >= 2 && fullPath[1] != L':' && fullPath[0] != L'\\') {
		std::wstring root = GetResourceRoot();
		if (!root.empty() && root.back() != L'\\') root += L"\\";
		pathToUse = root + fullPath;
	}

	// 캐시 조회 (원본 fullPath 키로 조회하여 동일 상대경로 호출 시 캐시 히트)
	auto it = m_bitmapCache.find(pathToUse);
	if (it != m_bitmapCache.end()) {
		return it->second;
	}

	// 캐시 미스 → 로드 후 캐싱
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
	std::wstring root = GetResourceRoot();
	if (!root.empty() && root.back() != L'\\') root += L"\\";
	return root + L"GameData";
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
	// 초기값: pivot 0.5, 1.0 / box 콜라이더 = 이미지 크기 (피벗 기준 오프셋)
	const float initPivotX = 0.5f;
	const float initPivotY = 1.0f;
	for (auto& typePair : m_objectVariants) {
		for (auto& idPair : typePair.second) {
			ResourcePathUtils::ObjectResourceDef& def = idPair.second;
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
}

void EditorResourceManager::LoadObjectResourceOverrides()
{
	std::wstring dir = GetGameDataPath();
	if (dir.empty()) return;
	if (dir.back() != L'\\') dir += L"\\";
	std::wstring path = dir + L"object_resource_overrides.txt";
	ResourcePathUtils::ParseObjectResourceOverridesFile(path,
		[this](GameObjectType type, GameObjectID id, const ResourcePathUtils::ObjectResourceDef& overrideDef) {
			auto itType = m_objectVariants.find(type);
			if (itType == m_objectVariants.end()) return;
			auto itId = itType->second.find(id);
			if (itId == itType->second.end()) return;
			ResourcePathUtils::ObjectResourceDef& def = itId->second;
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

bool EditorResourceManager::SaveObjectResourceOverride(GameObjectType type, GameObjectID id, const ResourcePathUtils::ObjectResourceDef& def)
{
	auto itType = m_objectVariants.find(type);
	if (itType == m_objectVariants.end()) return false;
	auto itId = itType->second.find(id);
	if (itId == itType->second.end()) return false;

	itId->second.pivotX = def.pivotX;
	itId->second.pivotY = def.pivotY;
	itId->second.hasCollider = def.hasCollider;
	itId->second.colliderType = def.colliderType;
	itId->second.colliderOffsetX = def.colliderOffsetX;
	itId->second.colliderOffsetY = def.colliderOffsetY;
	itId->second.colliderWidth = def.colliderWidth;
	itId->second.colliderHeight = def.colliderHeight;
	itId->second.colliderCenterX = def.colliderCenterX;
	itId->second.colliderCenterY = def.colliderCenterY;
	itId->second.colliderRadius = def.colliderRadius;

	std::wstring dir = GetGameDataPath();
	std::wstring path = dir + L"\\object_resource_overrides.txt";

	// 기존 파일 전체 로드 → (type,id) 키별로 한 줄씩 유지, 해당 키만 갱신 후 저장
	std::vector<std::wstring> lines;
	std::map<std::pair<GameObjectType, GameObjectID>, std::wstring> overrideLines;
	{
		std::wifstream ifs(path);
		if (ifs.is_open()) {
			std::wstring line;
			while (std::getline(ifs, line)) {
				lines.push_back(line);
				if (line.empty() || line[0] == L'#') continue;
				std::wistringstream iss(line);
				std::wstring typeName, idName;
				if (iss >> typeName >> idName) {
					GameObjectType t = EnumTables::GetGameObjectType(typeName.c_str());
					GameObjectID i = EnumTables::GetGameObjectID(idName.c_str());
					if (t != GOBJ_NONE && i != GOID_NONE)
						overrideLines[{ t, i }] = line;
				}
			}
		}
	}

	// 현재 (type, id) 한 줄 생성
	std::wostringstream oss;
	oss << EnumTables::GetEnumName(type) << L" " << EnumTables::GetEnumName(id)
		<< L" " << def.pivotX << L" " << def.pivotY
		<< L" " << (def.hasCollider ? 1 : 0) << L" " << (int)def.colliderType
		<< L" " << def.colliderOffsetX << L" " << def.colliderOffsetY
		<< L" " << def.colliderWidth << L" " << def.colliderHeight
		<< L" " << def.colliderCenterX << L" " << def.colliderCenterY << L" " << def.colliderRadius;
	overrideLines[{ type, id }] = oss.str();

	// GameData 폴더 없으면 생성
	CreateDirectoryW(dir.c_str(), nullptr);

	std::wofstream ofs(path);
	if (!ofs.is_open()) return false;
	ofs << L"# object_resource_overrides.txt - pivot and collider from Object Editor\n";
	ofs << L"# TypeName IDName pivotX pivotY hasCollider colliderType offsetX offsetY width height centerX centerY radius\n";
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
	ofs << L"# TypeName IDName pivotX pivotY hasCollider colliderType offsetX offsetY width height centerX centerY radius\n";
	for (const auto& typePair : m_objectVariants) {
		GameObjectType type = typePair.first;
		for (const auto& idPair : typePair.second) {
			GameObjectID id = idPair.first;
			const ResourcePathUtils::ObjectResourceDef& def = idPair.second;
			ofs << EnumTables::GetEnumName(type) << L" " << EnumTables::GetEnumName(id)
				<< L" " << def.pivotX << L" " << def.pivotY
				<< L" " << (def.hasCollider ? 1 : 0) << L" " << (int)def.colliderType
				<< L" " << def.colliderOffsetX << L" " << def.colliderOffsetY
				<< L" " << def.colliderWidth << L" " << def.colliderHeight
				<< L" " << def.colliderCenterX << L" " << def.colliderCenterY << L" " << def.colliderRadius << L"\n";
		}
	}
	return true;
}
