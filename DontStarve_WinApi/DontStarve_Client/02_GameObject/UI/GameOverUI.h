#pragma once

class UIText;
class UIButton;

class GameOverUI
{
public:
    GameOverUI();
    ~GameOverUI();

    void Init();
    void Update(float deltaTime);
    void Render();
    void Release();

    void Show();
    void Hide();
    bool IsVisible() const { return m_isVisible; }

private:
    UIText* m_gameOverText;
    UIButton* m_btnToLobby;
    UIText* m_btnToLobbyText;
    UIButton* m_btnQuit;
    UIText* m_btnQuitText;

    bool m_isVisible;

    static const float SORT_KEY;
};
