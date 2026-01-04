#pragma once

// 타일 타입 정의
enum TileType
{
	TILE_NONE = 0,
	TILE_DIRT,
	TILE_GRASS,
	TILE_FOREST,
	TILE_COUNT
};

enum TileID
{
	TILEID_NONE = 0,

	TILEID_DIRT_00 = 1,
	TILEID_DIRT_01 = 2,
	TILEID_DIRT_02 = 3,
	TILEID_DIRT_03 = 4,

	TILEID_GRASS_00 = 101,
	TILEID_GRASS_01 = 102,
	TILEID_GRASS_02 = 103,
	TILEID_GRASS_03 = 104,

	TILEID_FOREST_00 = 201,
	TILEID_FOREST_01 = 202,
	TILEID_FOREST_02 = 203,
	TILEID_FOREST_03 = 204,

};

// 보스 타입 정의
enum BossType
{
	BOSS_NONE = 0,
	BOSS_HOUND,           // Hound 보스
	BOSS_SPIDER_QUEEN,    // Spider Queen 보스
	BOSS_COUNT
};

// 캐릭터 타입 정의
enum CharacterType
{
	CHARACTER_WILSON = 0,     // 기본 캐릭터 (항상 해금됨)
	CHARACTER_WILLOW = 1,     // Hound 보스 클리어 시 해금
	CHARACTER_WOLFGANG = 2,   // Spider Queen 보스 클리어 시 해금
	CHARACTER_COUNT
};

// 게임 진행 단계 및 UI 씬을 모두 포함하는 통합 SceneType 정의
// (타이틀, 캐릭터 선택, 게임, 각 게임 씬 등)
enum SceneType
{
	SCENE_NONE = 0,
	SCENE_TITLE,               // 타이틀 씬
	SCENE_CHARACTER_SELECT,    // 캐릭터 선택 씬
	SCENE_GAME_FARMING_AREA,        // 1번째: 파밍 에리어 (게임 내 구역)
	SCENE_GAME_HOUND_FOREST,        // 2번째: 하운드 숲 (게임 내 구역)
	SCENE_GAME_SPIDER_QUEEN_HOUSE,  // 3번째: 거미여왕의 집 (게임 내 구역)
	SCENE_COUNT
};

// 게임 씬 로딩 상태 관리
enum class LoadingState
{
	NOT_STARTED,
	LOADING_MAP,
	CREATING_OBJECTS,
	SPAWNING_PLAYER,
	COMPLETED,
	FAILED
};

// 오브젝트 타입 정의
enum GameObjectType {
	GOBJ_NONE = 0,

	GOBJ_PLAYER,
	// 재료, 장비 아이템들
	GOBJ_ITEM,
	// 환경 오브젝트 (나무, 풀, 바위)
	GOBJ_NATURAL_ENVIR,
	// 몬스터 
	GOBJ_MONSTER,
	// 빌딩
	GOBJ_BUILDING,
	// UI 요소 (버튼, 이미지 등)
	GOBJ_UI,

	GOBJ_COUNT      // 오브젝트 개수
};

enum GameObjectID : UINT {
	GOID_NONE = 0,

	// 환경 오브젝트
	GOID_NORMAL_GRASS = 1,
	GOID_NORMAL_TREE_SHORT = 2,
	GOID_NORMAL_TREE_NORMAL = 3,
	GOID_NORMAL_TREE_TALL = 4,
	GOID_NORMAL_ROCK = 5,
	GOID_GOLD_ROCK = 6,
	GOID_NORMAL_SAPLING = 7,
	GOID_BERRY_TREE = 8,

	// 몬스터
	GOID_MONSTER_PIG = 101,
	GOID_MONSTER_SPIDER = 102,
	GOID_MONSTER_WARRIOR_SPIDER = 103,
	GOID_MONSTER_QUEEN_SPIDER = 104,
	GOID_MONSTER_HOUNDDOG = 105,
	GOID_MONSTER_REDHOUNDDOG = 106,
	GOID_MONSTER_ICEHOUNDDOG = 107,

	// 건축물
	GOID_BUILDING_PIGHOUSE = 201,
	GOID_BUILDING_SPIDER_SMALLEGG = 202,
	GOID_BUILDING_SPIDER_NORMALEGG = 203,
	GOID_BUILDING_SPIDER_TALLEGG = 204,

	// 재료 (아이템)
	GOID_ITEM_CUT_NORMAL_GRASS = 301,
	GOID_ITEM_NORMAL_TREE_LOG = 302,
	GOID_ITEM_NORMAL_TWIGS = 303,
	GOID_ITEM_NORMAL_ROCK = 304,
	GOID_ITEM_GOLD_ROCK = 305,
	GOID_ITEM_ROPE = 306,
	GOID_ITEM_CUT_NORMAL_STONE = 307,
	GOID_ITEM_MEAT = 308,
	GOID_ITEM_BERRY = 309,

