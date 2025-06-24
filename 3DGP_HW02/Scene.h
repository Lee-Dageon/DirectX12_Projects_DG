//-----------------------------------------------------------------------------
// File: Scene.h
//-----------------------------------------------------------------------------

#pragma once

#include "Shader.h"
#include "Player.h"  // Player 클래스 포함
#include "Object.h"  // CSplitedUFO 클래스를 포함하기 위해 추가

class CScene
{
public:
    CScene();
    ~CScene();

	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	void ReleaseObjects();

	ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	ID3D12RootSignature *GetGraphicsRootSignature() { return(m_pd3dGraphicsRootSignature); }

	bool ProcessInput();
    void AnimateObjects(float fTimeElapsed);
	void PrepareRender(ID3D12GraphicsCommandList* pd3dCommandList);
	void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera=NULL);

	void ReleaseUploadBuffers();

	// 총알 생성 함수를 public으로 이동
	void CreateBullet();

	// 플레이어 설정 함수 추가
	void SetPlayer(CPlayer* pPlayer) { m_pPlayer = pPlayer; }

	// 모든 UFO 폭발 함수 추가
	void ExplodeAllUFOs();

protected:
	ID3D12RootSignature			*m_pd3dGraphicsRootSignature = NULL;

	CGameObject					**m_ppObjects = 0;
	int							m_nObjects = 0;

	// 총알 관련 리소스 추가
	CMesh						*m_pBulletMesh = NULL;
	CShader						*m_pBulletShader = NULL;

	// 플레이어 포인터 추가
	CPlayer						*m_pPlayer = NULL;

	// 승리 상태를 저장하는 변수 추가
	bool						m_bGameWon = false;
	
	// 승리 타이머 변수 추가
	float                       m_fVictoryTimer = 0.0f;
	bool                       m_bVictoryMessageShown = false;

	// 지형 관련 멤버 추가
	CHeightMapTerrain*			m_pTerrain = NULL;

public:
	CHeightMapTerrain* GetTerrain() { return(m_pTerrain); }
};
