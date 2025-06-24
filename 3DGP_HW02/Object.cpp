//-----------------------------------------------------------------------------
// File: CGameObject.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Object.h"
#include "Shader.h"

CGameObject::CGameObject()
{
	m_xmf4x4World = Matrix4x4::Identity();
}

CGameObject::~CGameObject()
{
	if (m_pMesh) m_pMesh->Release();
	if (m_pShader) m_pShader->Release();
}

void CGameObject::SetMesh(CMesh *pMesh)
{
	if (m_pMesh) m_pMesh->Release();
	m_pMesh = pMesh;
	if (m_pMesh) m_pMesh->AddRef();
}

void CGameObject::SetShader(CShader *pShader)
{
	if (m_pShader) m_pShader->Release();
	m_pShader = pShader;
	if (m_pShader) m_pShader->AddRef();
}

void CGameObject::Animate(float fTimeElapsed)
{
}

void CGameObject::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CGameObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	XMFLOAT4X4 xmf4x4World;
	XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 16, &xmf4x4World, 0);

	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 3, &m_xmf3Color, 16);
}

void CGameObject::ReleaseShaderVariables()
{
}

void CGameObject::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	OnPrepareRender();

	if (m_pShader) m_pShader->Render(pd3dCommandList, pCamera);

	UpdateShaderVariables(pd3dCommandList);

	if (m_pMesh) m_pMesh->Render(pd3dCommandList);
}

void CGameObject::ReleaseUploadBuffers()
{
	if (m_pMesh) m_pMesh->ReleaseUploadBuffers();
}

void CGameObject::SetPosition(float x, float y, float z)
{
	m_xmf4x4World._41 = x;
	m_xmf4x4World._42 = y;
	m_xmf4x4World._43 = z;
}

void CGameObject::SetPosition(XMFLOAT3 xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

XMFLOAT3 CGameObject::GetPosition()
{
	return(XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43));
}

XMFLOAT3 CGameObject::GetLook()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33)));
}

XMFLOAT3 CGameObject::GetUp()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23)));
}

XMFLOAT3 CGameObject::GetRight()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12, m_xmf4x4World._13)));
}

