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

// ====================== Enum 문자열 변환 테이블 =======================

namespace EnumTables {
	// TileType 테이블 엔트리 구조체
	struct TileTypeEntry {
		TileType value;
		const WCHAR* name;
	};

	// TileID 테이블 엔트리 구조체
	struct TileIDEntry {
		TileID value;
		const WCHAR* name;
	};

	// GameObjectID 테이블 엔트리 구조체
	struct GameObjectIDEntry {
		GameObjectID value;
		const WCHAR* name;
	};

	// TileType 테이블
	static constexpr TileTypeEntry TileTypeTable[] = {
		{ TILE_NONE,   L"TILE_NONE" },
		{ TILE_DIRT,   L"TILE_DIRT" },
		{ TILE_GRASS,  L"TILE_GRASS" },
		{ TILE_FOREST, L"TILE_FOREST" },
		{ TILE_COUNT,  L"TILE_COUNT" }
	};

	// TileID 테이블
	static constexpr TileIDEntry TileIDTable[] = {
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
		{ TILEID_FOREST_03, L"TILEID_FOREST_03" }
	};

	// GameObjectID 테이블
	static constexpr GameObjectIDEntry GameObjectIDTable[] = {
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
		{ GOID_BUILDING_SPIDER_SACEGG,   L"GOID_BUILDING_SPIDER_SACEGG" },
		{ GOID_BUILDING_SPIDER_SMALLEGG, L"GOID_BUILDING_SPIDER_SMALLEGG" },
		{ GOID_BUILDING_SPIDER_NORMALEGG,L"GOID_BUILDING_SPIDER_NORMALEGG" },
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
		{ GOID_ITEM_WOOD_2,             L"GOID_ITEM_WOOD_2" },
		{ GOID_ITEM_SMALL_MEAT,         L"GOID_ITEM_SMALL_MEAT" },
		{ GOID_ITEM_MONSTER_MEAT,       L"GOID_ITEM_MONSTER_MEAT" },
		{ GOID_ITEM_COOKED_MONSTER_MEAT, L"GOID_ITEM_COOKED_MONSTER_MEAT" },
		{ GOID_ITEM_COOKED_SMALL_MEAT,  L"GOID_ITEM_COOKED_SMALL_MEAT" },
		{ GOID_ITEM_COOKED_MEAT,        L"GOID_ITEM_COOKED_MEAT" },
		{ GOID_TOOL_HALBERD,             L"GOID_TOOL_HALBERD" },
		{ GOID_TOOL_HAMMER,              L"GOID_TOOL_HAMMER" },
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
		{ GOID_BACK_BUTTON,              L"GOID_BACK_BUTTON" }
	};
}

// ====================== 리소스 경로 관련 구조체 =======================

namespace ResourcePathUtils
{	
	// 타일 리소스 정적 테이블 엔트리 (컴파일 타임 상수)
	struct TileDefEntry {
		TileType type;
		TileID id;
		const wchar_t* baseDir;
		const wchar_t* imageName;
	};

	// 오브젝트 리소스 정적 테이블 엔트리
	struct ObjectDefEntry {
		GameObjectID id;
		const wchar_t* baseDir;
		const wchar_t* imageName;
	};

	// ====================== 정적 리소스 테이블 =======================
	
