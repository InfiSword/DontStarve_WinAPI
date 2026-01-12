#pragma once

// Ÿ�� Ÿ�� ����
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

// ���� Ÿ�� ����
enum BossType
{
	BOSS_NONE = 0,
	BOSS_HOUND,           // Hound ����
	BOSS_SPIDER_QUEEN,    // Spider Queen ����
	BOSS_COUNT
};

// ĳ���� Ÿ�� ����
enum CharacterType
{
	CHARACTER_WILSON = 0,     // �⺻ ĳ���� (�׻� �رݵ�)
	CHARACTER_WILLOW = 1,     // Hound ���� Ŭ���� �� �ر�
	CHARACTER_WOLFGANG = 2,   // Spider Queen ���� Ŭ���� �� �ر�
	CHARACTER_COUNT
};

// ���� ���� �ܰ� �� UI ���� ��� �����ϴ� ���� SceneType ����
// (Ÿ��Ʋ, ĳ���� ����, ����, �� ���� �� ��)
enum SceneType
{
	SCENE_NONE = 0,
	SCENE_TITLE,               // Ÿ��Ʋ ��
	SCENE_CHARACTER_SELECT,    // ĳ���� ���� ��
	SCENE_GAME_FARMING_AREA,        // 1��°: �Ĺ� ������ (���� �� ����)
	SCENE_GAME_HOUND_FOREST,        // 2��°: �Ͽ�� �� (���� �� ����)
	SCENE_GAME_SPIDER_QUEEN_HOUSE,  // 3��°: �Ź̿����� �� (���� �� ����)
	SCENE_COUNT
};

// ���� �� �ε� ���� ����
enum class LoadingState
{
	NOT_STARTED,
	LOADING_MAP,
	CREATING_OBJECTS,
	SPAWNING_PLAYER,
	COMPLETED,
	FAILED
};

// ������Ʈ Ÿ�� ����
enum GameObjectType {
	GOBJ_NONE = 0,

	GOBJ_PLAYER,
	// ���, ��� �����۵�
	GOBJ_ITEM,
	// ȯ�� ������Ʈ (����, Ǯ, ����)
	GOBJ_NATURAL_ENVIR,
	// ���� 
	GOBJ_MONSTER,
	// ����
	GOBJ_BUILDING,
	// UI ��� (��ư, �̹��� ��)
	GOBJ_UI,

	GOBJ_COUNT      // ������Ʈ ����
};

enum GameObjectID : UINT {
	GOID_NONE = 0,

	// ȯ�� ������Ʈ
	GOID_NORMAL_GRASS = 1,
	GOID_NORMAL_TREE_SHORT = 2,
	GOID_NORMAL_TREE_NORMAL = 3,
	GOID_NORMAL_TREE_TALL = 4,
	GOID_NORMAL_ROCK = 5,
	GOID_GOLD_ROCK = 6,
	GOID_NORMAL_SAPLING = 7,
	GOID_BERRY_TREE = 8,

	// ����
	GOID_MONSTER_PIG = 101,
	GOID_MONSTER_SPIDER = 102,
	GOID_MONSTER_WARRIOR_SPIDER = 103,
	GOID_MONSTER_QUEEN_SPIDER = 104,
	GOID_MONSTER_HOUNDDOG = 105,
	GOID_MONSTER_REDHOUNDDOG = 106,
	GOID_MONSTER_ICEHOUNDDOG = 107,

	// ���๰
	GOID_BUILDING_PIGHOUSE = 201,
	GOID_BUILDING_SPIDER_SMALLEGG = 202,
	GOID_BUILDING_SPIDER_NORMALEGG = 203,
	GOID_BUILDING_SPIDER_TALLEGG = 204,

