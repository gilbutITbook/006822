//------------------------------------------------------------
// 3DPhysics_3_1.cpp
// 지형을 따라 물체가 운동한다
// 
//------------------------------------------------------------

#include <D3DX11.h>
#include <D3Dcompiler.h>
#include <xnamath.h>

#define VIEW_WIDTH					800					// 화면너비
#define VIEW_HEIGHT					600					// 화면높이

#define PI							3.1415927f			// 원주율
#define ROT_SPEED					( PI / 100.0f )		// 회전속도
#define CORNER_NUM					20					// 각수
#define PLAYER_SPEED				0.15f				// 플레이어의 이동속도
#define PLAYER_ANG_SPEED			( PI / 100.0f )		// 플레이어의 회전속도
#define GROUND_SIZE					30.0f				// 바닥 크기
#define GROUND_DIVIDE_NUM			50					// 바닥 분할수
#define HEIGHT_NUM					( GROUND_DIVIDE_NUM + 1	)	// 높이 데이터수
#define BLOCK_NUM				( GROUND_DIVIDE_NUM )	// 지형 블록수
#define BLOCK_WIDTH				( GROUND_SIZE / BLOCK_NUM )		// 블록너비
#define GR							0.002f				// 중력가속도
#define G_CYLINDER_R				10.0f				// 지면 원기둥 반지름
#define G_SPHERE_R					10.0f				// 지면 구 반지름
#define G_SPHERE_Z_POS				5.0f				// 지면 구 Z 위치


// 정점구조체
struct CUSTOMVERTEX {
    XMFLOAT4	v4Pos;
	XMFLOAT2	v2UV;
};


struct MY_PLAYER {
	XMFLOAT3			v3Pos;					// 위치
	XMFLOAT3			v3Vel;					// 속도
	float				fAngleXZ;				// xz평면에서 향하는 각도
	XMFLOAT3			v3Normal;				// 법선벡터
	float				fEnergy;				// 전에너지
};

MY_PLAYER	Player_1;							// 플레이어 데이터


// 높이 데이터
float			g_fHeights[HEIGHT_NUM][HEIGHT_NUM];
// 법선 벡터
XMFLOAT3		g_v3Normal[HEIGHT_NUM][HEIGHT_NUM];


int				g_nHitTriangleIndex;						// 충돌 삼각형 인덱스


float CheckGroundHeight( MY_PLAYER *pPlayer );									// 지면 높이 체크


XMFLOAT3 Subtract( XMFLOAT3 *pv3Vec1, XMFLOAT3 *pv3Vec2 )						// 벡터의 뺄셈
{
	return XMFLOAT3( pv3Vec1->x - pv3Vec2->x,
					 pv3Vec1->y - pv3Vec2->y,
					 pv3Vec1->z - pv3Vec2->z );
}


XMFLOAT3 Normalize( XMFLOAT3 *pv3Vec )											// 벡터의 단위화
{
	float		fLength;

	fLength = sqrtf( pv3Vec->x * pv3Vec->x +
					 pv3Vec->y * pv3Vec->y +
					 pv3Vec->z * pv3Vec->z );

	return XMFLOAT3( pv3Vec->x / fLength, pv3Vec->y / fLength, pv3Vec->z / fLength );
}


XMFLOAT3 Cross( XMFLOAT3 *pv3Vec1, XMFLOAT3 *pv3Vec2 )							// 벡터의 외적
{
	return XMFLOAT3( pv3Vec1->y * pv3Vec2->z - pv3Vec1->z * pv3Vec2->y,
					 pv3Vec1->z * pv3Vec2->x - pv3Vec1->x * pv3Vec2->z,
					 pv3Vec1->x * pv3Vec2->y - pv3Vec1->y * pv3Vec2->x );
}


int InitPlayer( void )									// 플레이어 초기화
{
	float			fVel;								// 길이

	// 플레이어1
	Player_1.v3Pos = XMFLOAT3( G_CYLINDER_R - 3.0f, 0.0f, -4.0f );
	Player_1.v3Pos.y = CheckGroundHeight( &Player_1 );	// 플레이어 높이 설정
	Player_1.v3Vel = XMFLOAT3( 0.0f, 0.0f, 0.10f );
	Player_1.fAngleXZ = 0.0f;
	Player_1.v3Normal = XMFLOAT3( 0.0f, 1.0f, 0.0f );
	Player_1.fEnergy = GR * Player_1.v3Pos.y;
	fVel = sqrtf( Player_1.v3Vel.x * Player_1.v3Vel.x +
				  Player_1.v3Vel.y * Player_1.v3Vel.y +
				  Player_1.v3Vel.z * Player_1.v3Vel.z );
	Player_1.fEnergy += 0.5f * fVel * fVel;

	return 0;
}


// 물체의 이동
int MovePlayer( void )
{
	XMFLOAT3		v3Drop, v3Side, v3Up;
	XMFLOAT3		v3Vel;
	float			fNowKineticE;						// 현재 운동 에너지
	float			fNowVel;							// 현재 속도

	// 왼쪽으로 회전
	if ( GetAsyncKeyState( VK_LEFT ) ) {
		Player_1.fAngleXZ += PLAYER_ANG_SPEED;
		if ( Player_1.fAngleXZ > ( 2.0f * PI ) )
			Player_1.fAngleXZ -= 2.0f * PI;
	}
	// 오른쪽으로 회전
	if ( GetAsyncKeyState( VK_RIGHT ) ) {
		Player_1.fAngleXZ -= PLAYER_ANG_SPEED;
		if ( Player_1.fAngleXZ < ( 2.0f * PI ) )
			Player_1.fAngleXZ += 2.0f * PI;
	}

	// 위치에 속도를 더한다
	Player_1.v3Pos.x += Player_1.v3Vel.x;
	Player_1.v3Pos.y += Player_1.v3Vel.y;
	Player_1.v3Pos.z += Player_1.v3Vel.z;

	// 밀려난 부분 처리(되돌림)
	if ( Player_1.v3Pos.x > GROUND_SIZE / 2 ) {
		Player_1.v3Pos.x = GROUND_SIZE / 2;
		Player_1.v3Vel.x = -Player_1.v3Vel.x;
	}
	if ( Player_1.v3Pos.x < -GROUND_SIZE / 2 ) {
		Player_1.v3Pos.x = -GROUND_SIZE / 2;
		Player_1.v3Vel.x = -Player_1.v3Vel.x;
	}
	if ( Player_1.v3Pos.z > GROUND_SIZE / 2 ) {
		Player_1.v3Pos.z = GROUND_SIZE / 2;
		Player_1.v3Vel.z = -Player_1.v3Vel.z;
	}
	if ( Player_1.v3Pos.z < -GROUND_SIZE / 2 ) {
		Player_1.v3Pos.z = -GROUND_SIZE / 2;
		Player_1.v3Vel.z = -Player_1.v3Vel.z;
	}

	Player_1.v3Pos.y = CheckGroundHeight( &Player_1 );	// 플레이어 높이 설정

	// 가속도 계산
	v3Up = Player_1.v3Normal;
	v3Side = XMFLOAT3( v3Up.z, 0.0f, -v3Up.x );		// 사이드벡터
//	v3Side = Cross( &v3Up, &XMFLOAT3( 0.0f, -1.0f, 0.0f ) );
	v3Drop = Cross( &v3Side, &v3Up );
	Player_1.v3Vel.x += GR * v3Drop.x;
	Player_1.v3Vel.y += GR * v3Drop.y;
	Player_1.v3Vel.z += GR * v3Drop.z;
	// 속도직교화보정
	v3Up = Player_1.v3Normal;
	v3Vel = Player_1.v3Vel;
	v3Side = Cross( &v3Vel, &v3Up );
	v3Vel = Cross( &v3Up, &v3Side );
	v3Vel = Normalize( &v3Vel );
	// 운동에너지보정
	fNowKineticE = Player_1.fEnergy - GR * Player_1.v3Pos.y;
	if ( fNowKineticE > 0.0f ) {				// 일반 에너지 계산
		fNowVel = sqrtf( 2.0f * fNowKineticE );
		Player_1.v3Vel.x = fNowVel * v3Vel.x;
		Player_1.v3Vel.y = fNowVel * v3Vel.y;
		Player_1.v3Vel.z = fNowVel * v3Vel.z;
	}
	else {										// 오차로 정지할 경우, 속도=가속도
		Player_1.v3Vel.x = GR * v3Drop.x;
		Player_1.v3Vel.y = GR * v3Drop.y;
		Player_1.v3Vel.z = GR * v3Drop.z;
	}

	return 0;
}