	// 타일 리소스 정적 테이블
	static constexpr TileDefEntry TileResourceTable[] = {
		{ TILE_DIRT, TILEID_DIRT_00, L"Resource\\Tiles\\Dirt", L"dirt_01.png" },
		{ TILE_DIRT, TILEID_DIRT_01, L"Resource\\Tiles\\Dirt", L"dirt_02.png" },
		{ TILE_DIRT, TILEID_DIRT_02, L"Resource\\Tiles\\Dirt", L"dirt_03.png" },
		{ TILE_DIRT, TILEID_DIRT_03, L"Resource\\Tiles\\Dirt", L"dirt_04.png" },
		{ TILE_GRASS, TILEID_GRASS_00, L"Resource\\Tiles\\Grass", L"grass_01.png" },
		{ TILE_GRASS, TILEID_GRASS_01, L"Resource\\Tiles\\Grass", L"grass_02.png" },
		{ TILE_GRASS, TILEID_GRASS_02, L"Resource\\Tiles\\Grass", L"grass_03.png" },
		{ TILE_GRASS, TILEID_GRASS_03, L"Resource\\Tiles\\Grass", L"grass_04.png" },
		{ TILE_FOREST, TILEID_FOREST_00, L"Resource\\Tiles\\Forest", L"forest_01.png" },
		{ TILE_FOREST, TILEID_FOREST_01, L"Resource\\Tiles\\Forest", L"forest_02.png" },
		{ TILE_FOREST, TILEID_FOREST_02, L"Resource\\Tiles\\Forest", L"forest_03.png" },
		{ TILE_FOREST, TILEID_FOREST_03, L"Resource\\Tiles\\Forest", L"forest_04.png" }
	};

	static constexpr size_t TileResourceCount = sizeof(TileResourceTable) / sizeof(TileDefEntry);

