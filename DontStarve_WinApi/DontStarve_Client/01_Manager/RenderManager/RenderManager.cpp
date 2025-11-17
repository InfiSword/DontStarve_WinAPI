#include "../../99_Default/pch.h"
#include "RenderManager.h"
#include "../../02_GameObject/GameObject/GameObject.h"
#include "../../02_GameObject/Player/Player.h"
#include "../../02_GameObject/GameObject/Tree.h"
#include "../../02_GameObject/GameObject/Rock.h"
#include "../../03_Animation/Animator.h"
#include "../CameraManager/CameraManager.h"
#include "../ObjectManager/ObjectManager.h"
#include "../../../Header/Enum.h"

RenderManager::RenderManager() {}
RenderManager::~RenderManager() {}

void RenderManager::Init()
{
	m_drawCommands.reserve(1000); // 성능 최적화를 위한 사전 할당
}

void RenderManager::LateInit()
{
}

void RenderManager::Update(float deltaTime)
{
}

void RenderManager::LateUpdate()
{
}

void RenderManager::Render()
{
	OutputDebugStringW(L"RenderManager: Render() 메서드 호출됨\n");
	
	// 화면에 보이는 게임 오브젝트들 렌더링
	RenderVisibleGameObjects();
	
	// ObjectManager의 테두리 렌더링 (UI 레이어에 그리기)
	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (objectManager && objectManager->IsBoundsDisplayEnabled()) {
		objectManager->RenderBounds();
	}
	
	OutputDebugStringW(L"RenderManager: Render() 메서드 완료\n");
}

void RenderManager::Release()
{
	Clear();
}

void RenderManager::AddDrawCommand(Gdiplus::Bitmap* pBitmap, const Gdiplus::RectF& destRect, const Gdiplus::RectF& sourceRect, Gdiplus::Unit srcUnit, const Gdiplus::PointF& objectScreenPos, RenderLayer layer, float sortKey, Direction direction)
{
	// 비트맵 유효성 검사
	if (!pBitmap) {
		return;
	}

	if (pBitmap->GetLastStatus() != Gdiplus::Ok) {
		return;
	}

	m_drawCommands.emplace_back(DrawCommand(pBitmap, destRect, sourceRect, srcUnit, objectScreenPos, layer, sortKey, direction));
}

void RenderManager::AddTextCommand(const std::wstring& text, Gdiplus::Font* pFont, Gdiplus::Brush* pBrush, Gdiplus::StringFormat* pStringFormat, const Gdiplus::RectF& destRect, RenderLayer layer, float sortKey)
{
	m_drawCommands.emplace_back(DrawCommand(text, pFont, pBrush, pStringFormat, destRect, layer, sortKey));
}

//하이라이트 용
void RenderManager::AddDrawCommand(const Gdiplus::RectF& rect, const Gdiplus::Color& color, float thickness, RenderLayer layer, float sortKey) {
	m_drawCommands.emplace_back(DrawCommand(rect, color, thickness, layer, sortKey));
}

void RenderManager::AddFillRectangleCommand(const Gdiplus::RectF& rect, const Gdiplus::Color& color, RenderLayer layer, float sortKey) {
	m_drawCommands.emplace_back(DrawCommand(rect, color, layer, sortKey, true));
}

// === UI 렌더링 전용 메소드들 구현 ===

void RenderManager::RenderUIImage(Gdiplus::Bitmap* bitmap, float x, float y, float width, float height,
	RenderLayer layer, float sortKey)
{
	if (!bitmap) return;

	Gdiplus::RectF destRect(x, y, width, height);
	Gdiplus::RectF sourceRect(0, 0, static_cast<float>(bitmap->GetWidth()), static_cast<float>(bitmap->GetHeight()));

	AddDrawCommand(bitmap, destRect, sourceRect, Gdiplus::UnitPixel,
		Gdiplus::PointF(x + width / 2, y + height / 2), layer, sortKey, DIR_DOWN);
}