int MakeGroundNormals( CUSTOMVERTEX *pVertices, WORD *pIndices, int nIndexNum )
{
	int					i, j;
	int					nTriangleNum;
	CUSTOMVERTEX		p1, p2, p3;
	XMFLOAT3			v1, v2;
	XMFLOAT3			v3Normal;
	int					nIndex1, nIndex2, nIndex3;

	for ( i = 0; i < HEIGHT_NUM; i++ ) {
		for ( j = 0; j < HEIGHT_NUM; j++ ) {
			g_v3Normal[i][j] = XMFLOAT3( 0.0f, 0.0f, 0.0f );
		}
	}

	nTriangleNum = 0;
	for ( i = 0; i < nIndexNum; i += 3 ) {
		nIndex1 = pIndices[i    ];
		nIndex2 = pIndices[i + 1];
		nIndex3 = pIndices[i + 2];
		p1 = pVertices[nIndex1];
		p2 = pVertices[nIndex2];
		p3 = pVertices[nIndex3];
		v1 = XMFLOAT3( p3.v4Pos.x - p1.v4Pos.x, p3.v4Pos.y - p1.v4Pos.y, p3.v4Pos.z - p1.v4Pos.z );
		v2 = XMFLOAT3( p2.v4Pos.x - p1.v4Pos.x, p2.v4Pos.y - p1.v4Pos.y, p2.v4Pos.z - p1.v4Pos.z );
		v3Normal = Cross( &v1, &v2 );
		v3Normal = Normalize( &v3Normal );
		g_v3Normal[nIndex1 / HEIGHT_NUM][nIndex1 % HEIGHT_NUM].x += v3Normal.x;
		g_v3Normal[nIndex1 / HEIGHT_NUM][nIndex1 % HEIGHT_NUM].y += v3Normal.y;
		g_v3Normal[nIndex1 / HEIGHT_NUM][nIndex1 % HEIGHT_NUM].z += v3Normal.z;
		g_v3Normal[nIndex2 / HEIGHT_NUM][nIndex2 % HEIGHT_NUM].x += v3Normal.x;
		g_v3Normal[nIndex2 / HEIGHT_NUM][nIndex2 % HEIGHT_NUM].y += v3Normal.y;
		g_v3Normal[nIndex2 / HEIGHT_NUM][nIndex2 % HEIGHT_NUM].z += v3Normal.z;
		g_v3Normal[nIndex3 / HEIGHT_NUM][nIndex3 % HEIGHT_NUM].x += v3Normal.x;
		g_v3Normal[nIndex3 / HEIGHT_NUM][nIndex3 % HEIGHT_NUM].y += v3Normal.y;
		g_v3Normal[nIndex3 / HEIGHT_NUM][nIndex3 % HEIGHT_NUM].z += v3Normal.z;
	}

	for ( i = 0; i < HEIGHT_NUM; i++ ) {
		for ( j = 0; j < HEIGHT_NUM; j++ ) {
			g_v3Normal[i][j] = Normalize( &( g_v3Normal[i][j] ) );
		}
	}

	return 0;
}


float CheckGroundHeight( MY_PLAYER *pPlayer )									// 지면의 높이 체크
{
	float		fPlayerBlock_x, fPlayerBlock_z;
	int			nBlockIndex_x, nBlockIndex_z;
	float		fBlockRes_x, fBlockRes_z;
	float		fGrad_x, fGrad_z;					// 기울기
	int			nInIndex;
	float		fHeight;
	XMFLOAT3	v3Grad1, v3Grad2;

	fPlayerBlock_x = ( pPlayer->v3Pos.x + ( GROUND_SIZE / 2 ) ) / ( float )BLOCK_WIDTH;
	nBlockIndex_x = ( int )fPlayerBlock_x;
	if ( nBlockIndex_x < 0 ) nBlockIndex_x = 0;
	if ( nBlockIndex_x > BLOCK_NUM - 1 ) nBlockIndex_x = BLOCK_NUM - 1;
	fBlockRes_x = fPlayerBlock_x - nBlockIndex_x;
	fPlayerBlock_z = ( pPlayer->v3Pos.z + ( GROUND_SIZE / 2 ) ) / ( float )BLOCK_WIDTH;
	nBlockIndex_z = ( int )fPlayerBlock_z;
	if ( nBlockIndex_z < 0 ) nBlockIndex_z = 0;
	if ( nBlockIndex_z > BLOCK_NUM - 1 ) nBlockIndex_z = BLOCK_NUM - 1;
	fBlockRes_z = fPlayerBlock_z - nBlockIndex_z;

	if ( ( fBlockRes_x + fBlockRes_z ) < 1.0f ) {
		fGrad_x = ( g_fHeights[nBlockIndex_z][nBlockIndex_x + 1] - g_fHeights[nBlockIndex_z][nBlockIndex_x] );
		fGrad_z = ( g_fHeights[nBlockIndex_z + 1][nBlockIndex_x] - g_fHeights[nBlockIndex_z][nBlockIndex_x] );
		fHeight = fGrad_x * fBlockRes_x + fGrad_z * fBlockRes_z + g_fHeights[nBlockIndex_z][nBlockIndex_x];
		nInIndex = 0;
		v3Grad1 = Subtract( &g_v3Normal[nBlockIndex_z][nBlockIndex_x + 1],
							&g_v3Normal[nBlockIndex_z][nBlockIndex_x] );
		v3Grad2 = Subtract( &g_v3Normal[nBlockIndex_z + 1][nBlockIndex_x],
							&g_v3Normal[nBlockIndex_z][nBlockIndex_x] );
		pPlayer->v3Normal.x = v3Grad1.x * fBlockRes_x + v3Grad2.x * fBlockRes_z
							  + g_v3Normal[nBlockIndex_z][nBlockIndex_x].x;
		pPlayer->v3Normal.y = v3Grad1.y * fBlockRes_x + v3Grad2.y * fBlockRes_z
							  + g_v3Normal[nBlockIndex_z][nBlockIndex_x].y;
		pPlayer->v3Normal.z = v3Grad1.z * fBlockRes_x + v3Grad2.z * fBlockRes_z
							  + g_v3Normal[nBlockIndex_z][nBlockIndex_x].z;
	}
	else {
		fGrad_x = ( g_fHeights[nBlockIndex_z + 1][nBlockIndex_x] - g_fHeights[nBlockIndex_z + 1][nBlockIndex_x + 1] );
		fGrad_z = ( g_fHeights[nBlockIndex_z][nBlockIndex_x + 1] - g_fHeights[nBlockIndex_z + 1][nBlockIndex_x + 1] );
		fHeight = fGrad_x * ( 1.0f - fBlockRes_x ) + fGrad_z * ( 1.0f - fBlockRes_z )
				  + g_fHeights[nBlockIndex_z + 1][nBlockIndex_x + 1];
		nInIndex = 1;
		v3Grad1 = Subtract( &g_v3Normal[nBlockIndex_z + 1][nBlockIndex_x],
							&g_v3Normal[nBlockIndex_z + 1][nBlockIndex_x + 1] );
		v3Grad2 = Subtract( &g_v3Normal[nBlockIndex_z][nBlockIndex_x + 1],
							&g_v3Normal[nBlockIndex_z + 1][nBlockIndex_x + 1] );
		pPlayer->v3Normal.x = v3Grad1.x * ( 1.0f - fBlockRes_x ) + v3Grad2.x * ( 1.0f - fBlockRes_z )
							  + g_v3Normal[nBlockIndex_z + 1][nBlockIndex_x + 1].x;
		pPlayer->v3Normal.y = v3Grad1.y * ( 1.0f - fBlockRes_x ) + v3Grad2.y * ( 1.0f - fBlockRes_z )
							  + g_v3Normal[nBlockIndex_z + 1][nBlockIndex_x + 1].y;
		pPlayer->v3Normal.z = v3Grad1.z * ( 1.0f - fBlockRes_x ) + v3Grad2.z * ( 1.0f - fBlockRes_z )
							  + g_v3Normal[nBlockIndex_z + 1][nBlockIndex_x + 1].z;
	}
	pPlayer->v3Normal = Normalize( &( pPlayer->v3Normal ) );

	g_nHitTriangleIndex = nBlockIndex_z * GROUND_DIVIDE_NUM * 2 + nBlockIndex_x * 2 + nInIndex;

	return fHeight;
}



