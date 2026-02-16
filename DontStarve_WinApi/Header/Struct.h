#pragma once
#include <windows.h>
#include <gdiplus.h>
#include "Define.h"
#include "Enum.h"
#include <cstddef> 
#include <string>
#include <cstring> 
#include <vector>
#include <map>
#include <cmath>

namespace EnumUtils {

	// EnumNamePair : 열거형과 문자열 이름을 묶는 구조체
	template<typename EnumType>
	struct EnumNamePair
	{
		EnumType value;
		const WCHAR* name;
	};

	// EnumTraits 기본 템플릿 선언
	template<typename T>
	struct EnumTraits;

	// EnumTraits 특수화
	// 배열 대신 std::vector 사용 가능 GetPairsArray()는 vector의 raw data 반환

	// TileType 특수화
	template<> struct EnumTraits<TileType>
	{
		static const std::vector<EnumNamePair<TileType>>& GetPairsVector() {
			static const std::vector<EnumNamePair<TileType>> pairs = {
				{ TILE_NONE,      L"TILE_NONE" },
				{ TILE_DIRT,      L"TILE_DIRT" },
				{ TILE_GRASS,     L"TILE_GRASS" },
				{ TILE_FOREST,    L"TILE_FOREST" },
				{ TILE_COUNT,     L"TILE_COUNT" }
			};
			return pairs;
		}
		static size_t GetPairsSize() { return GetPairsVector().size(); }
		static const EnumNamePair<TileType>* GetPairsArray() { return GetPairsVector().data(); }
	};

	// TileID 특수화
	template<> struct EnumTraits<TileID>
	{
		static const std::vector<EnumNamePair<TileID>>& GetPairsVector() {
			static const std::vector<EnumNamePair<TileID>> pairs = {
				{ TILEID_NONE,      L"TILEID_NONE" },
				{ TILEID_DIRT_00,   L"TILEID_DIRT_00" },
				{ TILEID_DIRT_01,   L"TILEID_DIRT_01" },
				{ TILEID_DIRT_02,   L"TILEID_DIRT_02" },
				{ TILEID_DIRT_03,   L"TILEID_DIRT_03" },
				{ TILEID_GRASS_00,  L"TILEID_GRASS_00" },
				{ TILEID_GRASS_01,  L"TILEID_GRASS_01" },
				{ TILEID_GRASS_02,  L"TILEID_GRASS_02" },
				{ TILEID_GRASS_03,  L"TILEID_GRASS_03" },
				{ TILEID_FOREST_00, L"TILEID_FOREST_00" },
				{ TILEID_FOREST_01, L"TILEID_FOREST_01" },
				{ TILEID_FOREST_02, L"TILEID_FOREST_02" },
				{ TILEID_FOREST_03, L"TILEID_FOREST_03" },
			};
			return pairs;
		}
		static size_t GetPairsSize() { return GetPairsVector().size(); }
		static const EnumNamePair<TileID>* GetPairsArray() { return GetPairsVector().data(); }
	};

	// GameObjectType 특수화
	template<> struct EnumTraits<GameObjectType>
	{
		static const std::vector<EnumNamePair<GameObjectType>>& GetPairsVector() {
			static const std::vector<EnumNamePair<GameObjectType>> pairs = {
				{ GOBJ_NONE,        L"GOBJ_NONE" },
				{ GOBJ_ITEM,        L"GOBJ_ITEM" },
				{ GOBJ_NATURAL_ENVIR, L"GOBJ_NATURAL_ENVIR" },
				{ GOBJ_MONSTER,     L"GOBJ_MONSTER" },
				{ GOBJ_BUILDING,    L"GOBJ_BUILDING" },
				{ GOBJ_UI,          L"GOBJ_UI" },
				{ GOBJ_COUNT,       L"GOBJ_COUNT" }
			};
			return pairs;
		}
		static size_t GetPairsSize() { return GetPairsVector().size(); }
		static const EnumNamePair<GameObjectType>* GetPairsArray() { return GetPairsVector().data(); }
	};

