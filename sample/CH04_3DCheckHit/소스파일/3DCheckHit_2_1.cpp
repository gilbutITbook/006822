//------------------------------------------------------------
// 3DCheckHit_2_1.cpp
// 평면적 삼각형과의 충돌판정
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
#define PLAYER_SPEED				0.08f				// 플레이어의 이동속도
#define CHECK_TRIANGLE_NUM			3					// 충돌판정 삼각형의 수
#define GROUND_SIZE					20.0f				// 바닥크기


// 정점구조체
struct CUSTOMVERTEX {
    XMFLOAT4	v4Pos;
	XMFLOAT2	v2UV;
};


struct MY_PLAYER {
	XMFLOAT3			v3Pos;					// 위치
};

MY_PLAYER	Player_1;							// 플레이어 데이터


// 충돌판정 삼각형 정점
XMFLOAT3				g_TriangleVertices[CHECK_TRIANGLE_NUM * 3] = { XMFLOAT3(   2.0f,  0.01f, -2.0f ), 
																	   XMFLOAT3(  -2.0f,  0.01f, -2.0f ), 
																	   XMFLOAT3(   0.0f,  0.01f,  1.5f ),
																	   XMFLOAT3(   6.0f,  0.01f, -2.5f ), 
																	   XMFLOAT3(   2.5f,  0.01f, -1.0f ), 
																	   XMFLOAT3(   4.0f,  0.01f,  4.5f ),
																	   XMFLOAT3(  -1.0f,  0.01f,  5.0f ), 
																	   XMFLOAT3(  -5.0f,  0.01f, -0.5f ), 
																	   XMFLOAT3(  -1.0f,  0.01f,  1.0f ) };



XMFLOAT3 Subtract( XMFLOAT3 *pv3Vec1, XMFLOAT3 *pv3Vec2 )						// 벡터의 뺄셈
{
	return XMFLOAT3( pv3Vec1->x - pv3Vec2->x,
					 pv3Vec1->y - pv3Vec2->y,
					 pv3Vec1->z - pv3Vec2->z );
}

// 삼각형과 점의 충돌판정
int CheckHit( XMFLOAT3 *pv3Triangle, XMFLOAT3 *pv3Point )
{
	// 삼각형 충돌판정 체크
	XMFLOAT3		v3TriVec0, v3TriVec1, v3TriVec2;
	XMFLOAT3		v3HitVec0, v3HitVec1, v3HitVec2;
	float			fCross0, fCross1, fCross2;
	int				bHit;

	// 삼각형 사이클 벡터
	v3TriVec0 = Subtract( pv3Triangle + 1, pv3Triangle + 0 );
	v3TriVec1 = Subtract( pv3Triangle + 2, pv3Triangle + 1 );
	v3TriVec2 = Subtract( pv3Triangle + 0, pv3Triangle + 2 );
	// 삼각형 정점에서 플레이어로 향한 벡터
	v3HitVec0 = Subtract( pv3Point, pv3Triangle + 0 );
	v3HitVec1 = Subtract( pv3Point, pv3Triangle + 1 );
	v3HitVec2 = Subtract( pv3Point, pv3Triangle + 2 );
	// 각각의 외적 
	fCross0 = v3TriVec0.z * v3HitVec0.x - v3TriVec0.x * v3HitVec0.z;
	fCross1 = v3TriVec1.z * v3HitVec1.x - v3TriVec1.x * v3HitVec1.z;
	fCross2 = v3TriVec2.z * v3HitVec2.x - v3TriVec2.x * v3HitVec2.z;
	bHit = FALSE;
	if ( fCross0 >= 0.0f ) {
		if ( ( fCross1 >= 0.0f ) && ( fCross2 >= 0.0f ) )
		{
			bHit = TRUE;
		}
	}
	else {
		if ( ( fCross1 < 0.0f ) && ( fCross2 < 0.0f ) )
		{
			bHit = TRUE;
		}
	}

	return bHit;
}


int InitPlayer( void )									// 플레이어 초기화
{
	// 플레이어1
	Player_1.v3Pos = XMFLOAT3( 0.0f, 0.0f, -4.0f );

	return 0;
}