XMMATRIX CreateWorldMatrix( MY_PLAYER *pPlayer )		// 월드행렬 생성
{
	static float	fAngleX = 0.0f;						// x축 회전각
	XMMATRIX		matRot_Y;							// y축 회전행렬
	XMMATRIX		matRot_X;							// x축 회전행렬
	XMMATRIX		matScaleTrans;						// 확대축소평행이동행렬
	XMFLOAT3		v3PreForward;
	XMMATRIX		matResult;
	XMFLOAT3		v3Foward, v3Side, v3Up;

	v3Up = pPlayer->v3Normal;
	v3Foward = XMFLOAT3( cosf( pPlayer->fAngleXZ ), 0.0f, sinf( pPlayer->fAngleXZ ) );
	v3Side = Cross( &v3Foward, &v3Up );
	v3Side = Normalize( &v3Side );
	v3Foward = Cross( &v3Up, &v3Side );
	v3Foward = Normalize( &v3Foward );
	matResult = XMMatrixIdentity();				// 단위행렬로
	matResult._11 = v3Foward.x;
	matResult._12 = v3Foward.y;
	matResult._13 = v3Foward.z;
	matResult._21 = v3Up.x;
	matResult._22 = v3Up.y;
	matResult._23 = v3Up.z;
	matResult._31 = v3Side.x;
	matResult._32 = v3Side.y;
	matResult._33 = v3Side.z;
	matResult._41 = pPlayer->v3Pos.x;
	matResult._42 = pPlayer->v3Pos.y;
	matResult._43 = pPlayer->v3Pos.z;

	return matResult;									// 결과
}


// 직육면체 만들기(인덱스)
int Make3DRectangleIndexed( float fWidth, float fHeight, float fDepth, int bCenterMode,
							CUSTOMVERTEX *pVertices, int *pVertexNum,
							WORD *pIndices, int *pIndexNum,
							int nIndexOffset )
{
	int					i;
	int					nIndex;						// 데이터의 인덱스
	int					nUpIndex2;					// 지면 2번째 인덱스
	float				fHeight1, fHeight2;

	// 중심위치 모드
	if ( bCenterMode ) {
		fHeight1 =  fHeight / 2.0f;
		fHeight2 = -fHeight / 2.0f;
	}
	else {
		fHeight1 = fHeight;
		fHeight2 = 0.0f;
	}

	// 정점 데이터 작성
	// 윗면 
	pVertices[0].v4Pos  = XMFLOAT4( -fWidth / 2.0f, fHeight1, -fDepth / 2.0f, 1.0f );
	pVertices[0].v2UV = XMFLOAT2( 1.0f / 3.0f, 1.0f / 3.0f );
	pVertices[1].v4Pos  = XMFLOAT4(  fWidth / 2.0f,	fHeight1, -fDepth / 2.0f, 1.0f );
	pVertices[1].v2UV = XMFLOAT2( 1.0f / 3.0f, 2.0f / 3.0f );
	pVertices[2].v4Pos  = XMFLOAT4(  fWidth / 2.0f,	fHeight1,  fDepth / 2.0f, 1.0f );
	pVertices[2].v2UV = XMFLOAT2( 2.0f / 3.0f, 2.0f / 3.0f );
	pVertices[3].v4Pos  = XMFLOAT4( -fWidth / 2.0f,	fHeight1,  fDepth / 2.0f, 1.0f );
	pVertices[3].v2UV = XMFLOAT2( 2.0f / 3.0f, 1.0f / 3.0f );

	// 옆면용 아랫면(uv의 차이로 1군데당 2개씩)
	pVertices[4] = pVertices[0];
	pVertices[4].v2UV = XMFLOAT2( 1.0f / 3.0f, 0.0f / 3.0f );
	pVertices[5] = pVertices[0];
	pVertices[5].v2UV = XMFLOAT2( 0.0f / 3.0f, 1.0f / 3.0f );
	pVertices[6] = pVertices[1];
	pVertices[6].v2UV = XMFLOAT2( 0.0f / 3.0f, 2.0f / 3.0f );
	pVertices[7] = pVertices[1];
	pVertices[7].v2UV = XMFLOAT2( 1.0f / 3.0f, 3.0f / 3.0f );
	pVertices[8] = pVertices[2];
	pVertices[8].v2UV = XMFLOAT2( 2.0f / 3.0f, 3.0f / 3.0f );
	pVertices[9] = pVertices[2];
	pVertices[9].v2UV = XMFLOAT2( 3.0f / 3.0f, 2.0f / 3.0f );
	pVertices[10] = pVertices[3];
	pVertices[10].v2UV = XMFLOAT2( 3.0f / 3.0f, 1.0f / 3.0f );
	pVertices[11] = pVertices[3];
	pVertices[11].v2UV = XMFLOAT2( 2.0f / 3.0f, 0.0f / 3.0f );
	for ( i = 4; i < 12; i++ ) {
		pVertices[i].v4Pos.y = fHeight2;
	}

	*pVertexNum = 12;

	// 인덱스 데이터 작성
	// 윗면
	nIndex = 0;
	pIndices[nIndex    ] = nIndexOffset + 0;
	pIndices[nIndex + 1] = nIndexOffset + 1;
	pIndices[nIndex + 2] = nIndexOffset + 2;
	nIndex += 3;
	pIndices[nIndex    ] = nIndexOffset + 2;
	pIndices[nIndex + 1] = nIndexOffset + 3;
	pIndices[nIndex + 2] = nIndexOffset + 0;
	nIndex += 3;

	// 옆면
	for ( i = 0; i < 4; i++ ) {
		nUpIndex2 = ( i + 1 ) % 4;
		pIndices[nIndex    ] = nIndexOffset + i;
		pIndices[nIndex + 1] = nIndexOffset + 4 + ( i * 2 + 1 ) % 8;
		pIndices[nIndex + 2] = nIndexOffset + nUpIndex2;
		nIndex += 3;
		pIndices[nIndex    ] = nIndexOffset + 4 + ( i * 2 + 1 ) % 8;
		pIndices[nIndex + 1] = nIndexOffset + 4 + ( i * 2 + 1 + 1 ) % 8;
		pIndices[nIndex + 2] = nIndexOffset + nUpIndex2;
		nIndex += 3;
	}
	*pIndexNum = nIndex;

	return 0;
}