	// GameObjectID 특수화
	template<> struct EnumTraits<GameObjectID>
	{
		static const std::vector<EnumNamePair<GameObjectID>>& GetPairsVector() {
			static const std::vector<EnumNamePair<GameObjectID>> pairs = {
				{ GOID_NONE,                     L"GOID_NONE" },
				{ GOID_NORMAL_GRASS,             L"GOID_NORMAL_GRASS" },
				{ GOID_NORMAL_TREE_SHORT,        L"GOID_NORMAL_TREE_SHORT" },
				{ GOID_NORMAL_TREE_NORMAL,       L"GOID_NORMAL_TREE_NORMAL" },
				{ GOID_NORMAL_TREE_TALL,         L"GOID_NORMAL_TREE_TALL" },
				{ GOID_NORMAL_ROCK,              L"GOID_NORMAL_ROCK" },
				{ GOID_GOLD_ROCK,                L"GOID_GOLD_ROCK" },
				{ GOID_NORMAL_SAPLING,           L"GOID_NORMAL_SAPLING" },
				{ GOID_BERRY_TREE,               L"GOID_BERRY_TREE" },
				{ GOID_MONSTER_PIG,              L"GOID_MONSTER_PIG" },
				{ GOID_MONSTER_SPIDER,           L"GOID_MONSTER_SPIDER" },
				{ GOID_MONSTER_WARRIOR_SPIDER,   L"GOID_MONSTER_WARRIOR_SPIDER" },
				{ GOID_MONSTER_QUEEN_SPIDER,     L"GOID_MONSTER_QUEEN_SPIDER" },
				{ GOID_MONSTER_HOUNDDOG,         L"GOID_MONSTER_HOUNDDOG" },
				{ GOID_MONSTER_REDHOUNDDOG,      L"GOID_MONSTER_REDHOUNDDOG" },
				{ GOID_MONSTER_ICEHOUNDDOG,      L"GOID_MONSTER_ICEHOUNDDOG" },
				{ GOID_BUILDING_PIGHOUSE,        L"GOID_BUILDING_PIGHOUSE" },
				{ GOID_BUILDING_SPIDER_SMALLEGG, L"GOID_BUILDING_SPIDER_SMALLEGG" },
				{ GOID_BUILDING_SPIDER_NORMALEGG, L"GOID_BUILDING_SPIDER_NORMALEGG" },
				{ GOID_BUILDING_SPIDER_TALLEGG,  L"GOID_BUILDING_SPIDER_TALLEGG" },
				{ GOID_ITEM_CUT_NORMAL_GRASS,    L"GOID_ITEM_CUT_NORMAL_GRASS" },
				{ GOID_ITEM_NORMAL_TREE_LOG,     L"GOID_ITEM_NORMAL_TREE_LOG" },
				{ GOID_ITEM_NORMAL_TWIGS,        L"GOID_ITEM_NORMAL_TWIGS" },
				{ GOID_ITEM_NORMAL_ROCK,         L"GOID_ITEM_NORMAL_ROCK" },
				{ GOID_ITEM_GOLD_ROCK,           L"GOID_ITEM_GOLD_ROCK" },
				{ GOID_ITEM_ROPE,                L"GOID_ITEM_ROPE" },
				{ GOID_ITEM_CUT_NORMAL_STONE,    L"GOID_ITEM_CUT_NORMAL_STONE" },
				{ GOID_ITEM_MEAT,                L"GOID_ITEM_MEAT" },
				{ GOID_ITEM_BERRY,               L"GOID_ITEM_BERRY" },
				{ GOID_ITEM_AXE,                 L"GOID_ITEM_AXE" },
				{ GOID_ITEM_PICKAXE,             L"GOID_ITEM_PICKAXE" },
				{ GOID_PLAYER_WILSON,            L"GOID_PLAYER_WILSON" },
				{ GOID_PLAYER_WILLOW,            L"GOID_PLAYER_WILLOW" },
				{ GOID_PLAYER_WOLFGANG,          L"GOID_PLAYER_WOLFGANG" },
				{ GOID_PLAYER_INFO,              L"GOID_PLAYER_INFO" },
				{ GOID_PLAYER_PORTRAIT,          L"GOID_PLAYER_PORTRAIT" },
				{ GOID_MAIN_BG,                  L"GOID_MAIN_BG" },
				{ GOID_GAME_LOGO,                L"GOID_GAME_LOGO" },
				{ GOID_BUTTON1,                  L"GOID_BUTTON1" },
				{ GOID_ENDBUTTON1,               L"GOID_ENDBUTTON1" },
				{ GOID_SELECT_BUTTON,            L"GOID_SELECT_BUTTON" },
				{ GOID_CANCEL_SELECTION,         L"GOID_CANCEL_SELECTION" },
				{ GOID_BACK_BUTTON,              L"GOID_BACK_BUTTON" },
			};
			return pairs;
		}
		static size_t GetPairsSize() { return GetPairsVector().size(); }
		static const EnumNamePair<GameObjectID>* GetPairsArray() { return GetPairsVector().data(); }
	};