int MovePlayer( void )									// 구의 이동
{
	// 좌
	if ( GetAsyncKeyState( VK_LEFT ) ) {
		Player_1.v3Pos.x -= PLAYER_SPEED;
	}
	// 우
	if ( GetAsyncKeyState( VK_RIGHT ) ) {
		Player_1.v3Pos.x += PLAYER_SPEED;
	}
	// 후
	if ( GetAsyncKeyState( VK_UP ) ) {
		Player_1.v3Pos.z += PLAYER_SPEED;
	}
	// 전
	if ( GetAsyncKeyState( VK_DOWN ) ) {
		Player_1.v3Pos.z -= PLAYER_SPEED;
	}

	return 0;
}



XMMATRIX CreateWorldMatrix( float x, float y, float z, float fSize )	// 월드행렬 생성
{
	float			fAngleY;							// y축 회전각
	static float	fAngleX = 0.0f;						// x축 회전각
	XMMATRIX		matRot_Y;							// y축 회전행렬
	XMMATRIX		matRot_X;							// x축 회전행렬
	XMMATRIX		matScaleTrans;						// 확대축소평행이동행렬

	// 강제회전
	fAngleY = 2.0f * PI * ( float )( timeGetTime() % 3000 ) / 3000.0f;

	// 행렬작성
	matRot_Y = XMMatrixRotationY( fAngleY );
	matRot_X = XMMatrixRotationX( fAngleX );

	matScaleTrans= XMMatrixIdentity();
	matScaleTrans._11 = fSize;
	matScaleTrans._22 = fSize;
	matScaleTrans._33 = fSize;
	matScaleTrans._41 = x;
	matScaleTrans._42 = y;
	matScaleTrans._43 = z;

	return matRot_Y * matRot_X * matScaleTrans;		// 변환의 합성
}


int MakeSphereIndexed( float x, float y, float z, float r,
					   CUSTOMVERTEX *pVertices, int *pVertexNum,
					   WORD *pIndices, int *pIndexNum,
					   int nIndexOffset )			// 구의 작성(중심센터와 인덱스)
{
	int					i, j;
	float				fTheta;
	float				fPhi;
	float				fAngleDelta;
	int					nIndex;						// 데이터의 인덱스
	int					nIndexY;					// x방향 인덱스

	// 정점 데이터 작성
	fAngleDelta = 2.0f * PI / CORNER_NUM;
	nIndex = 0;
	fTheta = 0.0f;
	for ( i = 0; i < CORNER_NUM / 2 + 1; i++ ) {
		fPhi = 0.0f;
		for ( j = 0; j < CORNER_NUM + 1; j++ ) {
			pVertices[nIndex].v4Pos  = XMFLOAT4( x + r * sinf( fTheta ) * cosf( fPhi ),
												 y + r * cosf( fTheta ),
												 z + r * sinf( fTheta ) * sinf( fPhi ), 1.0f );
			pVertices[nIndex].v2UV = XMFLOAT2( fPhi / ( 2.0f * PI ), fTheta / PI );
			nIndex++;
			fPhi += fAngleDelta;
		}
		fTheta += fAngleDelta;
	}
	*pVertexNum = nIndex;

	// 인덱스 데이터 작성
	nIndex = 0;
	for ( i = 0; i < CORNER_NUM; i++ ) {
		for ( j = 0; j < CORNER_NUM / 2; j++ ) {
			nIndexY = j * ( CORNER_NUM + 1 );
			pIndices[nIndex    ] = nIndexOffset + nIndexY + i;
			pIndices[nIndex + 1] = nIndexOffset + nIndexY + ( CORNER_NUM + 1 ) + i;
			pIndices[nIndex + 2] = nIndexOffset + nIndexY + i + 1;
			nIndex += 3;
			pIndices[nIndex    ] = nIndexOffset + nIndexY + i + 1;
			pIndices[nIndex + 1] = nIndexOffset + nIndexY + ( CORNER_NUM + 1 ) + i;
			pIndices[nIndex + 2] = nIndexOffset + nIndexY + ( CORNER_NUM + 1 ) + i + 1;
			nIndex += 3;
		}
	}
	*pIndexNum = nIndex;

	return 0;
}