	// ��� (������)
	GOID_ITEM_CUT_NORMAL_GRASS = 301,
	GOID_ITEM_NORMAL_TREE_LOG = 302,
	GOID_ITEM_NORMAL_TWIGS = 303,
	GOID_ITEM_NORMAL_ROCK = 304,
	GOID_ITEM_GOLD_ROCK = 305,
	GOID_ITEM_ROPE = 306,
	GOID_ITEM_CUT_NORMAL_STONE = 307,
	GOID_ITEM_MEAT = 308,
	GOID_ITEM_BERRY = 309,

	// ���� (������)
	GOID_ITEM_AXE = 401,
	GOID_ITEM_PICKAXE = 402,

	// �÷��̾� ĳ����
	GOID_PLAYER_WILSON = 1001,
	GOID_PLAYER_WILLOW = 1002,
	GOID_PLAYER_WOLFGANG = 1003,

	// UI ����
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

// �ȷ�Ʈ ������ ī�װ��� ���� ( ������ ���� )
enum ItemCategory {
	CATEGORY_NONE = -1,
	CATEGORY_TILE,
	CATEGORY_OBJECT
};


// GameObject ���� ������
enum Direction {
	DIR_NONE = 0,
	DIR_UP,
	DIR_DOWN,
	DIR_LEFT,
	DIR_RIGHT,
	DIR_COUNT
};

enum PlayerState {
	IDLE,               // ��� ���� (�������� ����)
	WALK,               // �ȱ� ���� (�̵� ��)
	MOVING_TO_TARGET,   // Ŭ���� �������� �̵� ��
	PICKUP,             // ������Ʈ�� ��ȣ�ۿ� 
	CHOP,
	ATTACK,
	HIT,
	COUNT,
};

// Monster ���� ����
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

// Tree ���� ����  
enum TreeState {
	TREE_IDLE = 0,    // �Ϲ� ���� (��鸲)
	TREE_CHOP,        // �������� ��
	TREE_FALL,        // �������� ��
	TREE_FALLEN,      // ������ ������ ����
	TREE_STATE_COUNT
};

// Rock ���� ���� (�ı� �ܰ�)
enum RockState {
	ROCK_INTACT = 0,   // ������ ���� (level1)
	ROCK_CRACKED,      // ���� �� ���� (level2)  
	ROCK_BROKEN,       // ���� �μ��� ���� (level3)
	ROCK_DESTROYED,    // ������ �ı��� ����
	ROCK_STATE_COUNT
};

// Grass ���� ����
enum GrassState {
	GRASS_IDLE = 0,    // �Ϲ� ����
	GRASS_PICKED,      // ä���� ����
	GRASS_REGROWING,   // �缺�� ��
	GRASS_STATE_COUNT
};

// Building ���� ���� (�ð� ���)
enum BuildingState {
	BUILDING_NOON = 0,
	BUILDING_NIGHT,
	BUILDING_DAMAGED,
	BUILDING_DESTROYED,
	BUILDING_STATE_COUNT
};

enum RenderLayer {
	LAYER_NONE = -1,
	LAYER_TILE_BACKGROUND = 0,   // Ÿ�� (���� �Ʒ�)
	LAYER_WORLD_TILE,
	LAYER_WORLD_OBJECT,          // ���� ������Ʈ (��� ���� ������Ʈ)
	LAYER_UI_BACKGROUND,         // UI ���
	LAYER_UI_FOREGROUND,         // UI ������, �ؽ�Ʈ
	LAYER_DEBUG_OVERLAY,         // ����� ���� (���� ��)

	LAYER_COUNT                  // ���̾� ����
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

// 렌더링 명령 종류
enum DrawCommandType {
	DRAW_COMMAND_IMAGE,
	DRAW_COMMAND_TEXT,
	DRAW_COMMAND_RECTANGLE,
	DRAW_COMMAND_FILL_RECTANGLE
};

// �ݶ��̴� Ÿ�� ����
enum ColliderType {
	COLLIDER_BOX = 0,      // �簢�� �ݶ��̴�
	COLLIDER_CIRCLE,       // ���� �ݶ��̴�
	COLLIDER_COUNT
};