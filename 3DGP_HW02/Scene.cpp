//-----------------------------------------------------------------------------
// File: CScene.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Scene.h"
#include <algorithm>

CScene::CScene()
{
	
}

CScene::~CScene()
{
}

//#define _WITH_TEXT_MODEL_FILE
#define _WITH_BINARY_MODEL_FILE

void CScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	// 게임 상태 초기화
	m_bGameWon = false;
	m_fVictoryTimer = 0.0f;
	m_bVictoryMessageShown = false;

#ifdef _WITH_TEXT_MODEL_FILE
	CMesh *pUfoMesh = new CMesh(pd3dDevice, pd3dCommandList, "Models/UFO.txt", true);
	CMesh *pFlyerMesh = new CMesh(pd3dDevice, pd3dCommandList, "Models/FlyerPlayership.txt", true);
#endif
#ifdef _WITH_BINARY_MODEL_FILE
	CMesh *pUfoMesh = new CMesh(pd3dDevice, pd3dCommandList, "Models/UFO.bin", false);
	CMesh *pFlyerMesh = new CMesh(pd3dDevice, pd3dCommandList, "Models/FlyerPlayership.bin", false);

	// 총알용 메쉬 초기화 - 별도의 메쉬 생성
	m_pBulletMesh = new CMesh(pd3dDevice, pd3dCommandList, "Models/UFO.bin", false);
	m_pBulletMesh->AddRef();  // 참조 카운트 증가

	// 셰이더 생성
	CPseudoLightingShader *pShader = new CPseudoLightingShader();
	pShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	// 총알용 셰이더도 같은 셰이더 사용
	m_pBulletShader = pShader;
	m_pBulletShader->AddRef();  // 참조 카운트 증가

	// 디버그 메시지로 초기화 확인
	if (m_pBulletMesh && m_pBulletShader)
		OutputDebugString(L"Bullet Mesh and Shader initialized successfully!\n");
	else
		OutputDebugString(L"Failed to initialize Bullet Mesh or Shader!\n");
#endif

	// 지형 생성
	XMFLOAT3 xmf3Scale(8.0f, 2.0f, 8.0f);
	XMFLOAT4 xmf4Color(0.0f, 0.8f, 0.0f, 1.0f);  // 녹색 계열의 색상
	//지형을 하나의 큰 격자(257x257)로 생성한다. 
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList,
		m_pd3dGraphicsRootSignature, _T("HeightMap.raw"), 257, 257, 257,
		257, xmf3Scale, xmf4Color);

	m_nObjects = 20;  // UFO 20개 생성
	m_ppObjects = new CGameObject*[m_nObjects];

	// 난수 생성을 위한 시드 설정
	srand((unsigned)time(NULL));

	// UFO 생성
	for(int i = 0; i < m_nObjects; i++)
	{
		m_ppObjects[i] = new CGameObject();
		m_ppObjects[i]->SetMesh(pUfoMesh);
		m_ppObjects[i]->SetShader(pShader);

		// 랜덤 위치 설정 (더 넓은 범위로 수정)
		float x = ((float)rand() / RAND_MAX * 300.0f) - 150.0f;  // -150 ~ 150
		float y = ((float)rand() / RAND_MAX * 80.0f) - 40.0f;   // -40 ~ 40
		float z = ((float)rand() / RAND_MAX * 200.0f) - 50.0f;    // -50 ~ 150

		// 중앙 근처는 피하도록 조정 (플레이어 주변)
		if (abs(x) < 10.0f) x += (x < 0) ? -10.0f : 10.0f;
		if (abs(y) < 5.0f) y += (y < 0) ? -5.0f : 5.0f;
		if (z < 20.0f) z += 15.0f;

		m_ppObjects[i]->SetPosition(x, y, z);

		// 랜덤 색상 설정 (0.3 ~ 1.0 범위로 제한해서 너무 어둡지 않게)
		float r = ((float)rand() / RAND_MAX * 0.7f) + 0.3f;
		float g = ((float)rand() / RAND_MAX * 0.7f) + 0.3f;
		float b = ((float)rand() / RAND_MAX * 0.7f) + 0.3f;
		m_ppObjects[i]->SetColor(XMFLOAT3(r, g, b));
	}
}

