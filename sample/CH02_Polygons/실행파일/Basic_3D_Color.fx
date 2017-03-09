// ---------------------------------------------------------
// Basic_3D_Color.fx
// Simple3Dシェーダ(頂点に色付き)
// ---------------------------------------------------------


// 定数バッファ
cbuffer cbNeverChanges : register( b0 )
{
    matrix View;
};


// VertexShader入力形式
struct VS_INPUT {
    float4 v4Position	: POSITION;		// 位置
    float4 v4Color		: COLOR;		// 色
};

// VertexShader出力形式
struct VS_OUTPUT {
    float4 v4Position	: SV_POSITION;	// 位置
    float4 v4Color		: COLOR;		// 色
};

// 頂点シェーダ
VS_OUTPUT VS( VS_INPUT Input )
{
    VS_OUTPUT	Output;

    Output.v4Position = mul( Input.v4Position, View );
    Output.v4Color = Input.v4Color;

    return Output;
}

// ピクセルシェーダ
float4 PS( VS_OUTPUT Input ) : SV_TARGET {
    return Input.v4Color;
}
