//------------------------------------------------------- ----------------------
// File: Object.h
//-----------------------------------------------------------------------------

#pragma once

#include "Mesh.h"
#include "Camera.h"

class CShader;

class CGameObject
{
public:
	CGameObject();
	virtual ~CGameObject();

public:
	XMFLOAT4X4						m_xmf4x4World;
	CMesh							*m_pMesh = NULL;

	CShader							*m_pShader = NULL;

	XMFLOAT3						m_xmf3Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	bool							m_bSplitUFO = false;  // 분열된 UFO인지 표시하는 플래그

	void SetMesh(CMesh *pMesh);
	void SetShader(CShader *pShader);

	CMesh* GetMesh() { return m_pMesh; }
	CShader* GetShader() { return m_pShader; }

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void Animate(float fTimeElapsed);
	virtual void OnPrepareRender() { }
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);

	virtual void ReleaseUploadBuffers();

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();

	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xmf3Position);

	void SetColor(XMFLOAT3 xmf3Color) { m_xmf3Color = xmf3Color; }
	XMFLOAT3 GetColor() { return m_xmf3Color; }

	void SetSplitUFO(bool bSplit) { m_bSplitUFO = bSplit; }
	bool IsSplitUFO() const { return m_bSplitUFO; }

	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);

	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void Rotate(XMFLOAT3 *pxmf3Axis, float fAngle);

	void SetScale(float x, float y, float z);
};

class CUfoObject : public CGameObject
{
public:
	CUfoObject();
	virtual ~CUfoObject();
};

class CBulletObject : public CGameObject
{
public:
	CBulletObject();
	virtual ~CBulletObject();

	virtual void Animate(float fTimeElapsed);
	
	void SetVelocity(XMFLOAT3 xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }
	XMFLOAT3 GetVelocity() const { return m_xmf3Velocity; }

	bool IsOutOfRange() const { return m_fTravelDistance >= m_fMaxTravelDistance; }
	
	void SetMaxTravelDistance(float fMaxDistance) { m_fMaxTravelDistance = fMaxDistance; }

protected:
	XMFLOAT3 m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float m_fTravelDistance = 0.0f;
	float m_fMaxTravelDistance = 100.0f;
};

class CSplitedUFO : public CGameObject
{
public:
	CSplitedUFO();
	virtual ~CSplitedUFO();

	virtual void Animate(float fTimeElapsed);
	
	void SetVelocity(XMFLOAT3 xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }
	XMFLOAT3 GetVelocity() const { return m_xmf3Velocity; }
	
	bool IsExpired() const { return (m_fLifeTime >= m_fMaxLifeTime); }

protected:
	XMFLOAT3 m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float m_fLifeTime = 0.0f;  // 생성 후 경과 시간
	const float m_fMaxLifeTime = 10.0f;  // 최대 수명 (10초)
};

// 지형 관련 클래스들 추가
class CHeightMapTerrain : public CGameObject
{
public:
	CHeightMapTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, 
		ID3D12RootSignature* pd3dGraphicsRootSignature, LPCTSTR pFileName, int nWidth, int nLength, 
		int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color);
	virtual ~CHeightMapTerrain();

private:
	//지형의 높이 맵으로 사용할 이미지이다. 
	class CHeightMapImage* m_pHeightMapImage;
	//높이 맵의 가로와 세로 크기이다. 
	int m_nWidth;
	int m_nLength;
	//지형을 실제로 몇 배 확대할 것인가를 나타내는 스케일 벡터이다. 
	XMFLOAT3 m_xmf3Scale;

	// 다중 메쉬 지원을 위한 멤버 변수들
	CMesh** m_ppMeshes = NULL;
	int m_nMeshes = 0;

public:
	//지형의 높이를 계산하는 함수이다(월드 좌표계). 높이 맵의 높이에 스케일의 y를 곱한 값이다. 
	float GetHeight(float x, float z);
	//지형의 법선 벡터를 계산하는 함수이다(월드 좌표계). 높이 맵의 법선 벡터를 사용한다. 
	XMFLOAT3 GetNormal(float x, float z);
	int GetHeightMapWidth();
	int GetHeightMapLength();
	XMFLOAT3 GetScale() { return(m_xmf3Scale); }
	//지형의 크기(가로/세로)를 반환한다. 높이 맵의 크기에 스케일을 곱한 값이다. 
	float GetWidth();
	float GetLength();

	void SetMesh(int nIndex, CMesh* pMesh);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
	virtual void ReleaseUploadBuffers();
};