	// 도구 (아이템)
	GOID_ITEM_AXE = 401,
	GOID_ITEM_PICKAXE = 402,

	// 플레이어 캐릭터
	GOID_PLAYER_WILSON = 1001,
	GOID_PLAYER_WILLOW = 1002,
	GOID_PLAYER_WOLFGANG = 1003,

	// UI 관련
	GOID_MAIN_BG = 3001,
	GOID_GAME_LOGO = 3002,
	GOID_BUTTON1 = 3003,
	GOID_ENDBUTTON1 = 3004,
	GOID_SELECT_BUTTON = 3005,
	GOID_CANCEL_SELECTION = 3006,
	GOID_BACK_BUTTON = 3007,
	GOID_PLAYER_PORTRAIT = 3008,
	GOID_PLAYER_INFO = 3009,
};

// 팔레트 아이템 카테고리 정의 ( 에디터 전용 )
enum ItemCategory {
	CATEGORY_NONE = -1,
	CATEGORY_TILE,
	CATEGORY_OBJECT
};


// GameObject 방향 열거형
enum Direction {
	DIR_NONE = 0,
	DIR_UP,
	DIR_DOWN,
	DIR_LEFT,
	DIR_RIGHT,
	DIR_COUNT
};

enum PlayerState {
	IDLE,               // 대기 상태 (움직이지 않음)
	WALK,               // 걷기 상태 (이동 중)
	MOVING_TO_TARGET,   // 클릭된 지점으로 이동 중
	PICKUP,             // 오브젝트와 상호작용 
	CHOP,
	ATTACK,
	HIT,
	COUNT,
};

// Monster 전용 상태
enum MonsterState
{
	MONSTER_IDLE = 0,
	MONSTER_WALK,
	MONSTER_RUN,
	MONSTER_ATTACK,
	MONSTER_HIT,
	MONSTER_DEATH,
	MONSTER_PATROL,
	MONSTER_CHASE,
	MONSTER_STATE_COUNT
};

// Tree 전용 상태  
enum TreeState {
	TREE_IDLE = 0,    // 일반 상태 (흔들림)
	TREE_CHOP,        // 베어지는 중
	TREE_FALL,        // 쓰러지는 중
	TREE_FALLEN,      // 완전히 쓰러진 상태
	TREE_STATE_COUNT
};

// Rock 전용 상태 (파괴 단계)
enum RockState {
	ROCK_INTACT = 0,   // 온전한 상태 (level1)
	ROCK_CRACKED,      // 금이 간 상태 (level2)  
	ROCK_BROKEN,       // 거의 부서진 상태 (level3)
	ROCK_DESTROYED,    // 완전히 파괴된 상태
	ROCK_STATE_COUNT
};

// Grass 전용 상태
enum GrassState {
	GRASS_IDLE = 0,    // 일반 상태
	GRASS_PICKED,      // 채집된 상태
	GRASS_REGROWING,   // 재성장 중
	GRASS_STATE_COUNT
};

// Building 전용 상태 (시간 기반)
enum BuildingState {
	BUILDING_NOON = 0,
	BUILDING_NIGHT,
	BUILDING_DAMAGED,
	BUILDING_DESTROYED,
	BUILDING_STATE_COUNT
};

enum RenderLayer {
	LAYER_NONE = -1,
	LAYER_TILE_BACKGROUND = 0,   // 타일 (가장 아래)
	LAYER_WORLD_TILE,
	LAYER_WORLD_OBJECT,          // 월드 오브젝트 (모든 게임 오브젝트)
	LAYER_UI_BACKGROUND,         // UI 배경
	LAYER_UI_FOREGROUND,         // UI 아이콘, 텍스트
	LAYER_DEBUG_OVERLAY,         // 디버깅 정보 (가장 위)

	LAYER_COUNT                  // 레이어 개수
};

enum class ButtonState
{
	NORMAL,
	HOVER,
	CLICKED,
	DISABLED
};

enum class ToolState 
{
	TOOL_NORMAL,
	TOOL_CRACKED,
	TOOL_BROKEN,
};

enum Time {
	NOON = 0,
	NIGHT,
	TIME_COUNT
};

// 콜라이더 타입 정의
enum ColliderType {
	COLLIDER_BOX = 0,      // 사각형 콜라이더
	COLLIDER_CIRCLE,       // 원형 콜라이더
	COLLIDER_COUNT
};