ID3D12RootSignature *CScene::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
	ID3D12RootSignature *pd3dGraphicsRootSignature = NULL;

	D3D12_ROOT_PARAMETER pd3dRootParameters[3];
	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[0].Constants.Num32BitValues = 4; //Time, ElapsedTime, xCursor, yCursor
	pd3dRootParameters[0].Constants.ShaderRegister = 0; //Time
	pd3dRootParameters[0].Constants.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[1].Constants.Num32BitValues = 19; //16 + 3
	pd3dRootParameters[1].Constants.ShaderRegister = 1; //World
	pd3dRootParameters[1].Constants.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[2].Constants.Num32BitValues = 35; //16 + 16 + 3
	pd3dRootParameters[2].Constants.ShaderRegister = 2; //Camera
	pd3dRootParameters[2].Constants.RegisterSpace = 0;
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 0;
	d3dRootSignatureDesc.pStaticSamplers = NULL;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void **)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

void CScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();

	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) delete m_ppObjects[j];
		delete[] m_ppObjects;
	}

	// 총알 리소스 해제
	if (m_pBulletMesh) m_pBulletMesh->Release();
	if (m_pBulletShader) m_pBulletShader->Release();

	// 지형 리소스 해제
	if (m_pTerrain) delete m_pTerrain;
}

void CScene::ReleaseUploadBuffers()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->ReleaseUploadBuffers();
	}
	
	// 지형 업로드 버퍼 해제
	if (m_pTerrain) m_pTerrain->ReleaseUploadBuffers();
}

bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}

bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		break;
	default:
		break;
	}
	return(false);
}

bool CScene::ProcessInput()
{
	return(false);
}