	template<typename EnumType>
	inline const WCHAR* GetEnumName(EnumType value, const WCHAR* defaultName = L"UNKNOWN_ENUM_VALUE") {
		const EnumNamePair<EnumType>* pairs = EnumTraits<EnumType>::GetPairsArray();
		size_t size = EnumTraits<EnumType>::GetPairsSize();
		for (size_t i = 0; i < size; ++i) {
			if (pairs[i].value == value) {
				return pairs[i].name;
			}
		}
		return defaultName;
	}

	template<typename EnumType>
	inline EnumType GetEnumValue(const WCHAR* name, EnumType defaultValue) {
		const EnumNamePair<EnumType>* pairs = EnumTraits<EnumType>::GetPairsArray();
		size_t size = EnumTraits<EnumType>::GetPairsSize();
		for (size_t i = 0; i < size; ++i) {
			// wstring 비교
			if (std::wcscmp(pairs[i].name, name) == 0) {
				return pairs[i].value;
			}
		}
		return defaultValue;
	}
}


// ====================== 데이터 구조 =======================

// 리소스 경로 관련 구조체
namespace ResourcePathUtils
{
	// 타일 리소스 정의 구조체 (절대경로로 확정)
	struct TileResourceDef {
		TileType type;
		TileID id;
		std::wstring baseDir;    // 절대경로 (리소스 기본 디렉토리)
		std::wstring imageName;  // 절대경로 (이미지 파일명 포함 전체 경로)

		TileResourceDef()
			: type(TILE_NONE), id(TILEID_NONE)
		{}

		TileResourceDef(TileType _type, TileID _id, const std::wstring& _baseDir, const std::wstring& _imageName)
			: type(_type), id(_id), baseDir(_baseDir), imageName(_imageName)
		{}
	};

	// 오브젝트 리소스 정의 구조체 (절대경로로 확정)
	struct ObjectResourceDef {
		GameObjectType type;
		GameObjectID id;
		std::wstring baseDir;    // 절대경로 (리소스 기본 디렉토리)
		std::wstring imageName;  // 절대경로 (이미지 파일명 포함 전체 경로)
		float pivotX;
		float pivotY;
		
		// 게임 오브젝트 데이터용 필드
		float x = 0, y = 0;  // 위치
		
		// 콜라이더 정보
		bool hasCollider = false;
		ColliderType colliderType = COLLIDER_BOX;
		int colliderOffsetX = 0;
		int colliderOffsetY = 0;
		int colliderWidth = 0;
		int colliderHeight = 0;
		float colliderCenterX = 0.0f;
		float colliderCenterY = 0.0f;
		float colliderRadius = 0.0f;

		ObjectResourceDef()
			: type(GOBJ_NONE), id(GOID_NONE), pivotX(0.5f), pivotY(1.0f), x(0), y(0)
		{}

		ObjectResourceDef(GameObjectType type_val, GameObjectID id_val, float x_val, float y_val,
			const std::wstring& baseDir_val, const std::wstring& imageName_val, float pivotX_val, float pivotY_val,
			bool hasCollider_val = false, ColliderType colliderType_val = COLLIDER_BOX,
			int colliderOffsetX_val = 0, int colliderOffsetY_val = 0,
			int colliderWidth_val = 0, int colliderHeight_val = 0,
			float colliderCenterX_val = 0.0f, float colliderCenterY_val = 0.0f, float colliderRadius_val = 0.0f)
			: type(type_val), id(id_val), baseDir(baseDir_val), imageName(imageName_val),
			pivotX(pivotX_val), pivotY(pivotY_val), x(x_val), y(y_val),
			hasCollider(hasCollider_val), colliderType(colliderType_val),
			colliderOffsetX(colliderOffsetX_val), colliderOffsetY(colliderOffsetY_val),
			colliderWidth(colliderWidth_val), colliderHeight(colliderHeight_val),
			colliderCenterX(colliderCenterX_val), colliderCenterY(colliderCenterY_val), colliderRadius(colliderRadius_val)
		{}
	};
}

struct PaletteItem {
	ItemCategory category;     // CATEGORY_TILE, CATEGORY_OBJECT
	int typeId;                // TileType 또는 GameObjectType
	UINT resourceId;           // 리소스 ID (TileID 또는 GameObjectID)

	RECT displayRect;
	Gdiplus::Bitmap* hBitmap;  // 아이콘 표시용 비트맵
	Gdiplus::RectF iconSourceRect;
};

