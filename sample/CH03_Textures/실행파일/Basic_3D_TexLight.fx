// ---------------------------------------------------------
// Basic_3D_TexLight.fx
// 3Dテクスチャ・光源処理付きシェーダ
// ---------------------------------------------------------


// テクスチャ
Texture2D Tex2D : register( t0 );		// テクスチャ

// テクスチャサンプラ
SamplerState MeshTextureSampler : register( s0 )
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

// 定数バッファ
cbuffer cbNeverChanges : register( b0 )
{
    matrix View;
    matrix World;
	float3 v3Light;
	float  Ambient;
	float  Directional;
};


// VertexShader入力形式
struct VS_INPUT {
    float4 v4Position	: POSITION;		// 位置
    float2 v2Tex		: TEXTURE;		// テクスチャ座標
    float3 v3Normal		: NORMAL;		// 法線ベクトル
};

// VertexShader出力形式
struct VS_OUTPUT {
    float4 v4Position	: SV_POSITION;	// 位置
    float4 v4Color		: COLOR;		// 色
    float2 v2Tex		: TEXTURE;		// テクスチャ座標
};

// 頂点シェーダ
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

// ピクセルシェーダ
float4 PS( VS_OUTPUT Input ) : SV_TARGET {
    return Tex2D.Sample( MeshTextureSampler, Input.v2Tex ) * Input.v4Color;
}