void CScene::AnimateObjects(float fTimeElapsed)
{
	// 충돌로 생성될 새로운 객체들을 임시 저장할 벡터
	std::vector<CGameObject*> newObjects;
	std::vector<int> objectsToRemove;  // 제거할 객체들의 인덱스
	
	// UFO 카운트 변수 추가
	int nUfoCount = 0;
	
	// 플레이어 위치 가져오기
	XMFLOAT3 playerPosition;
	XMFLOAT3 playerPosition = m_pPlayer->GetPosition();

	for (int j = 0; j < m_nObjects; j++)
	{
		// UFO인지 확인 (총알이나 파편이 아닌 객체)
		if (!dynamic_cast<CBulletObject*>(m_ppObjects[j]) && !m_ppObjects[j]->IsSplitUFO())
		{
			XMFLOAT3 ufoPosition = m_ppObjects[j]->GetPosition();
			XMFLOAT3 direction = Vector3::Subtract(playerPosition, ufoPosition);
			direction = Vector3::Normalize(direction);

			// 속도를 조절하여 서서히 이동
			float speed = 8.0f * fTimeElapsed; // 속도 조절
			XMFLOAT3 velocity = Vector3::ScalarProduct(direction, speed);
			m_ppObjects[j]->SetPosition(Vector3::Add(ufoPosition, velocity));

			nUfoCount++;
		}

		m_ppObjects[j]->Animate(fTimeElapsed);

		// 파편(SplitedUFO) 객체인 경우 수명 체크
		CSplitedUFO* pSplitUFO = dynamic_cast<CSplitedUFO*>(m_ppObjects[j]);
		if (pSplitUFO && pSplitUFO->IsExpired())
		{
			objectsToRemove.push_back(j);
			continue;
		}

		// CBulletObject인 경우 충돌 체크 및 거리 체크
		CBulletObject* pBullet = dynamic_cast<CBulletObject*>(m_ppObjects[j]);
		if (pBullet)
		{
			// 총알의 위치
			XMFLOAT3 bulletPos = pBullet->GetPosition();
			
			// 모든 UFO와 충돌 체크
			for (int i = 0; i < m_nObjects; i++)
			{
				if (i != j)  // 자기 자신은 제외
				{
					// UFO인지 확인 (총알이나 파편이 아닌 객체는 모두 UFO로 가정)
					if (!dynamic_cast<CBulletObject*>(m_ppObjects[i]) && !m_ppObjects[i]->IsSplitUFO())
					{
						XMFLOAT3 ufoPos = m_ppObjects[i]->GetPosition();
						
						// 간단한 거리 기반 충돌 체크 (임시로 5.0f를 충돌 범위로 설정)
						float dx = bulletPos.x - ufoPos.x;
						float dy = bulletPos.y - ufoPos.y;
						float dz = bulletPos.z - ufoPos.z;
						float distance = sqrt(dx*dx + dy*dy + dz*dz);
						
						if (distance < 5.0f)  // 충돌 발생
						{
							WCHAR szDebugString[256];
							wsprintf(szDebugString, L"Collision Detected!");
							OutputDebugString(szDebugString);
							
							// 충돌한 UFO를 50개의 작은 UFO로 분열
							for (int k = 0; k < 50; k++)
							{
								CSplitedUFO* pNewUfo = new CSplitedUFO();
								pNewUfo->SetMesh(m_ppObjects[i]->GetMesh());
								pNewUfo->SetShader(m_ppObjects[i]->GetShader());
								
								// 원래 위치에서 약간씩 랜덤하게 위치 설정
								float offsetX = ((float)rand() / RAND_MAX - 0.5f) * 2.0f;
								float offsetY = ((float)rand() / RAND_MAX - 0.5f) * 2.0f;
								float offsetZ = ((float)rand() / RAND_MAX - 0.5f) * 2.0f;
								
								pNewUfo->SetPosition(ufoPos.x + offsetX, ufoPos.y + offsetY, ufoPos.z + offsetZ);
								pNewUfo->SetScale(0.1f, 0.1f, 0.1f);  // 크기를 0.1배로
								
								// 원래 UFO의 색상을 그대로 사용
								pNewUfo->SetColor(m_ppObjects[i]->GetColor());
								
								// 랜덤한 방향으로 날아가도록 속도 설정
								float speedX = ((float)rand() / RAND_MAX - 0.5f) * 30.0f;  // -10 ~ 10 속도
								float speedY = ((float)rand() / RAND_MAX - 0.5f) * 30.0f;
								float speedZ = ((float)rand() / RAND_MAX - 0.5f) * 30.0f;
								pNewUfo->SetVelocity(XMFLOAT3(speedX, speedY, speedZ));
								
								newObjects.push_back(pNewUfo);
							}
							
							// 충돌한 UFO와 총알을 제거 목록에 추가
							objectsToRemove.push_back(i);
							objectsToRemove.push_back(j);
							
							// 한 총알이 여러 UFO와 충돌하지 않도록 break
							break;
						}
					}
				}
			}

			// 기존의 거리 체크 로직
			if (pBullet->IsOutOfRange())
			{
				objectsToRemove.push_back(j);
			}
		}
	}
	
	// 중복 제거를 위해 정렬하고 유니크하게 만들기
	std::sort(objectsToRemove.begin(), objectsToRemove.end());
	objectsToRemove.erase(std::unique(objectsToRemove.begin(), objectsToRemove.end()), objectsToRemove.end());
	
	// 뒤에서부터 제거 (인덱스가 변경되지 않도록)
	for (int i = objectsToRemove.size() - 1; i >= 0; i--)
	{
		int idx = objectsToRemove[i];
		delete m_ppObjects[idx];
		m_ppObjects[idx] = m_ppObjects[m_nObjects - 1];
		m_ppObjects[m_nObjects - 1] = nullptr;
		m_nObjects--;
	}
	
	// 새로운 객체들 추가
	if (newObjects.size() > 0)
	{
		CGameObject** ppNewObjects = new CGameObject*[m_nObjects + newObjects.size()];
		
		// 기존 객체들 복사
		for (int i = 0; i < m_nObjects; i++)
		{
			ppNewObjects[i] = m_ppObjects[i];
		}
		
		// 새로운 객체들 추가
		for (int i = 0; i < newObjects.size(); i++)
		{
			ppNewObjects[m_nObjects + i] = newObjects[i];
		}
		
		// 기존 배열 삭제하고 새 배열로 교체
		delete[] m_ppObjects;
		m_ppObjects = ppNewObjects;
		m_nObjects += newObjects.size();
	}

	// UFO가 모두 사라졌는지 체크 (nUfoCount가 0이면 승리)
	if (nUfoCount == 0 && !m_bGameWon)
	{
		m_bGameWon = true;  // 승리 상태 설정
	}

	// 승리 후 타이머 업데이트 및 메시지 박스 표시
	if (m_bGameWon && !m_bVictoryMessageShown)
	{
		m_fVictoryTimer += fTimeElapsed;
		if (m_fVictoryTimer >= 1.0f)  // 2초 후에 메시지 박스 표시
		{
			MessageBox(NULL, L"YOU WIN!", L"Victory", MB_OK);
			m_bVictoryMessageShown = true;
		}
	}
}