int MakeConeIndexed( float fHeight, float r,
					 CUSTOMVERTEX *pVertices, int *pVertexNum,
					 WORD *pIndices, int *pIndexNum,
					 int nIndexOffset )				// 원뿔 만들기(인덱스됨)
{
	int					i, j;
	float				fTheta;
	float				fAngleDelta;
	int					nIndex;						// 데이터의 인터페이스

	// 정점 데이터 작성
	fAngleDelta = 2.0f * PI / CORNER_NUM;
	nIndex = 0;
	pVertices[nIndex].v4Pos  = XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f );
	pVertices[nIndex].v2UV = XMFLOAT2( 0.5f, 1.0f );
	nIndex++;
	fTheta = 0.0f;
	for ( j = 0; j < CORNER_NUM + 1; j++ ) {
		pVertices[nIndex].v4Pos  = XMFLOAT4( r * cosf( fTheta ),
											 fHeight,
											 r * sinf( fTheta ), 1.0f );
		pVertices[nIndex].v2UV = XMFLOAT2( fTheta / ( 2.0f * PI ), 0.5f );
		nIndex++;
		fTheta += fAngleDelta;
	}
	pVertices[nIndex].v4Pos  = XMFLOAT4( 0.0f, fHeight, 0.0f, 1.0f );
	pVertices[nIndex].v2UV = XMFLOAT2( 0.5f, 0.0f );
	nIndex++;
	*pVertexNum = nIndex;

	// 인덱스 데이터 작성
	nIndex = 0;
	for ( i = 0; i < CORNER_NUM; i++ ) {
		pIndices[nIndex    ] = nIndexOffset + 0;
		pIndices[nIndex + 1] = nIndexOffset + i + 1 + 1;
		pIndices[nIndex + 2] = nIndexOffset + i + 1;
		nIndex += 3;
		pIndices[nIndex    ] = nIndexOffset + CORNER_NUM + 2;
		pIndices[nIndex + 1] = nIndexOffset + i + 1;
		pIndices[nIndex + 2] = nIndexOffset + i + 1 + 1;
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


// 링크라이브러리
#pragma comment( lib, "d3d11.lib" )   // D3D11라이브러리
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
	int					nIndexPos;						// 인덱스 위치
	int					nIndexNum;						// 인덱스수
	TEX_PICTURE			*ptpTexture;					// 텍스처
	XMMATRIX			mMatrix;						// 변환행렬
	XMFLOAT4			v4AddColor;						// 가산색
};


// 글로벌변수
UINT  g_nClientWidth;							// 그리는 영역의 너비
UINT  g_nClientHeight;							// 그리는 영역의 높이

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
ID3D11DepthStencilState *g_pDSDepthState = NULL;	// Z버퍼의 상태
ID3D11DepthStencilState *g_pDSDepthState_NoWrite = NULL;	// Z버퍼의 상태(Z버퍼 쓰기금지)
D3D_FEATURE_LEVEL       g_FeatureLevel;			// 피처레벨

ID3D11Buffer			*g_pVertexBuffer;		// 정점버퍼
ID3D11Buffer			*g_pIndexBuffer;		// 인덱스버퍼
ID3D11BlendState		*g_pbsAddBlend;			// 가산블렌드
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