// 화면에 그릴 정보를 담는 렌더 명령 데이터
struct DrawCommand {
	DrawCommandType type = DRAW_COMMAND_IMAGE;
	Gdiplus::Bitmap* pBitmap = nullptr;
	Gdiplus::RectF destRect = Gdiplus::RectF(0, 0, 0, 0);
	Gdiplus::RectF sourceRect = Gdiplus::RectF(0, 0, 0, 0);
	Gdiplus::Unit srcUnit = Gdiplus::UnitPixel;
	Gdiplus::PointF objectScreenPos = Gdiplus::PointF(0, 0);
	RenderLayer layer = LAYER_NONE;
	float sortKey = 0.0f;
	Direction direction = DIR_DOWN;

	std::wstring text;
	Gdiplus::Font* pFont = nullptr;
	Gdiplus::Brush* pBrush = nullptr;
	Gdiplus::StringFormat* pStringFormat = nullptr;

	Gdiplus::Color color = Gdiplus::Color(0, 0, 0, 0);
	float thickness = 0.0f;
	Gdiplus::Color tintColor = Gdiplus::Color(255, 255, 255, 255);  // 색상 틴트 (기본값: 흰색, 덮어쓰기 방식 - Unity Sprite 스타일)
	bool hasTint = false;  // 틴트 적용 여부
};

// 플레이어 스폰 위치 정보
struct PlayerSpawnData {
	float x, y;

	PlayerSpawnData() : x(0), y(0) {}
	PlayerSpawnData(float _x, float _y) : x(_x), y(_y) {}
};

// 맵 전체 데이터를 담는 구조체 (에디터 또는 게임)
struct MapData {
	std::wstring mapName;        // 맵 이름 (예: "Stage1", "Forest")
	std::wstring mapFilePath;    // .dsm 파일 경로

	int mapWidth;
	int mapHeight;
	PlayerSpawnData playerSpawn; // 플레이어 스폰 시작 위치

	ResourcePathUtils::TileResourceDef tiles[MAP_WIDTH][MAP_HEIGHT];        // 타일 데이터
	std::vector<ResourcePathUtils::ObjectResourceDef> gameObjects;      // 게임 오브젝트 리스트
	bool walkableAreas[MAP_WIDTH][MAP_HEIGHT];    // 이동 가능 영역

	MapData()
		: mapName(L""), mapFilePath(L""), mapWidth(MAP_WIDTH), mapHeight(MAP_HEIGHT)
	{
		for (int y = 0; y < MAP_HEIGHT; ++y)
			for (int x = 0; x < MAP_WIDTH; ++x) {
				tiles[x][y] = ResourcePathUtils::TileResourceDef();
				walkableAreas[x][y] = true;
			}
	}

	MapData(const std::wstring& name, const std::wstring& filePath)
		: mapName(name), mapFilePath(filePath), mapWidth(MAP_WIDTH), mapHeight(MAP_HEIGHT)
	{
		for (int y = 0; y < MAP_HEIGHT; ++y)
			for (int x = 0; x < MAP_WIDTH; ++x) {
				tiles[x][y] = ResourcePathUtils::TileResourceDef();
				walkableAreas[x][y] = true;
			}
	}
};

// 씬(맵) 클리어 정보 구조체
struct SceneClearInfo
{
	SceneType sceneType;
	bool isCleared;
	std::wstring clearDate;  // 클리어 날짜(문자열)

	SceneClearInfo(SceneType type, bool cleared = false, const std::wstring& date = L"")
		: sceneType(type), isCleared(cleared), clearDate(date) {}
};

// 씬 클리어 조건 정보 구조체
struct SceneClearCondition
{
	SceneType sceneType;
	std::wstring conditionDescription;  // 클리어 조건 설명
	std::vector<GameObjectID> requiredKills;   // 처치해야 하는 오브젝트 ID
	std::vector<GameObjectID> requiredItems;   // 모아야 하는 아이템 ID
	int requiredItemCount;      // 필요 아이템 수량

	SceneClearCondition(SceneType type, const std::wstring& desc,
		const std::vector<GameObjectID>& kills = {},
		const std::vector<GameObjectID>& items = {}, int count = 0)
		: sceneType(type), conditionDescription(desc),
		requiredKills(kills), requiredItems(items), requiredItemCount(count) {}
};

// 캐릭터 해금 정보 구조체
struct CharacterUnlockInfo
{
	GameObjectID characterID;
	bool isUnlocked;
	SceneType requiredScene;  // 해금에 필요한 씬

	CharacterUnlockInfo(GameObjectID id, bool unlocked = false, SceneType scene = SCENE_NONE)
		: characterID(id), isUnlocked(unlocked), requiredScene(scene) {}
};

