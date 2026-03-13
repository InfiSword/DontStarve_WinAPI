#include "99_Default/pch.h"
#include "GameProgressManager.h"
#include <iomanip>
#include <ctime>

GameProgressManager::GameProgressManager()
	: m_saveFilePath(L"GameData/game_progress.txt")
{
}

GameProgressManager::~GameProgressManager()
{
	Release();
}

void GameProgressManager::Init()
{
	// 저장 파일 로드 (파일이 없으면 기본 초기화)
	LoadFromFile(m_saveFilePath);
	
	// 현재 씬 진행도 초기화
	ResetCurrentSceneProgress();
	
	OutputDebugStringW(L"GameProgressManager: 초기화 완료\n");
}

void GameProgressManager::Update(float deltaTime)
{
	// 진행도 관련 업데이트가 필요한 경우 여기에 추가
}

void GameProgressManager::LateUpdate()
{
	// LateUpdate가 필요한 경우 여기에 추가
}

void GameProgressManager::Render()
{
	// 진행도 UI 렌더링이 필요한 경우 여기에 추가
}

void GameProgressManager::Release()
{
	// 게임 종료 시 진행도 저장
	SaveToFile(m_saveFilePath);
	
	// 메모리 정리
	m_currentSceneKillCounts.clear();
	m_currentSceneItemCounts.clear();
	
	OutputDebugStringW(L"GameProgressManager: 해제 완료\n");
}

// ====================== 씬 클리어 관련 =======================

bool GameProgressManager::IsSceneCleared(SceneType sceneType) const
{
	return m_gameProgress.IsSceneCleared(sceneType);
}

void GameProgressManager::ClearScene(SceneType sceneType)
{
	// 씬 클리어 처리
	m_gameProgress.ClearScene(sceneType);
	
	// 클리어 날짜 설정
	for (auto& sceneInfo : m_gameProgress.sceneClearInfos)
	{
		if (sceneInfo.sceneType == sceneType && sceneInfo.clearDate.empty())
		{
			sceneInfo.clearDate = GetCurrentDateString();
			break;
		}
	}
	
	// 저장
	SaveToFile(m_saveFilePath);
	
	std::wstring msg = L"GameProgressManager: 씬 클리어 - SceneType: " + std::to_wstring((int)sceneType) + L"\n";
	OutputDebugStringW(msg.c_str());
}

const SceneClearCondition* GameProgressManager::GetSceneClearCondition(SceneType sceneType) const
{
	return m_gameProgress.GetSceneClearCondition(sceneType);
}

// ====================== 캐릭터 해금 관련 =======================

bool GameProgressManager::IsCharacterUnlocked(GameObjectID characterID) const
{
	return m_gameProgress.IsCharacterUnlocked(characterID);
}

void GameProgressManager::UpdateCharacterUnlocks()
{
	m_gameProgress.UpdateCharacterUnlocks();
	
	// 저장
	SaveToFile(m_saveFilePath);
	
	OutputDebugStringW(L"GameProgressManager: 캐릭터 해금 업데이트 완료\n");
}

// ====================== 이벤트 기반 체크 =======================

void GameProgressManager::OnMonsterKilled(GameObjectID monsterID, SceneType currentScene)
{
	// 현재 씬의 처치 카운트 증가
	m_currentSceneKillCounts[monsterID]++;
	
	std::wstring msg = L"GameProgressManager: 몬스터 처치 - ID: " + std::to_wstring(monsterID) + 
		L", 카운트: " + std::to_wstring(m_currentSceneKillCounts[monsterID]) + L"\n";
	OutputDebugStringW(msg.c_str());
	
	// 클리어 조건 체크
	if (CheckCurrentSceneClearCondition(currentScene))
	{
		OutputDebugStringW(L"GameProgressManager: 씬 클리어 조건 달성!\n");
		ClearScene(currentScene);
	}
}

void GameProgressManager::OnItemCollected(GameObjectID itemID, int count, SceneType currentScene)
{
	// 현재 씬의 아이템 카운트 증가
	m_currentSceneItemCounts[itemID] += count;
	
	std::wstring msg = L"GameProgressManager: 아이템 획득 - ID: " + std::to_wstring(itemID) + 
		L", 카운트: " + std::to_wstring(m_currentSceneItemCounts[itemID]) + L"\n";
	OutputDebugStringW(msg.c_str());
	
	// 클리어 조건 체크
	if (CheckCurrentSceneClearCondition(currentScene))
	{
		OutputDebugStringW(L"GameProgressManager: 씬 클리어 조건 달성!\n");
		ClearScene(currentScene);
	}
}