//------------------------------------------------------------
// 아래는 DirectX로 표시하는 프로그램

#include <stdio.h>
#include <windows.h>
#include <tchar.h>								// Unicode・멀티바이트 문자관계


#define MAX_BUFFER_VERTEX				10000	// 최대버퍼정점수
#define MAX_BUFFER_INDEX				20000	// 최대버퍼인덱스수
#define MAX_MODEL_NUM					100		// 최대모델수


// 링크 라이브러리
#pragma comment( lib, "d3d11.lib" )   // D3D11 라이브러리
#pragma comment( lib, "d3dx11.lib" )
#pragma comment( lib, "winmm.lib" )


// 세이프 릴리스 매크로
#ifndef SAFE_RELEASE
#define SAFE_RELEASE( p )      { if ( p ) { ( p )->Release(); ( p )=NULL; } }
#endif

// 셰이더 상수 구조체
struct CBNeverChanges
{
    XMMATRIX mView;
	XMFLOAT4 v4AddColor;
};

// 텍스처 이미지 구조체
struct TEX_PICTURE {
	ID3D11ShaderResourceView	*pSRViewTexture;
	D3D11_TEXTURE2D_DESC		tdDesc;
	int							nWidth, nHeight;
};

// 모델 구조체
struct MY_MODEL {
	int					nVertexPos;						// 정점위치
	int					nVertexNum;						// 정점수
	int					nIndexPos;						// 인덱스위치
	int					nIndexNum;						// 인덱스수
	TEX_PICTURE			*ptpTexture;					// 텍스처
	XMMATRIX			mMatrix;						// 변환행렬
	XMFLOAT4			v4AddColor;						// 가산색
};


// 글로벌 변수
UINT  g_nClientWidth;							// 그릴 영역 너비
UINT  g_nClientHeight;							// 그릴 영역 높이

HWND        g_hWnd;         // 윈도우 핸들


ID3D11Device			*g_pd3dDevice;			// 디바이스
IDXGISwapChain			*g_pSwapChain;			// DXGI스왑체인
ID3D11DeviceContext		*g_pImmediateContext;	// 디바이스 컨텍스트
ID3D11RasterizerState	*g_pRS;					// 래스터라이저
ID3D11RasterizerState	*g_pRS_Cull_CW;			// 래스터라이저(시계방향 컬링)
ID3D11RasterizerState	*g_pRS_Cull_CCW;		// 래스터라이저(반시계방향 컬링)
ID3D11RenderTargetView	*g_pRTV;				// 렌더링 타깃
ID3D11Texture2D*        g_pDepthStencil = NULL;	// Z버퍼
ID3D11DepthStencilView* g_pDepthStencilView = NULL;	// Z버퍼의 뷰
ID3D11DepthStencilState *g_pDSDepthState = NULL;	// Z버퍼의 스테이트
ID3D11DepthStencilState *g_pDSDepthState_NoWrite = NULL;	// Z버퍼의 스테이트(Z버퍼쓰기금지)
D3D_FEATURE_LEVEL       g_FeatureLevel;			// 피처레벨

ID3D11Buffer			*g_pVertexBuffer;		// 정점버퍼
ID3D11Buffer			*g_pIndexBuffer;		// 인덱스 버퍼
ID3D11BlendState		*g_pbsAddBlend;			// 가산 블렌드
ID3D11VertexShader		*g_pVertexShader;		// 정점 셰이더
ID3D11PixelShader		*g_pPixelShader;		// 픽셀 셰이더
ID3D11InputLayout		*g_pInputLayout;		// 셰이더 입력 레이아웃
ID3D11SamplerState		*g_pSamplerState;		// 샘플러 스테이트

ID3D11Buffer			*g_pCBNeverChanges = NULL;

// 그릴 정점 버퍼
CUSTOMVERTEX g_cvVertices[MAX_BUFFER_VERTEX];
int							g_nVertexNum = 0;

WORD		g_wIndices[MAX_BUFFER_INDEX];
int							g_nIndexNum = 0;

TEX_PICTURE				g_tSphere1Texture, g_tSphere2Texture;
MY_MODEL					g_mmGround;
MY_MODEL					g_mm3DRectangle;


