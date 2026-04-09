#pragma once

class SoundManager : public CSingleTon<SoundManager>
{
    friend class CSingleTon<SoundManager>;

private:
    SoundManager();
    virtual ~SoundManager();

public:
    void Init();
    void Release();

    // BGM (mci) - Only one BGM at a time
    void PlayBGM(const std::wstring& strPath, float fVolume = 0.5f);
    void StopBGM();
    void SetBGMVolume(float fVolume);

    // SFX (mci) - Multiple SFX can play simultaneously
    void PlaySFX(const std::wstring& strPath, float fVolume = 0.5f);

private:
    std::wstring m_strCurrentBGMPath;
    float m_fBGMVolume;
    int m_iSFXCount; // For unique aliases
};