TEX_PICTURE				g_tGroundTexture, g_tAreaTexture;
TEX_PICTURE				g_tPlayerTexture;
MY_MODEL					g_mmPlayer,  g_mmGround;
MY_MODEL					g_mmTriangles[CHECK_TRIANGLE_NUM];


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
    
    // VertexShader생성
    hr = g_pd3dDevice->CreateVertexShader( pVertexShaderBuffer->GetBufferPointer(),
										   pVertexShaderBuffer->GetBufferSize(),
										   NULL, &g_pVertexShader );
    if ( FAILED( hr ) ) {
        SAFE_RELEASE( pVertexShaderBuffer );
        SAFE_RELEASE( pPixelShaderBuffer );
        return hr;
    }
    // PixelShader생성
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
    HRESULT hr = S_OK;

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

	// 택스처 작성
	g_tGroundTexture.pSRViewTexture =  NULL;
	hr = LoadTexture( _T( "10.bmp" ), &g_tGroundTexture, 691, 691, 1024, 1024 );
    if ( FAILED( hr ) ) {
 		MessageBox( NULL, _T( "Can't open 10.bmp" ), _T( "Error" ), MB_OK );
       return hr;
    }
	g_tAreaTexture.pSRViewTexture =  NULL;
	hr = LoadTexture( _T( "8.bmp" ), &g_tAreaTexture, 185, 185, 256, 256 );
    if ( FAILED( hr ) ) {
 		MessageBox( NULL, _T( "Can't open 8.bmp" ), _T( "Error" ), MB_OK );
       return hr;
    }
	g_tPlayerTexture.pSRViewTexture =  NULL;
	hr = LoadTexture( _T( "9.bmp" ), &g_tPlayerTexture, 222, 222, 256, 256 );
    if ( FAILED( hr ) ) {
 		MessageBox( NULL, _T( "Can't open 9.bmp" ), _T( "Error" ), MB_OK );
       return hr;
    }

	// 모델 작성
	int						nVertexNum1, nIndexNum1;
	int						nVertexNum2, nIndexNum2;
	// 플레이어
	MakeSphereIndexed( 0.0f, 0.68f, 0.0f, 0.16f,
					   &( g_cvVertices[g_nVertexNum] ), &nVertexNum1,
					   &( g_wIndices[g_nIndexNum] ),    &nIndexNum1, 0 );
	MakeConeIndexed( 0.5f, 0.2f,
					 &( g_cvVertices[g_nVertexNum + nVertexNum1] ), &nVertexNum2,
					 &( g_wIndices[g_nIndexNum + nIndexNum1] ),     &nIndexNum2,
					 nVertexNum1 );
	g_mmPlayer.nVertexPos = g_nVertexNum;
	g_mmPlayer.nVertexNum = nVertexNum1 + nVertexNum2;
	g_mmPlayer.nIndexPos = g_nIndexNum;
	g_mmPlayer.nIndexNum = nIndexNum1 + nIndexNum2;
	g_nVertexNum += nVertexNum1 + nVertexNum2;
	g_nIndexNum += nIndexNum1 + nIndexNum2;
	g_mmPlayer.ptpTexture = &g_tPlayerTexture;
	g_mmPlayer.mMatrix = XMMatrixIdentity();
	g_mmPlayer.v4AddColor = XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f );

	// 지면
	g_cvVertices[g_nVertexNum   ].v4Pos = XMFLOAT4( -GROUND_SIZE / 2, 0.0f, GROUND_SIZE / 2, 1.0f );
	g_cvVertices[g_nVertexNum   ].v2UV = XMFLOAT2( 0.0f, 0.0f );
	g_cvVertices[g_nVertexNum + 1].v4Pos = XMFLOAT4(  GROUND_SIZE / 2, 0.0f, GROUND_SIZE / 2, 1.0f );
	g_cvVertices[g_nVertexNum + 1].v2UV = XMFLOAT2( 1.0f, 0.0f );
	g_cvVertices[g_nVertexNum + 2].v4Pos = XMFLOAT4( -GROUND_SIZE / 2, 0.0f, -GROUND_SIZE / 2, 1.0f );
	g_cvVertices[g_nVertexNum + 2].v2UV = XMFLOAT2( 0.0f, 1.0f );
	g_cvVertices[g_nVertexNum + 3].v4Pos = XMFLOAT4(  GROUND_SIZE / 2, 0.0f, -GROUND_SIZE / 2, 1.0f );
	g_cvVertices[g_nVertexNum + 3].v2UV = XMFLOAT2( 1.0f, 1.0f );
	g_wIndices[g_nIndexNum   ] = 0;
	g_wIndices[g_nIndexNum + 1] = 2;
	g_wIndices[g_nIndexNum + 2] = 1;
	g_wIndices[g_nIndexNum + 3] = 1;
	g_wIndices[g_nIndexNum + 4] = 2;
	g_wIndices[g_nIndexNum + 5] = 3;
	g_mmGround.nVertexPos = g_nVertexNum;
	g_mmGround.nVertexNum = 4;
	g_mmGround.nIndexPos = g_nIndexNum;
	g_mmGround.nIndexNum = 6;
	g_nVertexNum += 4;
	g_nIndexNum += 6;
	g_mmGround.ptpTexture = &g_tGroundTexture;
	g_mmGround.mMatrix = XMMatrixIdentity();
	g_mmGround.v4AddColor = XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f );

	// 충돌판정 삼각형
	int					i;
	int					nIndex;
	nIndex = 0;
	for ( i = 0; i < CHECK_TRIANGLE_NUM * 3; i++ ) {
		g_cvVertices[g_nVertexNum + nIndex].v4Pos = XMFLOAT4( g_TriangleVertices[i].x,
															  g_TriangleVertices[i].y,
															  g_TriangleVertices[i].z, 1.0f );
		g_cvVertices[g_nVertexNum + nIndex].v2UV = XMFLOAT2( ( float )( i % 2 ), ( float )( i / 2 ) );
		g_wIndices[g_nIndexNum + nIndex] = i % 3;
		nIndex++;
	}
	for ( i = 0; i < CHECK_TRIANGLE_NUM; i++ ) {
		g_mmTriangles[i].nVertexPos = g_nVertexNum;
		g_mmTriangles[i].nVertexNum = 3;
		g_nVertexNum += 3;
		g_mmTriangles[i].nIndexPos = g_nIndexNum;
		g_mmTriangles[i].nIndexNum = 3;
		g_nIndexNum += 3;
		g_mmTriangles[i].ptpTexture = &g_tAreaTexture;
		g_mmTriangles[i].mMatrix = XMMatrixIdentity();
		g_mmTriangles[i].v4AddColor = XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f );
	}

	// 정점버퍼 인덱스 버퍼 작성
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

	return S_OK;
}