void RenderManager::RenderUIText(const std::wstring& text, Gdiplus::Font* font, Gdiplus::Brush* brush,
	float x, float y, float width, float height,
	RenderLayer layer, float sortKey)
{
	// 모든 포인터가 유효한지 확인
	if (!font || !brush || text.empty()) return;

	// 폰트와 브러시의 상태 확인
	Gdiplus::FontFamily fontFamily;
	if (font->GetFamily(&fontFamily) != Gdiplus::Ok) return;

	Gdiplus::StringFormat* stringFormat = new Gdiplus::StringFormat();
	if (!stringFormat) return;

	stringFormat->SetAlignment(Gdiplus::StringAlignmentCenter);
	stringFormat->SetLineAlignment(Gdiplus::StringAlignmentCenter);

	Gdiplus::RectF textRect(x, y, width, height);
	AddTextCommand(text, font, brush, stringFormat, textRect, layer, sortKey);

	// StringFormat은 DrawCommand 내부에서 관리되므로 여기서는 해제하지 않음
}

// === 기존 GameObject 렌더링 메소드들 ===

void RenderManager::RenderGameObject(GameObject* pObject)
{
	if (!pObject || !pObject->GetActive()) {
		OutputDebugStringW(L"RenderManager: GameObject가 null이거나 비활성화되어 있습니다.\n");
		return;
	}

	OutputDebugStringW((L"RenderManager: GameObject 렌더링 시작 - ID: " + std::to_wstring(pObject->GetID()) + L", Type: " + std::to_wstring(static_cast<int>(pObject->GetType())) + L"\n").c_str());

	// Animator가 있는 경우 애니메이션 렌더링
	if (pObject->GetAnimator()) {
		OutputDebugStringW(L"RenderManager: Animator를 사용하여 렌더링합니다.\n");
		
		// 월드 좌표를 스크린 좌표로 변환
		Gdiplus::PointF screenPos = CameraManager::GetInstance()->WorldToScreen(pObject->GetX(), pObject->GetY());

		// 렌더 레이어 결정
		RenderLayer layer = GetRenderLayerForObject(pObject);

		// 정렬 키 생성 (Y축 기반)
		float sortKey = layer + static_cast<int>(pObject->GetY());

		// Animator의 Draw 호출하여 RenderManager 큐에 추가
		pObject->GetAnimator()->Draw(nullptr, screenPos, 1.0f, pObject->GetDir(), layer, sortKey);
	}
	else 
	{
		OutputDebugStringW(L"RenderManager: 기본 비트맵을 사용하여 렌더링합니다.\n");
		
		// Animator가 없는 경우 기본 비트맵 렌더링
		Gdiplus::Bitmap* pBitmap = pObject->GetBitmap();
		if (!pBitmap) {
			OutputDebugStringW((L"RenderManager: GameObject의 비트맵이 null입니다. (ID: " + std::to_wstring(pObject->GetID()) + L")\n").c_str());
			return;
		}
		
		// 월드 좌표를 스크린 좌표로 변환
		Gdiplus::PointF screenPos = CameraManager::GetInstance()->WorldToScreen(pObject->GetX(), pObject->GetY());

		// 오브젝트 크기 계산
		float width = static_cast<float>(pBitmap->GetWidth());
		float height = static_cast<float>(pBitmap->GetHeight());

		// 피벗 적용하여 렌더링 위치 계산
		float renderX = screenPos.X - width * pObject->GetPivotX();
		float renderY = screenPos.Y - height * pObject->GetPivotY();
		
		// 디버그 출력 (너무 자주 출력하지 않도록 제한)
		static int renderDebugCounter = 0;
		if (++renderDebugCounter % 50 == 0) {
			OutputDebugStringW((L"RenderManager: 오브젝트 렌더링 - ID: " + 
				std::to_wstring(pObject->GetID()) + L", 월드 위치: (" + 
				std::to_wstring(pObject->GetX()) + L", " + std::to_wstring(pObject->GetY()) + 
				L"), 화면 위치: (" + std::to_wstring(screenPos.X) + L", " + 
				std::to_wstring(screenPos.Y) + L"), 렌더링 위치: (" + 
				std::to_wstring(renderX) + L", " + std::to_wstring(renderY) + L")\n").c_str());
		}

		// 렌더 레이어 결정
		RenderLayer layer = GetRenderLayerForObject(pObject);

		// 정렬 키 생성 (Y축 기반)
		float sortKey = layer + static_cast<int>(pObject->GetY());

		// RenderManager를 통해 렌더링 명령 추가
		AddDrawCommand(
			pBitmap,
			Gdiplus::RectF(renderX, renderY, width, height),
			Gdiplus::RectF(0, 0, width, height),
			Gdiplus::UnitPixel,
			screenPos,
			layer,
			sortKey,
			pObject->GetDir()
		);
		
		OutputDebugStringW((L"RenderManager: 렌더링 명령 추가 완료 - 위치: (" + std::to_wstring(renderX) + L", " + std::to_wstring(renderY) + L"), 크기: (" + std::to_wstring(width) + L", " + std::to_wstring(height) + L")\n").c_str());
	}
}