void CScene::PrepareRender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
}

void CScene::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	// 지형 먼저 렌더링
	if (m_pTerrain) m_pTerrain->Render(pd3dCommandList, pCamera);

	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j]) m_ppObjects[j]->Render(pd3dCommandList, pCamera);
	}
}

void CScene::CreateBullet()
{
	// 디버깅 메시지 출력
	WCHAR szDebugString[256];
	wsprintf(szDebugString, L"CreateBullet Function Called! Now Objects: %d\n", m_nObjects);
	OutputDebugString(szDebugString);

	// 1. 새로운 크기의 오브젝트 배열 생성
	CGameObject** ppNewObjects = new CGameObject*[m_nObjects + 1];

	// 2. 기존 오브젝트들을 새 배열로 복사
	for (int i = 0; i < m_nObjects; i++)
	{
		ppNewObjects[i] = m_ppObjects[i];
	}

	// 3. 새로운 총알 오브젝트 생성
	CBulletObject* pBulletObject = new CBulletObject();
	
	// 4. 플레이어의 위치와 방향을 가져와서 총알 위치 설정
	XMFLOAT3 xmf3PlayerPosition;
	XMFLOAT3 xmf3PlayerLook = m_pPlayer->GetLookVector();
	if (m_pPlayer)
	{
		xmf3PlayerPosition = m_pPlayer->GetPosition();
		
		
		// 플레이어 앞쪽으로 총알 생성 (Look 벡터 방향으로 3.0f 만큼)
		xmf3PlayerPosition.x += xmf3PlayerLook.x * 3.0f;
		xmf3PlayerPosition.y += xmf3PlayerLook.y * 3.0f -0.5f;
		xmf3PlayerPosition.z += xmf3PlayerLook.z * 3.0f;

	}
	else
	{
		OutputDebugString(L"No Player Reference!\n");
		xmf3PlayerPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);
	}
	
	if (m_pBulletMesh && m_pBulletShader)  // 메쉬와 셰이더가 유효한지 확인
	{
		pBulletObject->SetMesh(m_pBulletMesh);
		pBulletObject->SetShader(m_pBulletShader);
		pBulletObject->SetPosition(xmf3PlayerPosition);
		pBulletObject->SetScale(0.1f, 0.4f, 0.1f);  // 크기를 절반으로
		pBulletObject->SetColor(XMFLOAT3(1.0f, 1.0f, 0.0f));  // 노란색
		
		// Look 벡터를 총알의 속도로 설정
		pBulletObject->SetVelocity(xmf3PlayerLook);
		
		// 5. 새로운 총알을 배열의 마지막에 추가
		ppNewObjects[m_nObjects] = pBulletObject;
		
		// 6. 기존 배열 삭제하고 새 배열로 교체
		if (m_ppObjects) delete[] m_ppObjects;
		m_ppObjects = ppNewObjects;
		m_nObjects++;
		
		OutputDebugString(L"Bullet Created!\n");
	}
	else
	{
		OutputDebugString(L"No Mesh or Shader!\n");
		delete pBulletObject;  // 실패하면 메모리 해제
		delete[] ppNewObjects;
	}
}