// Direct3D 초기화
HRESULT InitD3D( void )
{
    HRESULT hr = S_OK;
	D3D_FEATURE_LEVEL  FeatureLevelsRequested[6] = { D3D_FEATURE_LEVEL_11_0,
													 D3D_FEATURE_LEVEL_10_1,
													 D3D_FEATURE_LEVEL_10_0,
													 D3D_FEATURE_LEVEL_9_3,
													 D3D_FEATURE_LEVEL_9_2,
													 D3D_FEATURE_LEVEL_9_1 };
	UINT               numLevelsRequested = 6;
	D3D_FEATURE_LEVEL  FeatureLevelsSupported;

	// 디바이스 생성
	hr = D3D11CreateDevice( NULL,
					D3D_DRIVER_TYPE_HARDWARE, 
					NULL, 
					0,
					FeatureLevelsRequested, 
					numLevelsRequested,
					D3D11_SDK_VERSION, 
					&g_pd3dDevice,
					&FeatureLevelsSupported,
					&g_pImmediateContext );
	if( FAILED ( hr ) ) {
		return hr;
	}

	// 팩토리 취득
	IDXGIDevice * pDXGIDevice;
	hr = g_pd3dDevice->QueryInterface( __uuidof( IDXGIDevice ), ( void ** )&pDXGIDevice );
	IDXGIAdapter * pDXGIAdapter;
	hr = pDXGIDevice->GetParent( __uuidof( IDXGIAdapter ), ( void ** )&pDXGIAdapter );
	IDXGIFactory * pIDXGIFactory;
	pDXGIAdapter->GetParent( __uuidof( IDXGIFactory ), ( void ** )&pIDXGIFactory);

	// 스왑체인 생성
    DXGI_SWAP_CHAIN_DESC	sd;
	ZeroMemory( &sd, sizeof( sd ) );
	sd.BufferCount = 1;
	sd.BufferDesc.Width = g_nClientWidth;
	sd.BufferDesc.Height = g_nClientHeight;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = g_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	hr = pIDXGIFactory->CreateSwapChain( g_pd3dDevice, &sd, &g_pSwapChain );

	pDXGIDevice->Release();
	pDXGIAdapter->Release();
	pIDXGIFactory->Release();

	if( FAILED ( hr ) ) {
		return hr;
	}

    // 렌더링 타깃 생성
    ID3D11Texture2D			*pBackBuffer = NULL;
    D3D11_TEXTURE2D_DESC BackBufferSurfaceDesc;
    hr = g_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
    if ( FAILED( hr ) ) {
		MessageBox( NULL, _T( "Can't get backbuffer." ), _T( "Error" ), MB_OK );
        return hr;
    }
    pBackBuffer->GetDesc( &BackBufferSurfaceDesc );
    hr = g_pd3dDevice->CreateRenderTargetView( pBackBuffer, NULL, &g_pRTV );
    SAFE_RELEASE( pBackBuffer );
    if ( FAILED( hr ) ) {
		MessageBox( NULL, _T( "Can't create render target view." ), _T( "Error" ), MB_OK );
        return hr;
    }

    // *** Create depth stencil texture ***
    D3D11_TEXTURE2D_DESC descDepth;
	RECT rc;
    GetClientRect( g_hWnd, &rc );
	ZeroMemory( &descDepth, sizeof(descDepth) );
    descDepth.Width = rc.right - rc.left;
    descDepth.Height = rc.bottom - rc.top;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    hr = g_pd3dDevice->CreateTexture2D( &descDepth, NULL, &g_pDepthStencil );
    if( FAILED( hr ) )
        return hr;

    // *** Create the depth stencil view ***
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory( &descDSV, sizeof(descDSV) );
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    hr = g_pd3dDevice->CreateDepthStencilView( g_pDepthStencil, &descDSV, &g_pDepthStencilView );
    if( FAILED( hr ) )
        return hr;

	// *** 렌더링 타깃 설정 ***
    g_pImmediateContext->OMSetRenderTargets( 1, &g_pRTV, g_pDepthStencilView );

	// 스텐실 스테이트 작성
	D3D11_DEPTH_STENCIL_DESC dsDesc;

	// Depth test parameters
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

	// Stencil test parameters
	dsDesc.StencilEnable = true;
	dsDesc.StencilReadMask = 0xFF;
	dsDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create depth stencil state
	hr = g_pd3dDevice->CreateDepthStencilState( &dsDesc, &g_pDSDepthState );

	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	hr = g_pd3dDevice->CreateDepthStencilState( &dsDesc, &g_pDSDepthState_NoWrite );

//	g_pImmediateContext->OMSetDepthStencilState( g_pDSDepthState, 1 );

    // 래스터라이저 설정
    D3D11_RASTERIZER_DESC drd;
	ZeroMemory( &drd, sizeof( drd ) );
	drd.FillMode				= D3D11_FILL_SOLID;
	drd.CullMode				= D3D11_CULL_NONE;
	drd.FrontCounterClockwise	= FALSE;
	drd.DepthClipEnable			= TRUE;
    hr = g_pd3dDevice->CreateRasterizerState( &drd, &g_pRS );
    if ( FAILED( hr ) ) {
		MessageBox( NULL, _T( "Can't create rasterizer state." ), _T( "Error" ), MB_OK );
        return hr;
    }
	g_pImmediateContext->RSSetState( g_pRS );

    // 래스터라이저 설정(시계방향 컬링)
	ZeroMemory( &drd, sizeof( drd ) );
	drd.FillMode				= D3D11_FILL_SOLID;
	drd.CullMode				= D3D11_CULL_BACK;
	drd.FrontCounterClockwise	= TRUE;
	drd.DepthClipEnable			= TRUE;
    hr = g_pd3dDevice->CreateRasterizerState( &drd, &g_pRS_Cull_CW );
    if ( FAILED( hr ) ) {
		MessageBox( NULL, _T( "Can't create rasterizer state." ), _T( "Error" ), MB_OK );
        return hr;
    }
//    g_pImmediateContext->RSSetState( g_pRS_Cull_CW );

    // 래스터라이저 설정(반시계방향 컬링)
	ZeroMemory( &drd, sizeof( drd ) );
	drd.FillMode				= D3D11_FILL_SOLID;
	drd.CullMode				= D3D11_CULL_BACK;
	drd.FrontCounterClockwise	= FALSE;
	drd.DepthClipEnable			= TRUE;
    hr = g_pd3dDevice->CreateRasterizerState( &drd, &g_pRS_Cull_CCW );
    if ( FAILED( hr ) ) {
		MessageBox( NULL, _T( "Can't create rasterizer state." ), _T( "Error" ), MB_OK );
        return hr;
    }
//    g_pImmediateContext->RSSetState( g_pRS_Cull_CCW );

    // 뷰포트 설정
    D3D11_VIEWPORT vp;
    vp.Width    = ( FLOAT )g_nClientWidth;
    vp.Height   = ( FLOAT )g_nClientHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
    g_pImmediateContext->RSSetViewports( 1, &vp );

    return S_OK;
}


// 프로그래머블 셰이더 작성
HRESULT MakeShaders( void )
{
    HRESULT hr;
    ID3DBlob* pVertexShaderBuffer = NULL;
    ID3DBlob* pPixelShaderBuffer = NULL;
    ID3DBlob* pError = NULL;

    DWORD dwShaderFlags = 0;
#ifdef _DEBUG
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif
    // 컴파일
    hr = D3DX11CompileFromFile( _T( "Basic_3D_TexMark.fx" ), NULL, NULL, "VS", "vs_4_0_level_9_1",
								dwShaderFlags, 0, NULL, &pVertexShaderBuffer, &pError, NULL );
    if ( FAILED( hr ) ) {
		MessageBox( NULL, _T( "Can't open Basic_3D_TexMark.fx" ), _T( "Error" ), MB_OK );
        SAFE_RELEASE( pError );
        return hr;
    }
    hr = D3DX11CompileFromFile( _T( "Basic_3D_TexMark.fx" ), NULL, NULL, "PS", "ps_4_0_level_9_1",
								dwShaderFlags, 0, NULL, &pPixelShaderBuffer, &pError, NULL );
    if ( FAILED( hr ) ) {
        SAFE_RELEASE( pVertexShaderBuffer );
        SAFE_RELEASE( pError );
        return hr;
    }
    SAFE_RELEASE( pError );
    
    // VertexShader 생성
    hr = g_pd3dDevice->CreateVertexShader( pVertexShaderBuffer->GetBufferPointer(),
										   pVertexShaderBuffer->GetBufferSize(),
										   NULL, &g_pVertexShader );
    if ( FAILED( hr ) ) {
        SAFE_RELEASE( pVertexShaderBuffer );
        SAFE_RELEASE( pPixelShaderBuffer );
        return hr;
    }
    // PixelShader 생성
    hr = g_pd3dDevice->CreatePixelShader( pPixelShaderBuffer->GetBufferPointer(),
										  pPixelShaderBuffer->GetBufferSize(),
										  NULL, &g_pPixelShader );
    if ( FAILED( hr ) ) {
        SAFE_RELEASE( pVertexShaderBuffer );
        SAFE_RELEASE( pPixelShaderBuffer );
        return hr;
    }

    // 입력버퍼의 입력형식
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXTURE",  0, DXGI_FORMAT_R32G32_FLOAT,       0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
	UINT numElements = ARRAYSIZE( layout );
	// 입력버퍼의 입력형식 생성
    hr = g_pd3dDevice->CreateInputLayout( layout, numElements,
										  pVertexShaderBuffer->GetBufferPointer(),
										  pVertexShaderBuffer->GetBufferSize(),
										  &g_pInputLayout );
    SAFE_RELEASE( pVertexShaderBuffer );
    SAFE_RELEASE( pPixelShaderBuffer );
    if ( FAILED( hr ) ) {
        return hr;
    }

    // 셰이더 상수 버퍼 생성
    D3D11_BUFFER_DESC bd;
    ZeroMemory( &bd, sizeof( bd ) );
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( CBNeverChanges );
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = g_pd3dDevice->CreateBuffer( &bd, NULL, &g_pCBNeverChanges );
    if( FAILED( hr ) )
        return hr;

    return S_OK;
}