void RenderManager::RenderTile(Gdiplus::Bitmap* pTileBitmap, float worldX, float worldY, float width, float height)
{
	if (!pTileBitmap) return;

	// 월드 좌표를 스크린 좌표로 변환 (타일의 중심점)
	Gdiplus::PointF screenPos = CameraManager::GetInstance()->WorldToScreen(worldX, worldY);

	// 타일 렌더링 스크린 위치 계산 
	float renderX = screenPos.X - width * 0.5f; // 중심점 X - 이미지 너비의 절반 = 좌상단 X
	float renderY = screenPos.Y - height * 0.5f; // 중심점 Y - 이미지 높이의 절반 = 좌상단 Y

	// 타일은 항상 가장 아래 레이어
	float sortKey = LAYER_WORLD_TILE;

	// 드로우 커맨드 추가
	Gdiplus::RectF destRect(renderX, renderY, width, height); 
	Gdiplus::RectF sourceRect(0, 0, static_cast<float>(pTileBitmap->GetWidth()), static_cast<float>(pTileBitmap->GetHeight()));

	AddDrawCommand(pTileBitmap, destRect, sourceRect, Gdiplus::UnitPixel, screenPos, LAYER_WORLD_TILE, sortKey, DIR_DOWN);
}

void RenderManager::RenderAllGameObjects(const std::vector<GameObject*>& gameObjects)
{
	// 모든 게임 오브젝트 렌더링 (애니메이션 업데이트는 ObjectManager::Update()에서 처리됨)
	for (GameObject* obj : gameObjects)
	{
		if (obj && obj->GetActive())
		{
			RenderGameObject(obj);
		}
	}
}

void RenderManager::RenderVisibleGameObjects()
{
	OutputDebugStringW(L"RenderManager: RenderVisibleGameObjects() 시작\n");
	
	// ObjectManager에서 오브젝트 수 확인
	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (!objectManager) {
		OutputDebugStringW(L"RenderManager: ObjectManager가 null입니다.\n");
		return;
	}
	
	const std::vector<GameObject*>& allObjects = objectManager->GetGameObjects();
	if (allObjects.empty()) {
		OutputDebugStringW(L"RenderManager: ObjectManager에 오브젝트가 없습니다. (초기화 중)\n");
		return;
	}
	
	// CameraManager에서 보이는 오브젝트들 가져오기
	CameraManager* cameraManager = CameraManager::GetInstance();
	if (!cameraManager) {
		OutputDebugStringW(L"RenderManager: CameraManager가 null입니다.\n");
		return;
	}

	const std::vector<GameObject*>& visibleObjects = cameraManager->GetVisibleObjects();
	
	if (visibleObjects.empty()) {
		OutputDebugStringW(L"RenderManager: CameraManager의 visibleObjects가 비어있습니다. ObjectManager에서 직접 가져옵니다.\n");
		
		// ObjectManager에서 모든 오브젝트 가져오기
		ObjectManager* objectManager = ObjectManager::GetInstance();
		if (!objectManager) {
			OutputDebugStringW(L"RenderManager: ObjectManager가 null입니다.\n");
			return;
		}
		
		const std::vector<GameObject*>& allObjects = objectManager->GetGameObjects();
		OutputDebugStringW((L"RenderManager: ObjectManager에서 " + std::to_wstring(allObjects.size()) + L"개의 오브젝트를 가져왔습니다.\n").c_str());
		
		// 모든 오브젝트 렌더링
		for (GameObject* obj : allObjects) {
			if (obj && obj->GetActive()) {
				RenderGameObject(obj);
			}
		}
	} else {
		OutputDebugStringW((L"RenderManager: CameraManager에서 " + std::to_wstring(visibleObjects.size()) + L"개의 visible 오브젝트를 렌더링합니다.\n").c_str());
		
		// 보이는 오브젝트들만 렌더링
		for (GameObject* obj : visibleObjects) {
			if (obj && obj->GetActive()) {
				RenderGameObject(obj);
			}
		}
	}
}