// 종료처리
int Cleanup( void )
{
    SAFE_RELEASE( g_tGroundTexture.pSRViewTexture );
    SAFE_RELEASE( g_tAreaTexture.pSRViewTexture );
    SAFE_RELEASE( g_tPlayerTexture.pSRViewTexture );
    SAFE_RELEASE( g_pVertexBuffer );
    SAFE_RELEASE( g_pIndexBuffer );

    SAFE_RELEASE( g_pSamplerState );
    SAFE_RELEASE( g_pbsAddBlend );
    SAFE_RELEASE( g_pInputLayout );
    SAFE_RELEASE( g_pPixelShader );
    SAFE_RELEASE( g_pVertexShader );
    SAFE_RELEASE( g_pCBNeverChanges );

    SAFE_RELEASE( g_pRS );									// ㅓ래스터라이저
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


// 모델 그리기
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
	int						i;
	int						bHitResult[3];			// 충돌판정결과

    // 화면 클리어
	XMFLOAT4	v4Color = XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f );
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

	// 삼각형과의 충돌판정
	for ( i = 0; i < CHECK_TRIANGLE_NUM; i++ ) {
		bHitResult[i] = CheckHit( &( g_TriangleVertices[i * 3] ), &( Player_1.v3Pos ) );
	}
		
	// 변환행렬
    CBNeverChanges	cbNeverChanges;
	XMMATRIX		mWorld;
	XMMATRIX		mView;
	XMMATRIX		mProjection;
	XMMATRIX		mViewProjection;

	// Initialize the view matrix
	XMVECTOR Eye = XMVectorSet( Player_1.v3Pos.x, Player_1.v3Pos.y + 3.0f, Player_1.v3Pos.z - 5.0f, 0.0f );
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
	DrawMyModel( &g_mmGround, &mViewProjection );

	// 충돌판정 삼각형
    g_pImmediateContext->RSSetState( g_pRS );						// 컬링 없음 
	for ( i = 0; i < CHECK_TRIANGLE_NUM; i++ ) {
		if ( bHitResult[i] ) {
			g_mmTriangles[i].v4AddColor = XMFLOAT4( 0.0f, 1.0f, 0.0f, 0.0f );
		}
		else {
			g_mmTriangles[i].v4AddColor = XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f );
		}
		DrawMyModel( &g_mmTriangles[i], &mViewProjection );
	}

	// 플레이어
    g_pImmediateContext->OMSetBlendState( NULL, NULL, 0xFFFFFFFF );
	g_mmPlayer.mMatrix = CreateWorldMatrix( Player_1.v3Pos.x, Player_1.v3Pos.y, Player_1.v3Pos.z, 1.0f );
	DrawMyModel( &g_mmPlayer, &mViewProjection );

    return S_OK;
}


// 엔트리포인트
int WINAPI _tWinMain( HINSTANCE hInst, HINSTANCE, LPTSTR, int )
{
	LARGE_INTEGER			nNowTime, nLastTime;		// 현재와 직전시각
	LARGE_INTEGER			nTimeFreq;					// 시간단위

    // 화면크기
    g_nClientWidth  = VIEW_WIDTH;						// 너비
    g_nClientHeight = VIEW_HEIGHT;						// 높이

	// Register the window class
    WNDCLASSEX wc = { sizeof( WNDCLASSEX ), CS_CLASSDC, MsgProc, 0L, 0L,
                      GetModuleHandle( NULL ), NULL, NULL, NULL, NULL,
                      _T( "D3D Sample" ), NULL };
    RegisterClassEx( &wc );

	RECT rcRect;
	SetRect( &rcRect, 0, 0, g_nClientWidth, g_nClientHeight );
	AdjustWindowRect( &rcRect, WS_OVERLAPPEDWINDOW, FALSE );
    g_hWnd = CreateWindow( _T( "D3D Sample" ), _T( "3DCheckHit_2_1" ),
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