	// 오브젝트 리소스 정적 테이블
	static constexpr ObjectDefEntry ObjectResourceTable[] = {
		{ GOID_PLAYER_WILSON, L"Resource\\Objects\\Player\\Wilson", L"Wilson_Image.png" },
		{ GOID_PLAYER_WILLOW, L"Resource\\Objects\\Player\\Willow", L"" },
		{ GOID_PLAYER_WOLFGANG, L"Resource\\Objects\\Player\\Wolfgang", L"" },
		{ GOID_NORMAL_TREE_SHORT, L"Resource\\Objects\\Tree1\\Short", L"evergreen_evergreen_short_idle_short_01.png" },
		{ GOID_NORMAL_TREE_NORMAL, L"Resource\\Objects\\Tree1\\Normal", L"evergreen_evergreen_short_idle_normal_01.png" },
		{ GOID_NORMAL_TREE_TALL, L"Resource\\Objects\\Tree1\\Tall", L"evergreen_evergreen_short_idle_tall_01.png" },
		{ GOID_NORMAL_ROCK, L"Resource\\Objects\\Rock\\Rock_Normal", L"rock01-0.png" },
		{ GOID_GOLD_ROCK, L"Resource\\Objects\\Rock\\Rock_Gold", L"rock02-0.png" },
		{ GOID_NORMAL_GRASS, L"Resource\\Objects\\Grass", L"grass.png" },
		{ GOID_NORMAL_SAPLING, L"Resource\\Objects\\Twign", L"sapling.png" },
		{ GOID_BERRY_TREE, L"Resource\\Objects\\Bush", L"BerryBush.png" },
		{ GOID_ITEM_CUT_NORMAL_GRASS, L"Resource\\Objects\\ingredient", L"cutgrass01-0.png" },
		{ GOID_ITEM_NORMAL_ROCK, L"Resource\\Objects\\ingredient", L"rocks01-0.png" },
		{ GOID_ITEM_NORMAL_TWIGS, L"Resource\\Objects\\ingredient", L"twigs01-0.png" },
		{ GOID_ITEM_NORMAL_TREE_LOG, L"Resource\\Objects\\ingredient", L"Tree1_log.png" },
		{ GOID_ITEM_GOLD_ROCK, L"Resource\\Objects\\ingredient", L"Gold_Item.png" },
		{ GOID_ITEM_ROPE, L"Resource\\Objects\\ingredient", L"rope01-0.png" },
		{ GOID_ITEM_CUT_NORMAL_STONE, L"Resource\\Objects\\ingredient", L"cutstone01-0.png" },
		{ GOID_ITEM_MEAT, L"Resource\\Objects\\food", L"meat-0.png" },
		{ GOID_ITEM_BERRY, L"Resource\\Objects\\food", L"Berry.png" },
		{ GOID_ITEM_WOOD_2, L"Resource\\Objects\\ingredient", L"Wood_2.png" },
		{ GOID_ITEM_SMALL_MEAT, L"Resource\\Objects\\food", L"meat_small01-0.png" },
		{ GOID_ITEM_MONSTER_MEAT, L"Resource\\Objects\\food", L"Monster_Meat.png" },
		{ GOID_ITEM_COOKED_MONSTER_MEAT, L"Resource\\Objects\\food", L"Cooked_Monster_Meat.png" },
		{ GOID_ITEM_COOKED_SMALL_MEAT, L"Resource\\Objects\\food", L"meat_small01-1.png" },
		{ GOID_ITEM_COOKED_MEAT, L"Resource\\Objects\\food", L"meat_01-1.png" },
		{ GOID_TOOL_GOLDEN_SCYTHE, L"Resource\\Objects\\Tools", L"Golden_Scythe_02.png" },
		{ GOID_TOOL_HAM_BAT, L"Resource\\Objects\\Tools", L"hamBat_01.png" },
		{ GOID_TOOL_PICKAXE, L"Resource\\Objects\\Tools", L"pickaxe-0.png" },
		{ GOID_TOOL_RED_AXE, L"Resource\\Objects\\Tools", L"Red_Axe_02.png" },
		{ GOID_TOOL_SPEAR, L"Resource\\Objects\\Tools", L"spear_03.png" },
		{ GOID_TOOL_SWAP_AXE, L"Resource\\Objects\\Tools", L"swap_axe-0.png" },
		{ GOID_TOOL_SWAP_SPEAR, L"Resource\\Objects\\Tools", L"swap_spear_wathgrithr_lightning-5.png" },
		{ GOID_TOOL_TORCH, L"Resource\\Objects\\Tools", L"torch.png" },
		{ GOID_TOOL_HALBERD, L"Resource\\Objects\\Tools", L"halberd.png" },
		{ GOID_TOOL_HAMMER, L"Resource\\Objects\\Tools", L"hammer.png" },
		{ GOID_MONSTER_SPIDER, L"Resource\\Objects\\Monster\\Spider\\Normal_Spider", L"Spider_spider_idle_01.png" },
		{ GOID_MONSTER_WARRIOR_SPIDER, L"Resource\\Objects\\Monster\\Spider\\Warrior_Spider", L"Warrior_spider_idle_01.png" },
		{ GOID_MONSTER_PIG, L"Resource\\Objects\\Monster\\Pig", L"pig_Image.png" },
		{ GOID_MONSTER_HOUNDDOG, L"Resource\\Objects\\Monster\\Hound\\Normal_Hound", L"Hound_hound_Image.png" },
		{ GOID_MONSTER_QUEEN_SPIDER, L"Resource\\Objects\\Monster\\Spider\\Queen", L"Queen_spider_queen_Image.png" },
		{ GOID_MONSTER_REDHOUNDDOG, L"Resource\\Objects\\Monster\\Hound\\Red_Hound", L"RedHound_hound_Image.png" },
		{ GOID_MONSTER_ICEHOUNDDOG, L"Resource\\Objects\\Monster\\Hound\\Ice_Hound", L"IceHound_hound_Image.png" },
		{ GOID_BUILDING_SPIDER_SACEGG, L"Resource\\Objects\\Building\\Egg", L"Egg_spider_cocoon_small_Image.png" },
		{ GOID_BUILDING_SPIDER_SMALLEGG, L"Resource\\Objects\\Building\\Egg", L"Egg_spider_cocoon_small_Image.png" },
		{ GOID_BUILDING_SPIDER_NORMALEGG, L"Resource\\Objects\\Building\\Egg", L"Egg_spider_cocoon_medium_Image.png" },
		{ GOID_BUILDING_SPIDER_TALLEGG, L"Resource\\Objects\\Building\\Egg", L"Egg_spider_cocoon_large_Image.png" },
		{ GOID_BUILDING_PIGHOUSE, L"Resource\\Objects\\Building\\House", L"pig_house.png" }
	};