// 텍스처 로드
int LoadTexture( TCHAR *szFileName, TEX_PICTURE *pTexPic, int nWidth, int nHeight,
				 int nTexWidth, int nTexHeight )
{
    HRESULT						hr;
	D3DX11_IMAGE_LOAD_INFO		liLoadInfo;
	ID3D11Texture2D				*pTexture;

	ZeroMemory( &liLoadInfo, sizeof( D3DX11_IMAGE_LOAD_INFO ) );
	liLoadInfo.Width = nTexWidth;
	liLoadInfo.Height = nTexHeight;
	liLoadInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	liLoadInfo.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	hr = D3DX11CreateShaderResourceViewFromFile( g_pd3dDevice, szFileName, &liLoadInfo,
												 NULL, &( pTexPic->pSRViewTexture ), NULL );
    if ( FAILED( hr ) ) {
        return hr;
    }
	pTexPic->pSRViewTexture->GetResource( ( ID3D11Resource ** )&( pTexture ) );
	pTexture->GetDesc( &( pTexPic->tdDesc ) );
	pTexture->Release();

	pTexPic->nWidth = nWidth;
	pTexPic->nHeight = nHeight;

	return S_OK;
}


// 드로우 모드 오브젝트 초기화
int InitDrawModes( void )
{
    HRESULT				hr;

	// 블렌드 스테이트
    D3D11_BLEND_DESC BlendDesc;
	BlendDesc.AlphaToCoverageEnable = FALSE;
	BlendDesc.IndependentBlendEnable = FALSE;
    BlendDesc.RenderTarget[0].BlendEnable           = TRUE;
    BlendDesc.RenderTarget[0].SrcBlend              = D3D11_BLEND_ONE;
    BlendDesc.RenderTarget[0].DestBlend             = D3D11_BLEND_ONE;
    BlendDesc.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
    BlendDesc.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
    BlendDesc.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_ZERO;
    BlendDesc.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
    BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    hr = g_pd3dDevice->CreateBlendState( &BlendDesc, &g_pbsAddBlend );
    if ( FAILED( hr ) ) {
        return hr;
    }

    // 샘플러
    D3D11_SAMPLER_DESC samDesc;
    ZeroMemory( &samDesc, sizeof( samDesc ) );
    samDesc.Filter          = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samDesc.AddressU        = D3D11_TEXTURE_ADDRESS_WRAP;
    samDesc.AddressV        = D3D11_TEXTURE_ADDRESS_WRAP;
    samDesc.AddressW        = D3D11_TEXTURE_ADDRESS_WRAP;
    samDesc.ComparisonFunc  = D3D11_COMPARISON_ALWAYS;
    samDesc.MaxLOD          = D3D11_FLOAT32_MAX;
    hr = g_pd3dDevice->CreateSamplerState( &samDesc, &g_pSamplerState );
    if ( FAILED( hr ) ) {
        return hr;
    }

    return S_OK;
}