void CGameObject::MoveStrafe(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Right, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveUp(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Up, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveForward(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

void CGameObject::Rotate(XMFLOAT3 *pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

void CGameObject::SetScale(float x, float y, float z)
{
	XMMATRIX mtxScale = XMMatrixScaling(x, y, z);
	m_xmf4x4World = Matrix4x4::Multiply(mtxScale, m_xmf4x4World);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CUfoObject::CUfoObject()
{

}

CUfoObject::~CUfoObject()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CBulletObject::CBulletObject()
{
}

CBulletObject::~CBulletObject()
{
}

void CBulletObject::Animate(float fTimeElapsed)
{
	// 현재 위치에 속도 * 시간을 더해서 새로운 위치 계산
	XMFLOAT3 xmf3Position = GetPosition();
	float fDistance = fTimeElapsed * 100.0f;  // 이번 프레임의 이동 거리
	
	xmf3Position.x += m_xmf3Velocity.x * fDistance;
	xmf3Position.y += m_xmf3Velocity.y * fDistance;
	xmf3Position.z += m_xmf3Velocity.z * fDistance;
	
	SetPosition(xmf3Position);
	
	// 이동한 거리 누적
	m_fTravelDistance += fDistance;

	CGameObject::Animate(fTimeElapsed);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CSplitedUFO::CSplitedUFO()
{
	m_bSplitUFO = true;
}

CSplitedUFO::~CSplitedUFO()
{
}

void CSplitedUFO::Animate(float fTimeElapsed)
{
	// 생존 시간 업데이트
	m_fLifeTime += fTimeElapsed;

	// 아직 살아있을 때만 이동 처리
	if (!IsExpired())
	{
		// 현재 위치에 속도를 더해서 이동
		XMFLOAT3 position = GetPosition();
		position.x += m_xmf3Velocity.x * fTimeElapsed;
		position.y += m_xmf3Velocity.y * fTimeElapsed;
		position.z += m_xmf3Velocity.z * fTimeElapsed;
		SetPosition(position);
	}

	CGameObject::Animate(fTimeElapsed);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 지형 관련 구현
CHeightMapTerrain::CHeightMapTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, 
	ID3D12RootSignature* pd3dGraphicsRootSignature, LPCTSTR pFileName, int nWidth, int nLength, 
	int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color) : CGameObject()
{
	//지형에 사용할 높이 맵의 가로, 세로의 크기이다. 
	m_nWidth = nWidth;
	m_nLength = nLength;
	/*지형 객체는 격자 메쉬들의 배열로 만들 것이다. nBlockWidth, nBlockLength는 격자 메쉬 하나의 가로, 세로 크기이다.
	cxQuadsPerBlock, czQuadsPerBlock은 격자 메쉬의 가로 방향과 세로 방향 사각형의 개수이다.*/
	int cxQuadsPerBlock = nBlockWidth - 1;
	int czQuadsPerBlock = nBlockLength - 1;
	//xmf3Scale는 지형을 실제로 몇 배 확대할 것인가를 나타낸다. 
	m_xmf3Scale = xmf3Scale;
	//지형에 사용할 높이 맵을 생성한다. 
	m_pHeightMapImage = new CHeightMapImage(pFileName, nWidth, nLength, xmf3Scale);
	//지형에서 가로 방향, 세로 방향으로 격자 메쉬가 몇 개가 있는 가를 나타낸다. 
	long cxBlocks = (m_nWidth - 1) / cxQuadsPerBlock;
	long czBlocks = (m_nLength - 1) / czQuadsPerBlock;
	//지형 전체를 표현하기 위한 격자 메쉬의 개수이다. 
	m_nMeshes = cxBlocks * czBlocks;

	//지형 전체를 표현하기 위한 격자 메쉬에 대한 포인터 배열을 생성한다. 
	m_ppMeshes = new CMesh*[m_nMeshes];
	for (int i = 0; i < m_nMeshes; i++) m_ppMeshes[i] = NULL;
	
	CHeightMapGridMesh* pHeightMapGridMesh = NULL;
	for (int z = 0, zStart = 0; z < czBlocks; z++)
	{
		for (int x = 0, xStart = 0; x < cxBlocks; x++)
		{
			//지형의 일부분을 나타내는 격자 메쉬의 시작 위치(좌표)이다. 
			xStart = x * (nBlockWidth - 1);
			zStart = z * (nBlockLength - 1);
			//지형의 일부분을 나타내는 격자 메쉬를 생성하여 지형 메쉬에 저장한다. 
			pHeightMapGridMesh = new CHeightMapGridMesh(pd3dDevice, pd3dCommandList, xStart,
				zStart, nBlockWidth, nBlockLength, xmf3Scale, xmf4Color, m_pHeightMapImage);
			SetMesh(x + (z * cxBlocks), pHeightMapGridMesh);
		}
	}
	//지형을 렌더링하기 위한 셰이더를 생성한다. 
	CTerrainShader* pShader = new CTerrainShader();
	pShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	SetShader(pShader);
}

CHeightMapTerrain::~CHeightMapTerrain()
{
	if (m_pHeightMapImage) delete m_pHeightMapImage;
	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i]) m_ppMeshes[i]->Release();
			m_ppMeshes[i] = NULL;
		}
		delete[] m_ppMeshes;
	}
}

float CHeightMapTerrain::GetHeight(float x, float z) 
{
	return(m_pHeightMapImage->GetHeight(x / m_xmf3Scale.x, z / m_xmf3Scale.z) * m_xmf3Scale.y);
}

XMFLOAT3 CHeightMapTerrain::GetNormal(float x, float z) 
{
	return(m_pHeightMapImage->GetHeightMapNormal(int(x / m_xmf3Scale.x), int(z / m_xmf3Scale.z)));
}

int CHeightMapTerrain::GetHeightMapWidth() 
{ 
	return(m_pHeightMapImage->GetHeightMapWidth()); 
}

int CHeightMapTerrain::GetHeightMapLength() 
{ 
	return(m_pHeightMapImage->GetHeightMapLength()); 
}

float CHeightMapTerrain::GetWidth() 
{ 
	return(m_nWidth * m_xmf3Scale.x); 
}

float CHeightMapTerrain::GetLength() 
{ 
	return(m_nLength * m_xmf3Scale.z); 
}

void CHeightMapTerrain::SetMesh(int nIndex, CMesh* pMesh)
{
	if (m_ppMeshes)
	{
		if (m_ppMeshes[nIndex]) m_ppMeshes[nIndex]->Release();
		m_ppMeshes[nIndex] = pMesh;
		if (pMesh) pMesh->AddRef();
	}
}

void CHeightMapTerrain::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	OnPrepareRender();
	UpdateShaderVariables(pd3dCommandList);
	if (m_pShader) m_pShader->Render(pd3dCommandList, pCamera);
	//지형 객체가 포함하는 모든 메시를 렌더링한다. 
	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i]) m_ppMeshes[i]->Render(pd3dCommandList);
		}
	}
}

void CHeightMapTerrain::ReleaseUploadBuffers()
{
	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i]) m_ppMeshes[i]->ReleaseUploadBuffers();
		}
	}
}


