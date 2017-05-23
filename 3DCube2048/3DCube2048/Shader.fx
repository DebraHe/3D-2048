//////////////////////////////////////////////////////////////////////////
// 定义常量常量缓存
//////////////////////////////////////////////////////////////////////////
//坐标变换矩阵的常量缓存
cbuffer MatrixBuffer
{
	matrix World;         //世界坐标变换矩阵
	matrix View;          //观察坐标变换矩阵
	matrix Projection;    //投影坐标变换矩阵
	float4 EyePosition;   //视点位置
};

Texture2D Texture;       //纹理变量

SamplerState Sampler     //定义采样器
{
	Filter = MIN_MAG_MIP_LINEAR;   //采用线性过滤
	AddressU = WRAP;              //寻址模式为WRAP
	AddressV = WRAP;              //寻址模式为WRAP
};

//////////////////////////////////////////////////////////////////////////
//定义输入结构
//////////////////////////////////////////////////////////////////////////
//顶点着色器的输入结构
struct VS_INPUT
{
	float4 Pos : POSITION;   //位置
	float2 Tex : TEXCOORD0;  //纹理
};

//顶点着色器的输出结构
struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;    //位置
	float2 Tex : TEXCOORD0;      //纹理
};

//////////////////////////////////////////////////////////////////////////
// 顶点着色器
//////////////////////////////////////////////////////////////////////////
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;              //声明一个VS_OUTPUT对象

	output.Pos = mul(input.Pos, World);         //在input坐标上进行世界变换
	output.Pos = mul(output.Pos, View);         //进行观察变换
	output.Pos = mul(output.Pos, Projection);   //进行投影变换

	output.Tex = input.Tex;       //纹理设置

	return output;
}

//////////////////////////////////////////////////////////////////////////
// 像素着色器
//////////////////////////////////////////////////////////////////////////
//默认像素着色器
float4 PS(VS_OUTPUT input) : SV_Target
{
	float4 texColor = Texture.Sample(Sampler, input.Tex);
	clip(texColor.a - 0.1f);
	return  texColor;   //返回纹理
}


//////////////////////////////////////////////////////////////////////////
// 定义Technique
//////////////////////////////////////////////////////////////////////////
technique11 TexTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}