// 지오메트리 초기화
HRESULT InitGeometry( void )
{
	int					i, j;
    HRESULT				hr = S_OK;

    // 정점 버퍼 생성
    D3D11_BUFFER_DESC BufferDesc;
    BufferDesc.Usage                = D3D11_USAGE_DYNAMIC;
    BufferDesc.ByteWidth            = sizeof( CUSTOMVERTEX ) * MAX_BUFFER_VERTEX;
    BufferDesc.BindFlags            = D3D11_BIND_VERTEX_BUFFER;
    BufferDesc.CPUAccessFlags       = D3D11_CPU_ACCESS_WRITE;
    BufferDesc.MiscFlags            = 0;

    D3D11_SUBRESOURCE_DATA SubResourceData;
    SubResourceData.pSysMem             = g_cvVertices;
    SubResourceData.SysMemPitch         = 0;
    SubResourceData.SysMemSlicePitch    = 0;
    hr = g_pd3dDevice->CreateBuffer( &BufferDesc, &SubResourceData, &g_pVertexBuffer );
    if ( FAILED( hr ) ) {
        return hr;
    }

    // 인덱스 버퍼 생성
    BufferDesc.Usage                = D3D11_USAGE_DYNAMIC;
    BufferDesc.ByteWidth            = sizeof( WORD ) * MAX_BUFFER_INDEX;
    BufferDesc.BindFlags            = D3D11_BIND_INDEX_BUFFER;
    BufferDesc.CPUAccessFlags       = D3D11_CPU_ACCESS_WRITE;
    BufferDesc.MiscFlags            = 0;

	SubResourceData.pSysMem         = g_wIndices;
    hr = g_pd3dDevice->CreateBuffer( &BufferDesc, &SubResourceData, &g_pIndexBuffer );
    if( FAILED( hr ) )
        return hr;

	// 텍스처 작성
	g_tSphere1Texture.pSRViewTexture =  NULL;
	hr = LoadTexture( _T( "18_2.bmp" ), &g_tSphere1Texture, 691, 691, 1024, 1024 );
    if ( FAILED( hr ) ) {
 		MessageBox( NULL, _T( "Can't open 18_2.bmp" ), _T( "Error" ), MB_OK );
       return hr;
    }
	g_tSphere2Texture.pSRViewTexture =  NULL;
	hr = LoadTexture( _T( "16.bmp" ), &g_tSphere2Texture, 614, 614, 1024, 1024 );
    if ( FAILED( hr ) ) {
 		MessageBox( NULL, _T( "Can't open 16.bmp" ), _T( "Error" ), MB_OK );
       return hr;
    }

	// 모델 작성
	int						nVertexNum1, nIndexNum1;

	// 플레이어
	Make3DRectangleIndexed( 1.5f, 0.7f, 1.0f, false,
					   &( g_cvVertices[g_nVertexNum] ), &nVertexNum1,
					   &( g_wIndices[g_nIndexNum] ),    &nIndexNum1, 0 );
	g_mm3DRectangle.nVertexPos = g_nVertexNum;
	g_mm3DRectangle.nVertexNum = nVertexNum1;
	g_mm3DRectangle.nIndexPos = g_nIndexNum;
	g_mm3DRectangle.nIndexNum = nIndexNum1;
	g_nVertexNum += nVertexNum1;
	g_nIndexNum += nIndexNum1;
	g_mm3DRectangle.ptpTexture = &g_tSphere2Texture;
	g_mm3DRectangle.mMatrix = XMMatrixIdentity();
	g_mm3DRectangle.v4AddColor = XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f );

	// 지면
	int				nIndex;
	int				nIndexZ1, nIndexZ2;
	float			x, y, z;
	float			ysq;
	float			u, v;

	nIndex = 0;
	z = -GROUND_SIZE / 2;
	for ( i = 0; i < GROUND_DIVIDE_NUM + 1; i++ ) {
		x = -GROUND_SIZE / 2;
		v = ( float )i / 20.0f;
		for ( j = 0; j < GROUND_DIVIDE_NUM + 1; j++ ) {
			if ( z > G_SPHERE_Z_POS ) {					// 구
				ysq = G_SPHERE_R * G_SPHERE_R - x * x - ( z - G_SPHERE_Z_POS ) * ( z - G_SPHERE_Z_POS );
			}
			else {										// 원기둥
				ysq = G_CYLINDER_R * G_CYLINDER_R - x * x;
			}
			if ( ysq > 0 ) y = -sqrtf( ysq );
					  else y = 0.0f;
			g_fHeights[i][j] = y;
			u = ( float )j / 20.0f;;
			g_cvVertices[g_nVertexNum + nIndex].v4Pos = XMFLOAT4( x, y, z, 1.0f );
			g_cvVertices[g_nVertexNum + nIndex].v2UV = XMFLOAT2( u, v );
			nIndex++;
			x += BLOCK_WIDTH;
		}
		z += BLOCK_WIDTH;
	}
	g_mmGround.nVertexPos = g_nVertexNum;
	g_mmGround.nVertexNum = nIndex;
	g_nVertexNum += nIndex;

	//fclose( fp );

	nIndex = 0;
	for ( i = 0; i < GROUND_DIVIDE_NUM; i++ ) {
		nIndexZ1 = i * ( GROUND_DIVIDE_NUM + 1 );
		nIndexZ2 = ( i + 1 ) * ( GROUND_DIVIDE_NUM + 1 );
		for ( j = 0; j < GROUND_DIVIDE_NUM; j++ ) {
			g_wIndices[g_nIndexNum + nIndex    ] = nIndexZ1 + j;
			g_wIndices[g_nIndexNum + nIndex + 1] = nIndexZ1 + j + 1;
			g_wIndices[g_nIndexNum + nIndex + 2] = nIndexZ2 + j;
			g_wIndices[g_nIndexNum + nIndex + 3] = nIndexZ1 + j + 1;
			g_wIndices[g_nIndexNum + nIndex + 4] = nIndexZ2 + j + 1;
			g_wIndices[g_nIndexNum + nIndex + 5] = nIndexZ2 + j;
			nIndex += 6;
		}
	}
	g_mmGround.nIndexPos = g_nIndexNum;
	g_mmGround.nIndexNum = nIndex;
	g_nIndexNum += nIndex;
	g_mmGround.ptpTexture = &g_tSphere1Texture;
	g_mmGround.mMatrix = XMMatrixIdentity();
	g_mmGround.v4AddColor = XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f );

	// 정점 버퍼 인덱스 버퍼 작성
	D3D11_MAPPED_SUBRESOURCE mappedVertices, mappedIndices;
	hr = g_pImmediateContext->Map( g_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedVertices );
    if( FAILED( hr ) )
        return hr;
	hr = g_pImmediateContext->Map( g_pIndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedIndices );
    if( FAILED( hr ) ) {
		g_pImmediateContext->Unmap( g_pVertexBuffer, 0 );
        return hr;
	}
	CopyMemory( mappedVertices.pData,  g_cvVertices, sizeof( CUSTOMVERTEX ) * g_nVertexNum );
	CopyMemory( mappedIndices.pData,  g_wIndices, sizeof( WORD ) * g_nIndexNum );
	g_pImmediateContext->Unmap( g_pVertexBuffer, 0 );
	g_pImmediateContext->Unmap( g_pIndexBuffer, 0 );

	MakeGroundNormals( &( g_cvVertices[g_mmGround.nVertexPos] ),
					   &( g_wIndices[g_mmGround.nIndexPos] ), g_mmGround.nIndexNum );


	return S_OK;
}


// 종료처리
int Cleanup( void )
{
    SAFE_RELEASE( g_tSphere1Texture.pSRViewTexture );
    SAFE_RELEASE( g_tSphere2Texture.pSRViewTexture );
    SAFE_RELEASE( g_pVertexBuffer );
    SAFE_RELEASE( g_pIndexBuffer );

    SAFE_RELEASE( g_pSamplerState );
    SAFE_RELEASE( g_pbsAddBlend );
    SAFE_RELEASE( g_pInputLayout );
    SAFE_RELEASE( g_pPixelShader );
    SAFE_RELEASE( g_pVertexShader );
    SAFE_RELEASE( g_pCBNeverChanges );

    SAFE_RELEASE( g_pRS );									// 래스터라이저
    SAFE_RELEASE( g_pRS_Cull_CW );
    SAFE_RELEASE( g_pRS_Cull_CCW );

	// 스테이터스 클리어
	if ( g_pImmediateContext ) {
		g_pImmediateContext->ClearState();
		g_pImmediateContext->Flush();
	}

    SAFE_RELEASE( g_pRTV );									// 렌더링 타깃
    SAFE_RELEASE( g_pDepthStencil );						// Z버퍼
    SAFE_RELEASE( g_pDepthStencilView );					// Z버퍼의 뷰
    SAFE_RELEASE( g_pDSDepthState );						// Z버퍼의 스테이트
    SAFE_RELEASE( g_pDSDepthState_NoWrite );

    // 스왑체인
    if ( g_pSwapChain != NULL ) {
        g_pSwapChain->SetFullscreenState( FALSE, 0 );
    }
    SAFE_RELEASE( g_pSwapChain );

    SAFE_RELEASE( g_pImmediateContext );					// 디바이스 컨텍스트
    SAFE_RELEASE( g_pd3dDevice );							// 디바이스

	return 0;
}


// 모데 그리기
int DrawMyModel( MY_MODEL *pmmDrawModel, XMMATRIX *pmViewProjection )
{
    CBNeverChanges	cbNeverChanges;

	cbNeverChanges.mView = XMMatrixTranspose( pmmDrawModel->mMatrix * *pmViewProjection );
	cbNeverChanges.v4AddColor = pmmDrawModel->v4AddColor;
	g_pImmediateContext->UpdateSubresource( g_pCBNeverChanges, 0, NULL, &cbNeverChanges, 0, 0 );
	g_pImmediateContext->PSSetShaderResources( 0, 1, &( pmmDrawModel->ptpTexture->pSRViewTexture ) );
	g_pImmediateContext->DrawIndexed( pmmDrawModel->nIndexNum, pmmDrawModel->nIndexPos, pmmDrawModel->nVertexPos );

	return 0;
}