RenderLayer RenderManager::GetRenderLayerForObject(GameObject* pObject)
{
	if (!pObject) 
		return LAYER_WORLD_OBJECT;

	GameObjectType type = pObject->GetType();

	switch (type)
	{
	case GOBJ_NATURAL_ENVIR:
	case GOBJ_BUILDING:
	case GOBJ_MONSTER:
	case GOBJ_PLAYER:
	case GOBJ_ITEM:
		return LAYER_WORLD_OBJECT; // 모든 게임 오브젝트는 동일한 레이어

	default:
		return LAYER_WORLD_OBJECT;
	}
}

void RenderManager::Clear() {
	m_drawCommands.clear();
}

// 이 함수는 Animator::Draw에서 가져온 변환 로직을 그대로 유지합니다.
void RenderManager::ApplyGdiTransform(Gdiplus::Graphics* pGraphics, const DrawCommand& command, float scaledWidth, float scaledHeight) {

	if (command.layer >= LAYER_UI_BACKGROUND || command.type == DRAW_COMMAND_TEXT)
	{
		return;
	}

	float transformCenterX = command.destRect.X + scaledWidth / 2.0f;
	float transformCenterY = command.destRect.Y + scaledHeight / 2.0f;
	switch (command.direction) {
	case DIR_DOWN:
		break;
	case DIR_UP:
		break;
	case DIR_LEFT:
		pGraphics->TranslateTransform(transformCenterX, transformCenterY);
		pGraphics->ScaleTransform(-1.0f, 1.0f); // X축 반전
		pGraphics->TranslateTransform(-transformCenterX, -transformCenterY);
		break;
	case DIR_RIGHT:
		break;
	}
}

void RenderManager::Flush(Gdiplus::Graphics* pGraphics) {
	if (!pGraphics || m_drawCommands.empty()) return;

	// 1. 레이어 우선순위 정렬 (Z-ordering & 배치 최적화)
	std::sort(m_drawCommands.begin(), m_drawCommands.end(), CompareDrawCommands);

	Gdiplus::Bitmap* currentBitmap = nullptr;
	Gdiplus::GraphicsState originalGraphicsState; // Flush 시작시 그래픽 상태

	int commandIndex = 0;
	for (const auto& cmd : m_drawCommands) {
		Gdiplus::GraphicsState commandSpecificState = pGraphics->Save(); // 각 커맨드마다 그래픽 상태 저장

		float currentScaledWidth = cmd.destRect.Width;
		float currentScaledHeight = cmd.destRect.Height;

		if (cmd.type == DRAW_COMMAND_IMAGE) {
			// 비트맵 유효성 검사 추가
			if (!cmd.pBitmap) {
				pGraphics->Restore(commandSpecificState);
				commandIndex++;
				continue;
			}

			// 비트맵 상태 검사
			if (cmd.pBitmap->GetLastStatus() != Gdiplus::Ok) {
				pGraphics->Restore(commandSpecificState);
				commandIndex++;
				continue;
			}

			// 소스/목적지 사각형 유효성 검사
			if (cmd.sourceRect.Width <= 0 || cmd.sourceRect.Height <= 0) {
				pGraphics->Restore(commandSpecificState);
				commandIndex++;
				continue;
			}

			if (cmd.destRect.Width <= 0 || cmd.destRect.Height <= 0) {
				pGraphics->Restore(commandSpecificState);
				commandIndex++;
				continue;
			}

			ApplyGdiTransform(pGraphics, cmd, cmd.destRect.Width, cmd.destRect.Height);
			pGraphics->DrawImage(cmd.pBitmap, cmd.destRect, cmd.sourceRect.X, cmd.sourceRect.Y,
				cmd.sourceRect.Width, cmd.sourceRect.Height, cmd.srcUnit);

		}
		else if (cmd.type == DRAW_COMMAND_TEXT) {
			if (cmd.pFont && cmd.pBrush && cmd.pStringFormat) {

				pGraphics->DrawString(cmd.text.c_str(), -1, cmd.pFont, cmd.destRect, cmd.pStringFormat, cmd.pBrush);
			}
		}
		else if (cmd.type == DRAW_COMMAND_RECTANGLE) {

			Gdiplus::Pen pen(cmd.color, cmd.thickness);
			pGraphics->DrawRectangle(&pen, cmd.destRect);

		}
		else if (cmd.type == DRAW_COMMAND_FILL_RECTANGLE) {
			Gdiplus::SolidBrush brush(cmd.color);
			pGraphics->FillRectangle(&brush, cmd.destRect);

		}
		pGraphics->Restore(commandSpecificState);
		commandIndex++;
	}
	Clear(); // 모든 커맨드를 그린 후 큐 정리
}