	static constexpr size_t ObjectResourceCount = sizeof(ObjectResourceTable) / sizeof(ObjectDefEntry);

	// 타일 리소스 정의 구조체 (런타임 사용, std::wstring 사용)
	struct TileResourceDef {
		TileType type;
		TileID id;
		std::wstring baseDir;
		std::wstring imageName;

		TileResourceDef()
			: type(TILE_NONE), id(TILEID_NONE)
		{}

		TileResourceDef(TileType _type, TileID _id, const std::wstring& _baseDir, const std::wstring& _imageName)
			: type(_type), id(_id), baseDir(_baseDir), imageName(_imageName)
		{}
	};

	// 오브젝트 리소스 정의 구조체
	struct ObjectResourceDef {
		GameObjectID id;
		std::wstring baseDir;
		std::wstring imageName;
		float pivotX;
		float pivotY;
		
		float x = 0, y = 0;
		
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
			: id(GOID_NONE), pivotX(0.5f), pivotY(1.0f), x(0), y(0)
		{}

		ObjectResourceDef(GameObjectID id_val, float x_val, float y_val,
			const std::wstring& baseDir_val, const std::wstring& imageName_val, float pivotX_val, float pivotY_val,
			bool hasCollider_val = false, ColliderType colliderType_val = COLLIDER_BOX,
			int colliderOffsetX_val = 0, int colliderOffsetY_val = 0,
			int colliderWidth_val = 0, int colliderHeight_val = 0,
			float colliderCenterX_val = 0.0f, float colliderCenterY_val = 0.0f, float colliderRadius_val = 0.0f)
			: id(id_val), baseDir(baseDir_val), imageName(imageName_val),
			pivotX(pivotX_val), pivotY(pivotY_val), x(x_val), y(y_val),
			hasCollider(hasCollider_val), colliderType(colliderType_val),
			colliderOffsetX(colliderOffsetX_val), colliderOffsetY(colliderOffsetY_val),
			colliderWidth(colliderWidth_val), colliderHeight(colliderHeight_val),
			colliderCenterX(colliderCenterX_val), colliderCenterY(colliderCenterY_val), colliderRadius(colliderRadius_val)
		{}
	};
}

// ====================== 도구 스탯 데이터 테이블 =======================

namespace ToolDataUtils
{
	// 도구 스탯 엔트리 구조체
	struct ToolStatsEntry {
		GameObjectID toolID;
		int damage;
		float attackRange;
	};

	// 도구 스탯 정적 테이블
	static constexpr ToolStatsEntry ToolStatsTable[] = {
		{ GOID_TOOL_GOLDEN_SCYTHE,  25,    120.0f },
		{ GOID_TOOL_HAM_BAT,        40,    100.0f },
		{ GOID_TOOL_PICKAXE,        15,    80.0f  },
		{ GOID_TOOL_RED_AXE,        20,    100.0f },
		{ GOID_TOOL_SPEAR,          30,    150.0f },
		{ GOID_TOOL_SWAP_AXE,       25,    100.0f },
		{ GOID_TOOL_SWAP_SPEAR,     45,    160.0f },
		{ GOID_TOOL_TORCH,          10,    90.0f  },
		{ GOID_TOOL_HALBERD,        50,    140.0f },
		{ GOID_TOOL_HAMMER,         15,    85.0f  }
	};

	static constexpr size_t ToolStatsCount = sizeof(ToolStatsTable) / sizeof(ToolStatsEntry);
}

// ====================== 아이템 디스플레이 이름 테이블 =======================

namespace ItemDisplayUtils
{
	// 아이템 디스플레이 이름 엔트리 구조체
	struct ItemDisplayEntry {
		GameObjectID itemID;
		const wchar_t* displayName;
	};