// 모델의 부분 그리기
int DrawMyModelPartial( MY_MODEL *pmmDrawModel, XMMATRIX *pmViewProjection,
						int nTrianglePos, int nTriangleNum )
{
    CBNeverChanges	cbNeverChanges;
	int				nIndexPos, nIndexNum;

	cbNeverChanges.mView = XMMatrixTranspose( pmmDrawModel->mMatrix * *pmViewProjection );
	cbNeverChanges.v4AddColor = pmmDrawModel->v4AddColor;
	g_pImmediateContext->UpdateSubresource( g_pCBNeverChanges, 0, NULL, &cbNeverChanges, 0, 0 );
	g_pImmediateContext->PSSetShaderResources( 0, 1, &( pmmDrawModel->ptpTexture->pSRViewTexture ) );
	nIndexPos = nTrianglePos * 3;
	if ( nIndexPos > ( pmmDrawModel->nIndexNum - 3 ) ) nIndexPos = pmmDrawModel->nIndexNum - 3;
	nIndexNum = nTriangleNum * 3;
	if ( ( nIndexPos + nIndexNum ) > pmmDrawModel->nIndexNum ) nIndexNum = pmmDrawModel->nIndexNum - nIndexPos;
	g_pImmediateContext->DrawIndexed( nIndexNum, pmmDrawModel->nIndexPos + nIndexPos, pmmDrawModel->nVertexPos );

	return 0;
}


// 윈도우 프로시저
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
        case WM_DESTROY:
            PostQuitMessage( 0 );
            return 0;
    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
}


// 렌더링
HRESULT Render( void )
{
    // 화면 클리어
	XMFLOAT4	v4Color = XMFLOAT4( 0.0f, 0.0f, 1.0f, 1.0f );
    g_pImmediateContext->ClearRenderTargetView( g_pRTV, ( float * )&v4Color );
	// *** Z버퍼 클리어 ***
    g_pImmediateContext->ClearDepthStencilView( g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );

    // 샘플러 설정
    g_pImmediateContext->PSSetSamplers( 0, 1, &g_pSamplerState );
    
    // 그리기 설정
    UINT nStrides = sizeof( CUSTOMVERTEX );
    UINT nOffsets = 0;
    g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer, &nStrides, &nOffsets );
    g_pImmediateContext->IASetIndexBuffer( g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );
    g_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    g_pImmediateContext->IASetInputLayout( g_pInputLayout );

    // 셰이더 설정
    g_pImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
    g_pImmediateContext->VSSetConstantBuffers( 0, 1, &g_pCBNeverChanges );
    g_pImmediateContext->PSSetShader( g_pPixelShader, NULL, 0 );
    g_pImmediateContext->PSSetConstantBuffers( 0, 1, &g_pCBNeverChanges );
		
	// 변환행렬
    CBNeverChanges	cbNeverChanges;
	XMMATRIX		mWorld;
	XMMATRIX		mView;
	XMMATRIX		mProjection;
	XMMATRIX		mViewProjection;

	// Initialize the view matrix
	XMVECTOR Eye = XMVectorSet( Player_1.v3Pos.x, Player_1.v3Pos.y + 3.0f, Player_1.v3Pos.z - 7.0f, 0.0f );
	XMVECTOR At = XMVectorSet( Player_1.v3Pos.x, Player_1.v3Pos.y, Player_1.v3Pos.z, 0.0f );
	XMVECTOR Up = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
	mView = XMMatrixLookAtLH( Eye, At, Up );

    // Initialize the projection matrix
	mProjection = XMMatrixPerspectiveFovLH( XM_PIDIV4, VIEW_WIDTH / ( FLOAT )VIEW_HEIGHT, 0.01f, 100.0f );

	mViewProjection = mView * mProjection;

    // 그리기
	g_pImmediateContext->OMSetDepthStencilState( g_pDSDepthState, 1 );
    g_pImmediateContext->RSSetState( g_pRS_Cull_CW );				// 컬링 있음

	// 지면
    g_pImmediateContext->OMSetBlendState( NULL, NULL, 0xFFFFFFFF );
	g_mmGround.v4AddColor = XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f );
	DrawMyModel( &g_mmGround, &mViewProjection );

	// 플레이어
    g_pImmediateContext->OMSetBlendState( NULL, NULL, 0xFFFFFFFF );
	g_mm3DRectangle.mMatrix = CreateWorldMatrix( &Player_1 );
	DrawMyModel( &g_mm3DRectangle, &mViewProjection );

    return S_OK;
}


// 엔트리포인트
int WINAPI _tWinMain( HINSTANCE hInst, HINSTANCE, LPTSTR, int )
{
	LARGE_INTEGER			nNowTime, nLastTime;		// 현재와 직전 시각
	LARGE_INTEGER			nTimeFreq;					// 시간단위

    // 화면 크기
    g_nClientWidth  = VIEW_WIDTH;						//너비
    g_nClientHeight = VIEW_HEIGHT;						//높이

	// Register the window class
    WNDCLASSEX wc = { sizeof( WNDCLASSEX ), CS_CLASSDC, MsgProc, 0L, 0L,
                      GetModuleHandle( NULL ), NULL, NULL, NULL, NULL,
                      _T( "D3D Sample" ), NULL };
    RegisterClassEx( &wc );

	RECT rcRect;
	SetRect( &rcRect, 0, 0, g_nClientWidth, g_nClientHeight );
	AdjustWindowRect( &rcRect, WS_OVERLAPPEDWINDOW, FALSE );
    g_hWnd = CreateWindow( _T( "D3D Sample" ), _T( "3DPhysics_3_1" ),
						   WS_OVERLAPPEDWINDOW, 100, 20, rcRect.right - rcRect.left, rcRect.bottom - rcRect.top,
						   GetDesktopWindow(), NULL, wc.hInstance, NULL );

    // Initialize Direct3D
    if( SUCCEEDED( InitD3D() ) && SUCCEEDED( MakeShaders() ) )
    {
        // Create the shaders
        if( SUCCEEDED( InitDrawModes() ) )
        {
			if ( SUCCEEDED( InitGeometry() ) ) {					// 지오메트리 작성
				
				InitPlayer();										// 플레이어 초기화
				// Show the window
				ShowWindow( g_hWnd, SW_SHOWDEFAULT );
				UpdateWindow( g_hWnd );
				
				QueryPerformanceFrequency( &nTimeFreq );			// 시간단위
				QueryPerformanceCounter( &nLastTime );				// 1프레임전 시각 초기화

				// Enter the message loop
				MSG msg;
				ZeroMemory( &msg, sizeof( msg ) );
				while( msg.message != WM_QUIT )
				{
					MovePlayer();
					Render();
					do {
						if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
						{
							TranslateMessage( &msg );
							DispatchMessage( &msg );
						}
						QueryPerformanceCounter( &nNowTime );
					} while( ( ( nNowTime.QuadPart - nLastTime.QuadPart ) < ( nTimeFreq.QuadPart / 90 ) ) &&
							 ( msg.message != WM_QUIT ) );
					while( ( ( nNowTime.QuadPart - nLastTime.QuadPart ) < ( nTimeFreq.QuadPart / 60 ) ) &&
						   ( msg.message != WM_QUIT ) )
					{
						QueryPerformanceCounter( &nNowTime );
					}
					nLastTime = nNowTime;
					g_pSwapChain->Present( 0, 0 );					// 표시
				}
			}
        }
    }

    // Clean up everything and exit the app
    Cleanup();
    UnregisterClass( _T( "D3D Sample" ), wc.hInstance );
    return 0;
}