bool GameProgressManager::CheckCurrentSceneClearCondition(SceneType sceneType)
{
	// 이미 클리어된 씬이면 false 반환
	if (IsSceneCleared(sceneType))
		return false;
	
	// 씬의 클리어 조건 가져오기
	const SceneClearCondition* condition = GetSceneClearCondition(sceneType);
	if (!condition)
		return false;
	
	// 필수 처치 대상 확인
	for (const auto& requiredKill : condition->requiredKills)
	{
		auto it = m_currentSceneKillCounts.find(requiredKill);
		if (it == m_currentSceneKillCounts.end() || it->second < 1)
		{
			// 아직 처치하지 않은 대상이 있음
			return false;
		}
	}
	
	// 필수 아이템 확인
	int totalItemCount = 0;
	for (const auto& requiredItem : condition->requiredItems)
	{
		auto it = m_currentSceneItemCounts.find(requiredItem);
		if (it != m_currentSceneItemCounts.end())
		{
			totalItemCount += it->second;
		}
	}
	
	// 필요한 아이템 개수를 모았는지 확인
	if (!condition->requiredItems.empty() && totalItemCount < condition->requiredItemCount)
	{
		return false;
	}
	
	// 모든 조건 충족
	return true;
}

// ====================== 현재 씬 진행도 =======================

void GameProgressManager::ResetCurrentSceneProgress()
{
	m_currentSceneKillCounts.clear();
	m_currentSceneItemCounts.clear();
	
	OutputDebugStringW(L"GameProgressManager: 현재 씬 진행도 초기화\n");
}

void GameProgressManager::SavePlayerState(const PlayerStateSnapshot& snapshot)
{
	m_playerSnapshot = snapshot;
	m_hasSavedPlayerState = true;
	OutputDebugStringW(L"GameProgressManager: 플레이어 상태 저장 완료\n");
}

// ====================== 저장/로드 =======================

void GameProgressManager::SaveToFile(const std::wstring& filePath)
{
	// 저장 디렉터리가 없으면 생성
	size_t lastSlash = filePath.find_last_of(L"\\/");
	if (lastSlash != std::wstring::npos) {
		std::wstring dir = filePath.substr(0, lastSlash);
		if (!dir.empty()) {
			CreateDirectoryW(dir.c_str(), nullptr);
		}
	}

	std::wofstream file(filePath);
	if (!file.is_open())
	{
		OutputDebugStringW(L"GameProgressManager: 파일 저장 실패\n");
		return;
	}
	
	file << L"[GAME_PROGRESS_V1]\n";
	file << L"# Scene Clear Info\n";
	
	// 씬 클리어 정보 저장
	for (const auto& sceneInfo : m_gameProgress.sceneClearInfos)
	{
		file << L"SCENE:" << (int)sceneInfo.sceneType 
			 << L",CLEARED:" << (sceneInfo.isCleared ? 1 : 0)
			 << L",DATE:" << sceneInfo.clearDate << L"\n";
	}
	
	file << L"\n# Character Unlock Info\n";
	
	// 캐릭터 해금 정보 저장
	for (const auto& charInfo : m_gameProgress.characterUnlockInfos)
	{
		file << L"CHARACTER:" << (int)charInfo.characterID
			 << L",UNLOCKED:" << (charInfo.isUnlocked ? 1 : 0) << L"\n";
	}
	
	file << L"\n# Kill Count (현재 씬 진행도 추적용)\n";
	
	// 현재 씬 처치 카운트 저장
	for (const auto& killCount : m_currentSceneKillCounts)
	{
		file << L"KILL_COUNT:" << (int)killCount.first 
			 << L":" << killCount.second << L"\n";
	}
	
	file << L"\n# Item Collection (현재 씬 진행도 추적용)\n";
	
	// 현재 씬 아이템 카운트 저장
	for (const auto& itemCount : m_currentSceneItemCounts)
	{
		file << L"ITEM_COUNT:" << (int)itemCount.first 
			 << L":" << itemCount.second << L"\n";
	}
	
	file.close();
	
	OutputDebugStringW(L"GameProgressManager: 파일 저장 완료\n");
}

