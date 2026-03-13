#pragma once
#include "../../../Header/SingleTon.h"
#include <string>
#include <vector>
#include <map>

// 전방 선언
enum SceneType;
enum GameObjectID : UINT;

// 플레이어 상태 스냅샷 (씬 전환 시 저장/복원용)
struct PlayerStateSnapshot
{
	int hp;
	int equippedSlotIndex;
	std::vector<std::pair<GameObjectID, UINT>> inventoryItems;

	PlayerStateSnapshot() : hp(100), equippedSlotIndex(-1) {}
};

// ====================== 게임 진행도 관련 구조체 =======================

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

// ====================== GameProgressManager 클래스 =======================

class GameProgressManager : public CSingleTon<GameProgressManager>
{
	friend class CSingleTon<GameProgressManager>;
private:
	GameProgressManager();
	~GameProgressManager();

public:
	// 생명주기 메서드
	void Init();
	void Update(float deltaTime);
	void LateUpdate();
	void Render();
	void Release();

	// 씬 클리어 관련
	bool IsSceneCleared(SceneType sceneType) const;
	void ClearScene(SceneType sceneType);
	const SceneClearCondition* GetSceneClearCondition(SceneType sceneType) const;

	// 캐릭터 해금 관련
	bool IsCharacterUnlocked(GameObjectID characterID) const;
	void UpdateCharacterUnlocks();

	// 이벤트 기반 체크 (핵심)
	void OnMonsterKilled(GameObjectID monsterID, SceneType currentScene);
	void OnItemCollected(GameObjectID itemID, int count, SceneType currentScene);
	bool CheckCurrentSceneClearCondition(SceneType sceneType);

	// 저장/로드 (텍스트 파일 형식)
	void SaveToFile(const std::wstring& filePath);
	void LoadFromFile(const std::wstring& filePath);

	// GameProgress 접근자
	const GameProgress& GetGameProgress() const { return m_gameProgress; }

	// 현재 씬 진행도 초기화 (씬 전환 시 호출)
	void ResetCurrentSceneProgress();

	// 플레이어 상태 저장/복원 관련
	void SavePlayerState(const PlayerStateSnapshot& snapshot);
	const PlayerStateSnapshot& GetPlayerState() const { return m_playerSnapshot; }
	bool HasSavedPlayerState() const { return m_hasSavedPlayerState; }
	void ClearSavedPlayerState() { m_hasSavedPlayerState = false; }

private:
	GameProgress m_gameProgress;  // 게임 진행도 데이터

	// 플레이어 상태 저장
	PlayerStateSnapshot m_playerSnapshot;
	bool m_hasSavedPlayerState = false;

	// 현재 씬 진행도 추적 (클리어 조건 체크용)
	std::map<GameObjectID, int> m_currentSceneKillCounts;
	std::map<GameObjectID, int> m_currentSceneItemCounts;

	std::wstring m_saveFilePath;  // 저장 파일 경로

	// 내부 헬퍼 함수
	std::wstring GetCurrentDateString() const;
	void ParseSceneLine(const std::wstring& line);
	void ParseCharacterLine(const std::wstring& line);
	void ParseKillCountLine(const std::wstring& line);
	void ParseItemCountLine(const std::wstring& line);
};
