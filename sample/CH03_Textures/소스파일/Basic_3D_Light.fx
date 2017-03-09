// ---------------------------------------------------------
// Basic_3D_Light.fx
// 3D 광원처리 셰이더
// ---------------------------------------------------------

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
    float3 v3Normal		: NORMAL;		// 법선벡터
};

// VertexShader출력형식
struct VS_OUTPUT {
    float4 v4Position	: SV_POSITION;	// 위치
    float4 v4Color		: COLOR;		// 색
};

// 정점 셰이더
VS_OUTPUT VS( VS_INPUT Input )
{
    VS_OUTPUT	Output;
	float3		v3WorldNormal;
	float		fPlusDot;
	float		fBright;

    Output.v4Position = mul( Input.v4Position, View );
	v3WorldNormal = mul( Input.v3Normal, World );
	fPlusDot = max( dot( v3WorldNormal, v3Light ), 0.0 );
    fBright = Directional * fPlusDot + Ambient;
    Output.v4Color = float4( fBright, fBright, fBright, 1.0 );

    return Output;
}

// 픽셀 셰이더
float4 PS( VS_OUTPUT Input ) : SV_TARGET {
    return Input.v4Color;
}