void CScene::ExplodeAllUFOs()
{
	// 새로운 오브젝트들을 저장할 벡터
	std::vector<CGameObject*> newObjects;
	std::vector<int> objectsToRemove;

	// 모든 오브젝트를 순회하면서 UFO 찾기
	for (int i = 0; i < m_nObjects; i++)
	{
		// UFO인지 확인 (총알이나 파편이 아닌 객체)
		if (!dynamic_cast<CBulletObject*>(m_ppObjects[i]) && !m_ppObjects[i]->IsSplitUFO())
		{
			XMFLOAT3 ufoPos = m_ppObjects[i]->GetPosition();
			
			// 50개의 파편 생성
			for (int k = 0; k < 50; k++)
			{
				CSplitedUFO* pNewUfo = new CSplitedUFO();
				pNewUfo->SetMesh(m_ppObjects[i]->GetMesh());
				pNewUfo->SetShader(m_ppObjects[i]->GetShader());
				
				// 원래 위치에서 약간씩 랜덤하게 위치 설정
				float offsetX = ((float)rand() / RAND_MAX - 0.5f) * 2.0f;
				float offsetY = ((float)rand() / RAND_MAX - 0.5f) * 2.0f;
				float offsetZ = ((float)rand() / RAND_MAX - 0.5f) * 2.0f;
				
				pNewUfo->SetPosition(ufoPos.x + offsetX, ufoPos.y + offsetY, ufoPos.z + offsetZ);
				pNewUfo->SetScale(0.1f, 0.1f, 0.1f);  // 크기를 0.1배로
				
				// 원래 UFO의 색상을 그대로 사용
				pNewUfo->SetColor(m_ppObjects[i]->GetColor());
				
				// 랜덤한 방향으로 날아가도록 속도 설정
				float speedX = ((float)rand() / RAND_MAX - 0.5f) * 30.0f;
				float speedY = ((float)rand() / RAND_MAX - 0.5f) * 30.0f;
				float speedZ = ((float)rand() / RAND_MAX - 0.5f) * 30.0f;
				pNewUfo->SetVelocity(XMFLOAT3(speedX, speedY, speedZ));
				
				newObjects.push_back(pNewUfo);
			}
			
			// UFO를 제거 목록에 추가
			objectsToRemove.push_back(i);
		}
	}

	// 중복 제거를 위해 정렬하고 유니크하게 만들기
	std::sort(objectsToRemove.begin(), objectsToRemove.end());
	objectsToRemove.erase(std::unique(objectsToRemove.begin(), objectsToRemove.end()), objectsToRemove.end());

	// 새로운 크기의 오브젝트 배열 생성
	int nNewObjects = m_nObjects - objectsToRemove.size() + newObjects.size();
	CGameObject** ppNewObjects = new CGameObject*[nNewObjects];

	// 제거되지 않는 기존 오브젝트들 복사
	int currentIndex = 0;
	for (int i = 0; i < m_nObjects; i++)
	{
		if (std::find(objectsToRemove.begin(), objectsToRemove.end(), i) == objectsToRemove.end())
		{
			ppNewObjects[currentIndex++] = m_ppObjects[i];
		}
		else
		{
			delete m_ppObjects[i];  // 제거될 오브젝트 메모리 해제
		}
	}

	// 새로운 파편 오브젝트들 추가
	for (auto& newObj : newObjects)
	{
		ppNewObjects[currentIndex++] = newObj;
	}

	// 기존 배열 삭제하고 새 배열로 교체
	delete[] m_ppObjects;
	m_ppObjects = ppNewObjects;
	m_nObjects = nNewObjects;
}