	// 아이템 디스플레이 이름 정적 테이블
	static constexpr ItemDisplayEntry ItemDisplayTable[] = {
		{ GOID_ITEM_NORMAL_TWIGS,        L"나뭇가지" },
		{ GOID_ITEM_NORMAL_TREE_LOG,     L"통나무" },
		{ GOID_ITEM_NORMAL_ROCK,         L"돌" },
		{ GOID_ITEM_CUT_NORMAL_GRASS,    L"풀" },
		{ GOID_ITEM_GOLD_ROCK,           L"금" },
		{ GOID_ITEM_CUT_NORMAL_STONE,    L"석재" },
		{ GOID_ITEM_ROPE,                L"밧줄" },
		{ GOID_ITEM_WOOD_2,              L"나무판자" },
		{ GOID_ITEM_MEAT,                L"고기" },
		{ GOID_ITEM_SMALL_MEAT,          L"작은 고기" },
		{ GOID_ITEM_MONSTER_MEAT,        L"몬스터 고기" },
		{ GOID_ITEM_COOKED_MEAT,         L"구운 고기" },
		{ GOID_ITEM_COOKED_SMALL_MEAT,   L"구운 작은 고기" },
		{ GOID_ITEM_COOKED_MONSTER_MEAT, L"구운 몬스터 고기" },
		{ GOID_TOOL_GOLDEN_SCYTHE,       L"황금 낫" },
		{ GOID_TOOL_HAM_BAT,             L"햄 방망이" },
		{ GOID_TOOL_PICKAXE,             L"곡괭이" },
		{ GOID_TOOL_RED_AXE,             L"빨간 도끼" },
		{ GOID_TOOL_SPEAR,               L"창" },
		{ GOID_TOOL_SWAP_AXE,            L"도끼" },
		{ GOID_TOOL_SWAP_SPEAR,          L"번개 창" },
		{ GOID_TOOL_TORCH,               L"횃불" },
		{ GOID_TOOL_HALBERD,             L"미늘창" },
		{ GOID_TOOL_HAMMER,              L"망치" }
	};

	static constexpr size_t ItemDisplayCount = sizeof(ItemDisplayTable) / sizeof(ItemDisplayEntry);
}
 
// ====================== 팔레트 아이템 구조체 =======================

struct PaletteItem 
{
	UINT resourceId;
	ItemCategory category;
	RECT displayRect;
	Gdiplus::Bitmap* hBitmap;
	Gdiplus::RectF iconSourceRect;
};

// ====================== 렌더 명령 구조체 =======================

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

	const std::wstring* textPtr = nullptr;
	Gdiplus::Font* pFont = nullptr;
	Gdiplus::Brush* pBrush = nullptr;
	Gdiplus::StringFormat* pStringFormat = nullptr;

	Gdiplus::Color color = Gdiplus::Color(0, 0, 0, 0);
	float thickness = 0.0f;
	Gdiplus::Color tintColor = Gdiplus::Color(255, 255, 255, 255);
	bool hasTint = false;
	bool preFlipped = false;
};

// ====================== 플레이어 스폰 데이터 구조체 =======================

struct PlayerSpawnData {
	float x, y;

	PlayerSpawnData() : x(-1), y(-1) {}
	PlayerSpawnData(float _x, float _y) : x(_x), y(_y) {}
};

// ====================== 맵 데이터 구조체 =======================

struct MapData {
	std::wstring mapName;
	std::wstring mapFilePath;

	int mapWidth;
	int mapHeight;
	PlayerSpawnData playerSpawn;

	ResourcePathUtils::TileResourceDef tiles[MAP_WIDTH][MAP_HEIGHT];
	std::vector<ResourcePathUtils::ObjectResourceDef> gameObjects;
	bool walkableAreas[MAP_WIDTH][MAP_HEIGHT];

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

// ====================== 애니메이션 프레임 구조체 =======================

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
