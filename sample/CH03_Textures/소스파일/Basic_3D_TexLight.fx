// ---------------------------------------------------------
// Basic_3D_TexLight.fx
// 3D 텍스처 광원처리 셰이더
// ---------------------------------------------------------


// 텍스처
Texture2D Tex2D : register( t0 );		// 텏처

// 텍스처 샘플러
SamplerState MeshTextureSampler : register( s0 )
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

// 상수 버퍼
cbuffer cbNeverChanges : register( b0 )
{
    matrix View;
    matrix World;
	float3 v3Light;
	float  Ambient;
	float  Directional;
};


// VertexShader입력형식
struct VS_INPUT {
    float4 v4Position	: POSITION;		// 위치
    float2 v2Tex		: TEXTURE;		// 텍스처 좌표
    float3 v3Normal		: NORMAL;		// 법선벡터
};

// VertexShader출력형식
struct VS_OUTPUT {
    float4 v4Position	: SV_POSITION;	// 위치
    float4 v4Color		: COLOR;		// 색
    float2 v2Tex		: TEXTURE;		// 텍스처 좌표
};

// 정점 셰이더
VS_OUTPUT VS( VS_INPUT Input )
{
    VS_OUTPUT	Output;
	float3		v3WorldNormal;
	float		fBright;

    Output.v4Position = mul( Input.v4Position, View );
	v3WorldNormal = mul( Input.v3Normal, World );
    fBright = Directional * max( dot( v3WorldNormal, v3Light ), 0 ) + Ambient;
    Output.v4Color = float4( fBright, fBright, fBright, 1.0 );
    Output.v2Tex = Input.v2Tex;

    return Output;
}

// 픽셀 셰이더
float4 PS( VS_OUTPUT Input ) : SV_TARGET {
    return Tex2D.Sample( MeshTextureSampler, Input.v2Tex ) * Input.v4Color;
}