// 게임 전체 진행도 저장 구조체
struct GameProgress
{
	std::vector<SceneClearInfo> sceneClearInfos;
	std::vector<CharacterUnlockInfo> characterUnlockInfos;
	std::vector<SceneClearCondition> sceneClearConditions;

	GameProgress()
	{
		// 기본 클리어 정보
		sceneClearInfos.emplace_back(SCENE_GAME_FARMING_AREA, true);      // 기본 해금
		sceneClearInfos.emplace_back(SCENE_GAME_HOUND_FOREST, false);
		sceneClearInfos.emplace_back(SCENE_GAME_SPIDER_QUEEN_HOUSE, false);

		// 기본 캐릭터 해금 정보
		characterUnlockInfos.emplace_back(GOID_PLAYER_WILSON, true, SCENE_NONE);
		characterUnlockInfos.emplace_back(GOID_PLAYER_WILLOW, false, SCENE_GAME_HOUND_FOREST);
		characterUnlockInfos.emplace_back(GOID_PLAYER_WOLFGANG, false, SCENE_GAME_SPIDER_QUEEN_HOUSE);

		// 클리어 조건 초기화
		InitializeSceneClearConditions();
	}

	// 씬별 클리어 조건 초기화
	void InitializeSceneClearConditions()
	{
		sceneClearConditions.emplace_back(
			SCENE_GAME_FARMING_AREA,
			L"기본으로 열려 있는 지역입니다."
		);

		sceneClearConditions.emplace_back(
			SCENE_GAME_HOUND_FOREST,
			L"하운드 몬스터들을 처치하세요.",
			std::vector<GameObjectID>{ GOID_MONSTER_HOUNDDOG, GOID_MONSTER_REDHOUNDDOG, GOID_MONSTER_ICEHOUNDDOG }
		);

		sceneClearConditions.emplace_back(
			SCENE_GAME_SPIDER_QUEEN_HOUSE,
			L"거미여왕을 처치하고 특정 아이템을 모으세요.",
			std::vector<GameObjectID>{ GOID_MONSTER_QUEEN_SPIDER },
			std::vector<GameObjectID>{ GOID_ITEM_MEAT, GOID_ITEM_BERRY },
			5
		);
	}

	// 특정 씬 클리어 조건 가져오기
	const SceneClearCondition* GetSceneClearCondition(SceneType sceneType) const
	{
		for (const auto& condition : sceneClearConditions)
			if (condition.sceneType == sceneType)
				return &condition;
		return nullptr;
	}

	// 씬 클리어 여부 확인
	bool IsSceneCleared(SceneType sceneType) const
	{
		for (const auto& sceneInfo : sceneClearInfos)
			if (sceneInfo.sceneType == sceneType)
				return sceneInfo.isCleared;
		return false;
	}

	// 캐릭터 해금 여부 확인
	bool IsCharacterUnlocked(GameObjectID characterID) const
	{
		for (const auto& charInfo : characterUnlockInfos)
			if (charInfo.characterID == characterID)
				return charInfo.isUnlocked;
		return false;
	}

	// 씬 클리어 처리
	void ClearScene(SceneType sceneType)
	{
		for (auto& sceneInfo : sceneClearInfos)
		{
			if (sceneInfo.sceneType == sceneType)
			{
				sceneInfo.isCleared = true;
				UpdateCharacterUnlocks(); // 캐릭터 자동 해금
				break;
			}
		}
	}

	// 캐릭터 해금 업데이트
	void UpdateCharacterUnlocks()
	{
		for (auto& charInfo : characterUnlockInfos)
		{
			if (charInfo.requiredScene != SCENE_NONE)
				charInfo.isUnlocked = IsSceneCleared(charInfo.requiredScene);
		}
	}
};

struct AnimationFrame {
	Gdiplus::RectF sourceRect;
	float duration;
	UINT width;
	UINT height;
	float pivotX;
	float pivotY;

	AnimationFrame()
		: sourceRect(0, 0, 0, 0), duration(0.1f),
		width(0), height(0), pivotX(0.5f), pivotY(0.5f)
	{}

	AnimationFrame(const Gdiplus::RectF& rect, float dur, UINT w, UINT h)
		: sourceRect(rect), duration(dur),
		width(w), height(h), pivotX(0.5f), pivotY(0.5f)
	{}

	AnimationFrame(const Gdiplus::RectF& rect, float dur, UINT w, UINT h, float px, float py)
		: sourceRect(rect), duration(dur),
		width(w), height(h), pivotX(px), pivotY(py)
	{}
};
