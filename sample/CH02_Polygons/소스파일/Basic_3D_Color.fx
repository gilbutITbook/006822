// ---------------------------------------------------------
// Basic_3D_Color.fx
// Simple3D셰이더(정점에 색칠)
// ---------------------------------------------------------


// 상수 버퍼
cbuffer cbNeverChanges : register( b0 )
{
    matrix View;
};


// VertexShader입력형식
struct VS_INPUT {
    float4 v4Position	: POSITION;		// 위치
    float4 v4Color		: COLOR;		// 색
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

    Output.v4Position = mul( Input.v4Position, View );
    Output.v4Color = Input.v4Color;

    return Output;
}

// 픽셀 셰이더
float4 PS( VS_OUTPUT Input ) : SV_TARGET {
    return Input.v4Color;
}
