#include "99_Default/pch.h"
#include "SoundManager.h"

SoundManager::SoundManager()
    : m_strCurrentBGMPath(L"")
    , m_fBGMVolume(0.5f)
    , m_iSFXCount(0)
{
}

SoundManager::~SoundManager()
{
    Release();
}

void SoundManager::Init()
{
    // MCI does not require special initialization.
}

void SoundManager::Release()
{
    StopBGM();
    mciSendString(L"close all", NULL, 0, NULL);
}

void SoundManager::PlayBGM(const std::wstring& strPath, float fVolume)
{
    // Same BGM already playing
    if (m_strCurrentBGMPath == strPath)
        return;

    StopBGM();

    // Normalize path (replace / with \)
    std::wstring normalizedPath = strPath;
    for (auto& ch : normalizedPath) {
        if (ch == L'/') ch = L'\\';
    }

    // Convert to absolute path for MCI reliability
    wchar_t szFullPath[MAX_PATH];
    std::wstring finalPath;
    if (_wfullpath(szFullPath, normalizedPath.c_str(), MAX_PATH) != nullptr) {
        finalPath = szFullPath;
    } else {
        finalPath = normalizedPath;
    }

    m_strCurrentBGMPath = strPath;
    m_fBGMVolume = fVolume;

    // MCI often works better with absolute paths.
    std::wstring strCommand = L"open \"" + finalPath + L"\" type mpegvideo alias bgm";
    if (mciSendString(strCommand.c_str(), NULL, 0, NULL) != 0)
    {
        // Try without type if it fails
        strCommand = L"open \"" + finalPath + L"\" alias bgm";
        mciSendString(strCommand.c_str(), NULL, 0, NULL);
    }

    mciSendString(L"play bgm repeat", NULL, 0, NULL);

    SetBGMVolume(fVolume);
}

void SoundManager::StopBGM()
{
    mciSendString(L"stop bgm", NULL, 0, NULL);
    mciSendString(L"close bgm", NULL, 0, NULL);
    m_strCurrentBGMPath = L"";
}

void SoundManager::SetBGMVolume(float fVolume)
{
    m_fBGMVolume = fVolume;
    int iVolume = static_cast<int>(fVolume * 1000.0f);
    std::wstring strCommand = L"setaudio bgm volume to " + std::to_wstring(iVolume);
    mciSendString(strCommand.c_str(), NULL, 0, NULL);
}

void SoundManager::PlaySFX(const std::wstring& strPath, float fVolume)
{
    std::wstring strAlias = L"sfx" + std::to_wstring(m_iSFXCount);
    
    // Close existing alias if it was already open from previous wrap-around
    std::wstring strClose = L"close " + strAlias;
    mciSendString(strClose.c_str(), NULL, 0, NULL);

    // Normalize path (replace / with \)
    std::wstring normalizedPath = strPath;
    for (auto& ch : normalizedPath) {
        if (ch == L'/') ch = L'\\';
    }

    // Convert to absolute path for MCI reliability
    wchar_t szFullPath[MAX_PATH];
    std::wstring finalPath;
    if (_wfullpath(szFullPath, normalizedPath.c_str(), MAX_PATH) != nullptr) {
        finalPath = szFullPath;
    } else {
        finalPath = normalizedPath;
    }

    std::wstring strOpen = L"open \"" + finalPath + L"\" type mpegvideo alias " + strAlias;
    if (mciSendString(strOpen.c_str(), NULL, 0, NULL) != 0)
    {
        // Try without type if it fails
        strOpen = L"open \"" + finalPath + L"\" alias " + strAlias;
        mciSendString(strOpen.c_str(), NULL, 0, NULL);
    }

    int iVolume = static_cast<int>(fVolume * 1000.0f);
    std::wstring strVol = L"setaudio " + strAlias + L" volume to " + std::to_wstring(iVolume);
    mciSendString(strVol.c_str(), NULL, 0, NULL);

    std::wstring strPlay = L"play " + strAlias + L" from 0";
    mciSendString(strPlay.c_str(), NULL, 0, NULL);

    m_iSFXCount++;
    if (m_iSFXCount > 20) // Limit simultaneous SFX to 20 for MCI stability
        m_iSFXCount = 0;
}