void GameProgressManager::LoadFromFile(const std::wstring& filePath)
{
	std::wifstream file(filePath);
	if (!file.is_open())
	{
		OutputDebugStringW(L"GameProgressManager: 파일 로드 실패 (새로 생성됨)\n");
		return;
	}
	
	std::wstring line;
	while (std::getline(file, line))
	{
		// 빈 줄이나 주석은 건너뛰기
		if (line.empty() || line[0] == L'#' || line[0] == L'[')
			continue;
		
		// 라인 파싱
		if (line.find(L"SCENE:") == 0)
		{
			ParseSceneLine(line);
		}
		else if (line.find(L"CHARACTER:") == 0)
		{
			ParseCharacterLine(line);
		}
		else if (line.find(L"KILL_COUNT:") == 0)
		{
			ParseKillCountLine(line);
		}
		else if (line.find(L"ITEM_COUNT:") == 0)
		{
			ParseItemCountLine(line);
		}
	}
	
	file.close();
	
	OutputDebugStringW(L"GameProgressManager: 파일 로드 완료\n");
}

// ====================== 내부 헬퍼 함수 =======================

std::wstring GameProgressManager::GetCurrentDateString() const
{
	std::time_t now = std::time(nullptr);
	std::tm localTime;
	localtime_s(&localTime, &now);
	
	wchar_t buffer[32];
	swprintf_s(buffer, L"%04d-%02d-%02d", 
		localTime.tm_year + 1900, 
		localTime.tm_mon + 1, 
		localTime.tm_mday);
	
	return std::wstring(buffer);
}

void GameProgressManager::ParseSceneLine(const std::wstring& line)
{
	// 형식: SCENE:1,CLEARED:1,DATE:2026-02-18
	size_t scenePos = line.find(L"SCENE:") + 6;
	size_t clearedPos = line.find(L",CLEARED:") + 9;
	size_t datePos = line.find(L",DATE:") + 6;
	
	int sceneType = std::stoi(line.substr(scenePos, line.find(L',', scenePos) - scenePos));
	int cleared = std::stoi(line.substr(clearedPos, line.find(L',', clearedPos) - clearedPos));
	std::wstring date = line.substr(datePos);
	
	// 해당 씬 정보 업데이트
	for (auto& sceneInfo : m_gameProgress.sceneClearInfos)
	{
		if ((int)sceneInfo.sceneType == sceneType)
		{
			sceneInfo.isCleared = (cleared == 1);
			sceneInfo.clearDate = date;
			break;
		}
	}
}

void GameProgressManager::ParseCharacterLine(const std::wstring& line)
{
	// 형식: CHARACTER:164,UNLOCKED:1
	size_t charPos = line.find(L"CHARACTER:") + 10;
	size_t unlockedPos = line.find(L",UNLOCKED:") + 10;
	
	int characterID = std::stoi(line.substr(charPos, line.find(L',', charPos) - charPos));
	int unlocked = std::stoi(line.substr(unlockedPos));
	
	// 해당 캐릭터 정보 업데이트
	for (auto& charInfo : m_gameProgress.characterUnlockInfos)
	{
		if ((int)charInfo.characterID == characterID)
		{
			charInfo.isUnlocked = (unlocked == 1);
			break;
		}
	}
}

void GameProgressManager::ParseKillCountLine(const std::wstring& line)
{
	// 형식: KILL_COUNT:146:3
	size_t firstColon = line.find(L':');
	size_t secondColon = line.find(L':', firstColon + 1);
	
	int monsterID = std::stoi(line.substr(firstColon + 1, secondColon - firstColon - 1));
	int count = std::stoi(line.substr(secondColon + 1));
	
	m_currentSceneKillCounts[(GameObjectID)monsterID] = count;
}

void GameProgressManager::ParseItemCountLine(const std::wstring& line)
{
	// 형식: ITEM_COUNT:160:2
	size_t firstColon = line.find(L':');
	size_t secondColon = line.find(L':', firstColon + 1);
	
	int itemID = std::stoi(line.substr(firstColon + 1, secondColon - firstColon - 1));
	int count = std::stoi(line.substr(secondColon + 1));
	
	m_currentSceneItemCounts[(GameObjectID)itemID] = count;
}
