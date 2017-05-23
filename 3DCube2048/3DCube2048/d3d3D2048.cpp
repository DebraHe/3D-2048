#include "d3dUtility.h"
#include <stdlib.h>
#include <time.h>
//声明全局的指针
ID3D11Device* device = NULL;//D3D11设备接口
IDXGISwapChain* swapChain = NULL;//交换链接口
ID3D11DeviceContext* immediateContext = NULL;
ID3D11RenderTargetView* renderTargetView = NULL;//渲染目标视图  

//Effect相关全局指针
ID3D11InputLayout* vertexLayout;
ID3DX11Effect* effect;
ID3DX11EffectTechnique* technique;

//声明三个坐标系矩阵
XMMATRIX world;         //用于世界变换的矩阵
XMMATRIX view;          //用于观察变换的矩阵
XMMATRIX projection;    //用于投影变换的矩阵

ID3D11DepthStencilView* depthStencilView;  //深度模板视图
ID3D11Texture2D* depthStencilBuffer;       //深度缓存


ID3D11ShaderResourceView* texture[13];   //4096
ID3D11ShaderResourceView* textureMiss;   //失败
ID3D11ShaderResourceView* textureGoal;   //成功达到4096
ID3D11ShaderResourceView* textureBackground;   //背景图片

static float angle = -XM_PI / 2;   //声明一个静态变量用于记录角度
static float dangle = 0.0f;
static float height = 0.0f;        //声明一个静态变量用于记录高度
static float dheight = 0.0f;
//
ID3D11RasterizerState* NoCullRS;             //背面消隐状态



//判断按键是否按下
bool Lbtndown = false;
bool Rbtndown = false;
bool Ubtndown = false;
bool Dbtndown = false;

//判断是否达到4096或者是达到“死亡”条件
bool goSucceed = false;
bool goDie = false;

//定义一个数组
int cubesite[4][4][4][2];

//记录向左向右旋转的面
int flag1 = 1;

//定义一个顶点结构，这个顶点包含坐标和法向量和纹理坐标
struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT2 Tex;
};

//初始化64个正方体，纹理贴为0
void InitCubesite()
{
	goSucceed = false;
	goDie = false;
	flag1 = 1;
	int v = 0;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			for (int k = 0; k < 4; k++)
			{

				cubesite[i][j][k][0] = v;
				cubesite[i][j][k][1] = 0;
				v = v + 36;

			}
		}
	}
}

//每移动一次，随机生成三个新正方体
void SetRandom()
{
	int nothingNum = 0;

	//判断是否达到“4096”，以及未填充的正方体个数
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			for (int k = 0; k < 4; k++)
			{
				if (cubesite[i][j][k][1] == 0) nothingNum++;
				if (cubesite[i][j][k][1] == 12)
				{
					goSucceed = true;
					return;
				}
			}
		}
	}

	//根据未填充的正方体数目来确定本次移动需要新填充的正方体数目
	int newNum = 3;
	switch (nothingNum)
	{
	case 0:
	{
			  //当64个正方体都被填满后，判断是否还有可合并的正方体
			  for (int i = 0; i < 4; i++)
			  {
				  for (int j = 0; j < 4; j++)
				  {
					  for (int k = 0; k < 4; k++)
					  {
						  if ((i - 1) >= 0 && cubesite[i][j][k][1] == cubesite[i - 1][j][k][1]) return;
						  else if ((i + 1) <= 3 && cubesite[i][j][k][1] == cubesite[i + 1][j][k][1]) return;
						  else if ((j - 1) >= 0 && cubesite[i][j][k][1] == cubesite[i][j - 1][k][1]) return;
						  else if ((j + 1) <= 3 && cubesite[i][j][k][1] == cubesite[i][j + 1][k][1]) return;
						  else if ((k - 1) >= 0 && cubesite[i][j][k][1] == cubesite[i][j][k - 1][1]) return;
						  else if ((j + 1) <= 3 && cubesite[i][j][k][1] == cubesite[i][j][k + 1][1]) return;

					  }
				  }
			  }
			  //如果都没有可合并的正方体，则本次游戏结束
			  goDie = true;
	}
		break;
	case 1:
		newNum = 1;
		break;
	case 2:
		newNum = 2;
		break;
	default:
		break;
	}

	//随机数生成
	int randomTexture[2] = { 1, 2 };
	int randomi[4] = { 0, 1, 2, 3 };
	int randomj[4] = { 0, 1, 2, 3 };
	int ramdomk[4] = { 0, 1, 2, 3 };
	int t1, t2, t3, t4;
	srand((unsigned)time(NULL));
	for (int l = 0; l < newNum;)
	{

		t1 = rand() % 2;
		t2 = rand() % 4;
		t3 = rand() % 4;
		t4 = rand() % 4;
		if (cubesite[randomi[t2]][randomj[t3]][ramdomk[t4]][1] == 0)
		{
			cubesite[randomi[t2]][randomj[t3]][ramdomk[t4]][1] = randomTexture[t1];
			l++;
		}
	}



}

void MoveZZ()
{
	int flag = 0;
	int swap = 0;
	int move = 0;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{

			for (int k = 3; k >-1; k--)//根据方向从左向右或从右向左依次遍历
			{
				if (cubesite[k][j][i][1] == 0)//当当前位置的贴图为0时，表示为空（透明的弄不出来），不发生换图，累计空的个数move
				{
					move++;
				}
				if (cubesite[k][j][i][1] > 0)//当当前位置的贴图不为0时，和前面move个位置的方块交换贴图
				{
					swap = cubesite[k][j][i][1];
					cubesite[k][j][i][1] = cubesite[k + move][j][i][1];
					cubesite[k + move][j][i][1] = swap;
					if (move != 0)
					{
						flag++;
					}
					if (k<3 && (k + move)<3)//交换完后，判断和左侧相邻图片贴图相不相同，相同则左侧位置方块贴图数字换位原来两倍，自己清零
					{
						if (cubesite[k + move][j][i][1] == cubesite[k + move + 1][j][i][1])
						{
							cubesite[k + move + 1][j][i][1]++;//贴图数字为了和图片文件路径对应，用2的次方数表达
							cubesite[k + move][j][i][1] = 0;
							move++;
							flag++;
						}
					}


				}


			}
			move = 0;
		}
	}
	if (flag>0)
		SetRandom();
	Sleep(500);  //防止此按键多次响应
}
void MoveZF()
{
	int flag = 0;
	int swap = 0;
	int move = 0;
	for (int i = 3; i >-1; i--)
	{
		for (int j = 0; j < 4; j++)
		{

			for (int k = 0; k < 4; k++)//根据方向从左向右或从右向左依次遍历
			{
				if (cubesite[k][j][i][1] == 0)//当当前位置的贴图为0时，表示为空（透明的弄不出来），不发生换图，累计空的个数move
				{
					move++;
				}
				if (cubesite[k][j][i][1] > 0)//当当前位置的贴图不为0时，和前面move个位置的方块交换贴图
				{
					swap = cubesite[k][j][i][1];
					cubesite[k][j][i][1] = cubesite[k - move][j][i][1];
					cubesite[k - move][j][i][1] = swap;
					if (move != 0)
					{
						flag++;
					}
					if (k>0 && (k - move)>0)//交换完后，判断和左侧相邻图片贴图相不相同，相同则左侧位置方块贴图数字换位原来两倍，自己清零
					{
						if (cubesite[k - move][j][i][1] == cubesite[k - move - 1][j][i][1])
						{
							cubesite[k - move - 1][j][i][1]++;//贴图数字为了和图片文件路径对应，用2的次方数表达
							cubesite[k - move][j][i][1] = 0;
							move++;
							flag++;
						}
					}


				}


			}
			move = 0;
		}
	}
	if (flag>0)
		SetRandom();
	Sleep(500);  //防止此按键多次响应
}
void MoveXZ()
{
	int flag = 0;
	int swap = 0;
	int move = 0;
	for (int i = 3; i >-1; i--)
	{
		for (int j = 0; j < 4; j++)
		{

			for (int k = 3; k >-1; k--)//根据方向从左向右或从右向左依次遍历
			{
				if (cubesite[i][j][k][1] == 0)// 当当前位置的贴图为0时，表示为空（透明的弄不出来），不发生换图，累计空的个数move
				{
					move++;
				}
				if (cubesite[i][j][k][1] > 0)//当当前位置的贴图不为0时，和前面move个位置的方块交换贴图
				{
					swap = cubesite[i][j][k][1];
					cubesite[i][j][k][1] = cubesite[i][j][k + move][1];
					cubesite[i][j][k + move][1] = swap;
					if (move != 0)
					{
						flag++;
					}
					if (k<3 && (k + move)<3)//交换完后，判断和左侧相邻图片贴图相不相同，相同则左侧位置方块贴图数字换位原来两倍，自己清零
					{
						if (cubesite[i][j][k + move][1] == cubesite[i][j][k + move + 1][1])
						{
							cubesite[i][j][k + move + 1][1]++;
							cubesite[i][j][k + move][1] = 0;
							move++;
							flag++;
						}
					}


				}
			}
			move = 0;
		}
	}
	if (flag>0)
		SetRandom();
	Sleep(500);  //防止此按键多次响应

}
void MoveXF()
{
	int flag = 0;
	int swap = 0;
	int move = 0;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{

			for (int k = 0; k < 4; k++)//根据方向从左向右或从右向左依次遍历
			{
				if (cubesite[i][j][k][1] == 0)//当当前位置的贴图为0时，表示为空（透明的弄不出来），不发生换图，累计空的个数move
				{
					move++;
				}
				if (cubesite[i][j][k][1] > 0)//当当前位置的贴图不为0时，和前面move个位置的方块交换贴图
				{
					swap = cubesite[i][j][k][1];
					cubesite[i][j][k][1] = cubesite[i][j][k - move][1];
					cubesite[i][j][k - move][1] = swap;
					if (move != 0)
					{
						flag++;
					}
					if (k>0 && (k - move)>0)//交换完后，判断和左侧相邻图片贴图相不相同，相同则左侧位置方块贴图数字换位原来两倍，自己清零
					{
						if (cubesite[i][j][k - move][1] == cubesite[i][j][k - move - 1][1])
						{
							cubesite[i][j][k - move - 1][1]++;//贴图数字为了和图片文件路径对应，用2的次方数表达
							cubesite[i][j][k - move][1] = 0;
							move++;
							flag++;
						}
					}


				}


			}
			move = 0;
		}
	}
	if (flag>0)
		SetRandom();
	Sleep(500);  //防止此按键多次响应
}
void MoveYZ()
{
	int flag = 0;
	int swap = 0;
	int move = 0;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{

			for (int k = 0; k < 4; k++)//根据方向从左向右或从右向左依次遍历
			{
				if (cubesite[i][k][j][1] == 0)//当当前位置的贴图为0时，表示为空（透明的弄不出来），不发生换图，累计空的个数move
				{
					move++;
				}
				if (cubesite[i][k][j][1] > 0)//当当前位置的贴图不为0时，和前面move个位置的方块交换贴图
				{
					swap = cubesite[i][k][j][1];
					cubesite[i][k][j][1] = cubesite[i][k - move][j][1];
					cubesite[i][k - move][j][1] = swap;
					if (move != 0)
					{
						flag++;
					}
					if (k>0 && (k - move)>0)//交换完后，判断和左侧相邻图片贴图相不相同，相同则左侧位置方块贴图数字换位原来两倍，自己清零
					{
						if (cubesite[i][k - move][j][1] == cubesite[i][k - move - 1][j][1])
						{
							cubesite[i][k - move - 1][j][1]++;//贴图数字为了和图片文件路径对应，用2的次方数表达
							cubesite[i][k - move][j][1] = 0;
							move++;
							flag++;
						}
					}


				}


			}
			move = 0;
		}
	}
	if (flag>0)
		SetRandom();
	Sleep(500);  //防止此按键多次响应
}
void MoveYF()
{
	int flag = 0;
	int swap = 0;
	int move = 0;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{

			for (int k = 3; k >-1; k--)//根据方向从左向右或从右向左依次遍历
			{
				if (cubesite[i][k][j][1] == 0)//当当前位置的贴图为0时，表示为空（透明的弄不出来），不发生换图，累计空的个数move
				{
					move++;
				}
				if (cubesite[i][k][j][1] > 0)//当当前位置的贴图不为0时，和前面move个位置的方块交换贴图
				{
					swap = cubesite[i][k][j][1];
					cubesite[i][k][j][1] = cubesite[i][k + move][j][1];
					cubesite[i][k + move][j][1] = swap;
					if (move != 0)
					{
						flag++;
					}
					if (k<3 && (k + move)<3)//交换完后，判断和左侧相邻图片贴图相不相同，相同则左侧位置方块贴图数字换位原来两倍，自己清零
					{
						if (cubesite[i][k + move][j][1] == cubesite[i][k + move + 1][j][1])
						{
							cubesite[i][k + move + 1][j][1]++;//贴图数字为了和图片文件路径对应，用2的次方数表达
							cubesite[i][k + move][j][1] = 0;
							move++;
							flag++;
						}
					}


				}
			}
			move = 0;
		}
	}
	if (flag>0)
		SetRandom();
	Sleep(500);  //防止此按键多次响应
}
//**************以下为框架函数******************
bool Setup()
{
	//这里主要包含5个主要步骤
	//第一步载入外部文件（包括fx文件及图像文件）
	//第二步创建各种渲染状态
	//第三步创建输入布局
	//第四步创建顶点缓存
	//*************第一步载入外部文件（包括fx文件及图像文件）****************************
	HRESULT hr = S_OK;              //声明HRESULT的对象用于记录函数调用是否成功
	ID3DBlob* pTechBlob = NULL;     //声明ID3DBlob的对象用于存放从文件读取的信息
	//从我们之前建立的.fx文件读取着色器相关信息
	hr = D3DX11CompileFromFile(L"Shader.fx", NULL, NULL, NULL, "fx_5_0",
		D3DCOMPILE_ENABLE_STRICTNESS, 0, NULL, &pTechBlob, NULL, NULL);
	if (FAILED(hr))
	{
		::MessageBox(NULL, L"fx文件载入失败", L"Error", MB_OK); //如果读取失败，弹出错误信息
		return hr;
	}
	//调用D3DX11CreateEffectFromMemory创建ID3DEffect对象
	hr = D3DX11CreateEffectFromMemory(pTechBlob->GetBufferPointer(),
		pTechBlob->GetBufferSize(), 0, device, &effect);

	if (FAILED(hr))
	{
		::MessageBox(NULL, L"创建Effect失败", L"Error", MB_OK);  //创建失败，弹出错误信息
		return hr;
	}
	//从外部图像文件载入纹理
	//纹理
	D3DX11CreateShaderResourceViewFromFile(device, L"bmp/00.png", NULL, NULL, &texture[0], NULL);
	D3DX11CreateShaderResourceViewFromFile(device, L"bmp/2.bmp", NULL, NULL, &texture[1], NULL);
	D3DX11CreateShaderResourceViewFromFile(device, L"bmp/4.bmp", NULL, NULL, &texture[2], NULL);
	D3DX11CreateShaderResourceViewFromFile(device, L"bmp/8.bmp", NULL, NULL, &texture[3], NULL);
	D3DX11CreateShaderResourceViewFromFile(device, L"bmp/16.bmp", NULL, NULL, &texture[4], NULL);
	D3DX11CreateShaderResourceViewFromFile(device, L"bmp/32.bmp", NULL, NULL, &texture[5], NULL);
	D3DX11CreateShaderResourceViewFromFile(device, L"bmp/64.bmp", NULL, NULL, &texture[6], NULL);
	D3DX11CreateShaderResourceViewFromFile(device, L"bmp/128.bmp", NULL, NULL, &texture[7], NULL);
	D3DX11CreateShaderResourceViewFromFile(device, L"bmp/256.bmp", NULL, NULL, &texture[8], NULL);
	D3DX11CreateShaderResourceViewFromFile(device, L"bmp/512.bmp", NULL, NULL, &texture[9], NULL);
	D3DX11CreateShaderResourceViewFromFile(device, L"bmp/1024.bmp", NULL, NULL, &texture[10], NULL);
	D3DX11CreateShaderResourceViewFromFile(device, L"bmp/2048.bmp", NULL, NULL, &texture[11], NULL);
	D3DX11CreateShaderResourceViewFromFile(device, L"bmp/4096.bmp", NULL, NULL, &texture[12], NULL);

	D3DX11CreateShaderResourceViewFromFile(device, L"Goal.png", NULL, NULL, &textureGoal, NULL);
	D3DX11CreateShaderResourceViewFromFile(device, L"Miss.png", NULL, NULL, &textureMiss, NULL);
	D3DX11CreateShaderResourceViewFromFile(device, L"background.png", NULL, NULL, &textureBackground, NULL);
	//*************第一步载入外部文件（包括fx文件及图像文件）****************************

	////*************第二步创建各种渲染状态************************************************

	//关闭背面消隐
	D3D11_RASTERIZER_DESC ncDesc;        //光栅器描述
	ZeroMemory(&ncDesc, sizeof(ncDesc));  //清零操作
	ncDesc.CullMode = D3D11_CULL_NONE;   //剔除特定朝向的三角形，这里不剔除，即全部绘制
	ncDesc.FillMode = D3D11_FILL_SOLID;  //填充模式，这里为利用三角形填充
	ncDesc.FrontCounterClockwise = false;//是否设置逆时针绕续的三角形为正面
	ncDesc.DepthClipEnable = true;       //开启深度裁剪
	//创建一个关闭背面消隐的状态，在需要用的时候才设置给设备上下文
	if (FAILED(device->CreateRasterizerState(&ncDesc, &NoCullRS)))
	{
		MessageBox(NULL, L"Create 'NoCull' rasterizer state failed!", L"Error", MB_OK);
		return false;
	}


	////*************第二步创建各种渲染状态************************************************

	//*************第三步创建输入布局****************************************************
	//用GetTechniqueByName获取ID3DX11EffectTechnique的对象
	//先设置默认的technique到Effect
	technique = effect->GetTechniqueByName("TexTech");                //默认Technique

	//D3DX11_PASS_DESC结构用于描述一个Effect Pass
	D3DX11_PASS_DESC PassDesc;
	//利用GetPassByIndex获取Effect Pass
	//再利用GetDesc获取Effect Pass的描述，并存如PassDesc对象中
	technique->GetPassByIndex(0)->GetDesc(&PassDesc);

	//创建并设置输入布局
	//这里我们定义一个D3D11_INPUT_ELEMENT_DESC数组，
	//由于我们定义的顶点结构包括位置坐标和法向量，所以这个数组里有两个元素
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	//layout元素个数
	UINT numElements = ARRAYSIZE(layout);
	//调用CreateInputLayout创建输入布局
	hr = device->CreateInputLayout(layout, numElements, PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize, &vertexLayout);
	//设置生成的输入布局到设备上下文中
	immediateContext->IASetInputLayout(vertexLayout);
	if (FAILED(hr))
	{
		::MessageBox(NULL, L"创建Input Layout失败", L"Error", MB_OK);
		return hr;
	}
	//*************第三步创建输入布局****************************************************

	//*************第四步创建顶点缓存****************************************************
	//这里需要定义箱子，池子，以及水面的顶点
	Vertex vertices[] =
	{
		//1-1-1
		//后面
		{ XMFLOAT3(-2.75f, 2.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(-1.75f, 2.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, 1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(-1.75f, 2.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(-2.75f, 2.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(-2.75f, 2.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(-2.75f, 1.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//1-1-2
		//后面
		{ XMFLOAT3(-1.25f, 2.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(-0.25f, 2.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, 1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(-0.25f, 2.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(-1.25f, 2.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(-1.25f, 2.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(-1.25f, 1.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//1-1-3
		//后面
		{ XMFLOAT3(0.25f, 2.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(1.25f, 2.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, 1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(1.25f, 2.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(0.25f, 2.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, 2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(0.25f, 2.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 2.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(0.25f, 1.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//1-1-4
		//后面
		{ XMFLOAT3(1.75f, 2.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(2.75f, 2.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, 1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(2.75f, 2.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(1.75f, 2.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, 2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(1.75f, 2.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 2.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(1.75f, 1.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//1-2-1
		//后面
		{ XMFLOAT3(-2.75f, 1.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(-1.75f, 1.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, 0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(-1.75f, 1.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(-2.75f, 1.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(-2.75f, 1.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(-2.75f, 0.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//1-2-2
		//后面
		{ XMFLOAT3(-1.25f, 1.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(-0.25f, 1.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, 0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(-0.25f, 1.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(-1.25f, 1.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(-1.25f, 1.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(-1.25f, 0.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },

		//1-2-3
		//后面
		{ XMFLOAT3(0.25f, 1.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(1.25f, 1.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, 0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(1.25f, 1.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(0.25f, 1.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, 1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(0.25f, 1.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(0.25f, 0.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//1-2-4
		//后面
		{ XMFLOAT3(1.75f, 1.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(2.75f, 1.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, 0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(2.75f, 1.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(1.75f, 1.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, 1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(1.75f, 1.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(1.75f, 0.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//1-3-1
		//后面
		{ XMFLOAT3(-2.75f, -0.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(-1.75f, -0.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, -1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(-1.75f, -0.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(-2.75f, -0.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(-2.75f, -0.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(-2.75f, -1.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//1-3-2
		//后面
		{ XMFLOAT3(-1.25f, -0.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(-0.25f, -0.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, -1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(-0.25f, -0.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(-1.25f, -0.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(-1.25f, -0.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(-1.25f, -1.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//1-3-3
		//后面
		{ XMFLOAT3(0.25f, -0.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(1.25f, -0.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, -1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(1.25f, -0.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(0.25f, -0.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, -0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(0.25f, -0.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -0.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(0.25f, -1.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//1-3-4
		//后面
		{ XMFLOAT3(1.75f, -0.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(2.75f, -0.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, -1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(2.75f, -0.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(1.75f, -0.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, -0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(1.75f, -0.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -0.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(1.75f, -1.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//1-4-1
		//后面
		{ XMFLOAT3(-2.75f, -1.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(-1.75f, -1.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, -2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(-1.75f, -1.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(-2.75f, -1.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(-2.75f, -1.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(-2.75f, -2.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//1-4-2
		//后面
		{ XMFLOAT3(-1.25f, -1.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(-0.25f, -1.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, -2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(-0.25f, -1.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(-1.25f, -1.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(-1.25f, -1.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(-1.25f, -2.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//1-4-3
		//后面
		{ XMFLOAT3(0.25f, -1.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(1.25f, -1.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, -2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(1.25f, -1.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(0.25f, -1.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, -1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(0.25f, -1.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(0.25f, -2.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//1-4-4
		//后面
		{ XMFLOAT3(1.75f, -1.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(2.75f, -1.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, -2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(2.75f, -1.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(1.75f, -1.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, -1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(1.75f, -1.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(1.75f, -2.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//2-1-1
		//后面
		{ XMFLOAT3(-2.75f, 2.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(-1.75f, 2.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, 1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(-1.75f, 2.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(-2.75f, 2.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(-2.75f, 2.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(-2.75f, 1.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//2-1-2
		//后面
		{ XMFLOAT3(-1.25f, 2.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(-0.25f, 2.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, 1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(-0.25f, 2.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(-1.25f, 2.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(-1.25f, 2.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(-1.25f, 1.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//2-1-3
		//后面
		{ XMFLOAT3(0.25f, 2.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(1.25f, 2.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, 1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(1.25f, 2.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(0.25f, 2.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, 2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(0.25f, 2.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 2.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(0.25f, 1.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//2-1-4
		//后面
		{ XMFLOAT3(1.75f, 2.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(2.75f, 2.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, 1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(2.75f, 2.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(1.75f, 2.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, 2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(1.75f, 2.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 2.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(1.75f, 1.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//2-2-1
		//后面
		{ XMFLOAT3(-2.75f, 1.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(-1.75f, 1.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, 0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(-1.75f, 1.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(-2.75f, 1.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(-2.75f, 1.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(-2.75f, 0.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//2-2-2
		//后面
		{ XMFLOAT3(-1.25f, 1.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(-0.25f, 1.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, 0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(-0.25f, 1.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(-1.25f, 1.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(-1.25f, 1.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(-1.25f, 0.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//2-2-3
		//后面
		{ XMFLOAT3(0.25f, 1.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(1.25f, 1.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, 0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(1.25f, 1.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(0.25f, 1.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, 1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(0.25f, 1.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(0.25f, 0.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//2-2-4
		//后面
		{ XMFLOAT3(1.75f, 1.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(2.75f, 1.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, 0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(2.75f, 1.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(1.75f, 1.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, 1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(1.75f, 1.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(1.75f, 0.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//2-3-1
		//后面
		{ XMFLOAT3(-2.75f, -0.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(-1.75f, -0.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, -1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(-1.75f, -0.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(-2.75f, -0.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(-2.75f, -0.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(-2.75f, -1.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//2-3-2
		//后面
		{ XMFLOAT3(-1.25f, -0.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(-0.25f, -0.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, -1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(-0.25f, -0.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(-1.25f, -0.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(-1.25f, -0.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(-1.25f, -1.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//2-3-3
		//后面
		{ XMFLOAT3(0.25f, -0.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(1.25f, -0.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, -1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(1.25f, -0.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(0.25f, -0.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, -0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(0.25f, -0.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -0.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(0.25f, -1.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//2-3-4
		//后面
		{ XMFLOAT3(1.75f, -0.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(2.75f, -0.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, -1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(2.75f, -0.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(1.75f, -0.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, -0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(1.75f, -0.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -0.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(1.75f, -1.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//2-4-1
		//后面
		{ XMFLOAT3(-2.75f, -1.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(-1.75f, -1.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, -2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(-1.75f, -1.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(-2.75f, -1.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(-2.75f, -1.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(-2.75f, -2.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//2-4-2
		//后面
		{ XMFLOAT3(-1.25f, -1.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(-0.25f, -1.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, -2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(-0.25f, -1.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(-1.25f, -1.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(-1.25f, -1.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(-1.25f, -2.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//2-4-3
		//后面
		{ XMFLOAT3(0.25f, -1.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(1.25f, -1.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, -2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(1.25f, -1.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(0.25f, -1.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, -1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(0.25f, -1.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(0.25f, -2.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//2-4-4
		//后面
		{ XMFLOAT3(1.75f, -1.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(2.75f, -1.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, -2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(2.75f, -1.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(1.75f, -1.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, -1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(1.75f, -1.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(1.75f, -2.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//*******(3,1,1)********
		//后面
		{ XMFLOAT3(-2.75f, 2.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(-1.75f, 2.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, 1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(-1.75f, 2.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(-2.75f, 2.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(-2.75f, 2.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(-2.75f, 1.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//*******(3,1,2)********
		//后面
		{ XMFLOAT3(-1.25f, 2.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(-0.25f, 2.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, 1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(-0.25f, 2.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(-1.25f, 2.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(-1.25f, 2.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(-1.25f, 1.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//*******(3,1,3)********
		//后面
		{ XMFLOAT3(0.25f, 2.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(1.25f, 2.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, 1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(1.25f, 2.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(0.25f, 2.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, 2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(0.25f, 2.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 2.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(0.25f, 1.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//*******(3,1,4)********
		//后面
		{ XMFLOAT3(1.75f, 2.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(2.75f, 2.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, 1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(2.75f, 2.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(1.75f, 2.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, 2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(1.75f, 2.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 2.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(1.75f, 1.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//*******(3,2,1)********
		//后面
		{ XMFLOAT3(-2.75f, 1.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(-1.75f, 1.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, 0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(-1.75f, 1.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(-2.75f, 1.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(-2.75f, 1.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(-2.75f, 0.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//*******(3,2,2)********
		//后面
		{ XMFLOAT3(-1.25f, 1.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(-0.25f, 1.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, 0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(-0.25f, 1.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(-1.25f, 1.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(-1.25f, 1.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(-1.25f, 0.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//*******(3,2,3)********
		//后面
		{ XMFLOAT3(0.25f, 1.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(1.25f, 1.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, 0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(1.25f, 1.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(0.25f, 1.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, 1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(0.25f, 1.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(0.25f, 0.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//*******(3,2,4)********
		//后面
		{ XMFLOAT3(1.75f, 1.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(2.75f, 1.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, 0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(2.75f, 1.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(1.75f, 1.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, 1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(1.75f, 1.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(1.75f, 0.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//*******(3,3,1)********
		//后面
		{ XMFLOAT3(-2.75f, -0.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(-1.75f, -0.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, -1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(-1.75f, -0.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(-2.75f, -0.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(-2.75f, -0.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(-2.75f, -1.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//*******(3,3,2)********
		//后面
		{ XMFLOAT3(-1.25f, -0.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(-0.25f, -0.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, -1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(-0.25f, -0.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(-1.25f, -0.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(-1.25f, -0.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(-1.25f, -1.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },

		//*******(3,3,3)********
		//后面
		{ XMFLOAT3(0.25f, -0.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(1.25f, -0.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, -1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(1.25f, -0.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(0.25f, -0.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, -0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(0.25f, -0.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -0.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(0.25f, -1.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//*******(3,3,4)********
		//后面
		{ XMFLOAT3(1.75f, -0.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(2.75f, -0.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, -1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(2.75f, -0.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(1.75f, -0.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, -0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(1.75f, -0.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -0.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(1.75f, -1.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//*******(3,4,1)********
		//后面
		{ XMFLOAT3(-2.75f, -1.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(-1.75f, -1.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, -2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(-1.75f, -1.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(-2.75f, -1.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(-2.75f, -1.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(-2.75f, -2.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//*******(3,4,2)********
		//后面
		{ XMFLOAT3(-1.25f, -1.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(-0.25f, -1.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, -2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(-0.25f, -1.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(-1.25f, -1.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(-1.25f, -1.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(-1.25f, -2.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },

		//*******(3,4,3)********
		//后面
		{ XMFLOAT3(0.25f, -1.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(1.25f, -1.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, -2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(1.25f, -1.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(0.25f, -1.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, -1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(0.25f, -1.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(0.25f, -2.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },

		//*******(3,4,4)********
		//后面
		{ XMFLOAT3(1.75f, -1.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(2.75f, -1.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, -2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(2.75f, -1.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(1.75f, -1.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, -1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(1.75f, -1.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(1.75f, -2.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },

		//*******(4,1,1)********
		//后面
		{ XMFLOAT3(-2.75f, 2.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(-1.75f, 2.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, 1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(-1.75f, 2.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(-2.75f, 2.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(-2.75f, 2.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(-2.75f, 1.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//*******(4,1,2)********
		//后面
		{ XMFLOAT3(-1.25f, 2.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(-0.25f, 2.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, 1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(-0.25f, 2.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(-1.25f, 2.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(-1.25f, 2.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(-1.25f, 1.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//*******(4,1,3)********
		//后面
		{ XMFLOAT3(0.25f, 2.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(1.25f, 2.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, 1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(1.25f, 2.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(0.25f, 2.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, 2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(0.25f, 2.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 2.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(0.25f, 1.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//*******(4,1,4)********
		//后面
		{ XMFLOAT3(1.75f, 2.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(2.75f, 2.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, 1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(2.75f, 2.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(1.75f, 2.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, 2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(1.75f, 2.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 2.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(1.75f, 1.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//*******(4,2,1)********
		//后面
		{ XMFLOAT3(-2.75f, 1.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(-1.75f, 1.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, 0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(-1.75f, 1.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(-2.75f, 1.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(-2.75f, 1.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(-2.75f, 0.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//*******(4,2,2)********
		//后面
		{ XMFLOAT3(-1.25f, 1.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(-0.25f, 1.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, 0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(-0.25f, 1.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(-1.25f, 1.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(-1.25f, 1.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(-1.25f, 0.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//*******(4,2,3)********
		//后面
		{ XMFLOAT3(0.25f, 1.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(1.25f, 1.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, 0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(1.25f, 1.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(0.25f, 1.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, 1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(0.25f, 1.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(0.25f, 0.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//*******(4,2,4)********
		//后面
		{ XMFLOAT3(1.75f, 1.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(2.75f, 1.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, 0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(2.75f, 1.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(1.75f, 1.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, 1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(1.75f, 1.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(1.75f, 0.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//*******(4,3,1)********
		//后面
		{ XMFLOAT3(-2.75f, -0.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(-1.75f, -0.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, -1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(-1.75f, -0.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(-2.75f, -0.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(-2.75f, -0.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(-2.75f, -1.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//*******(4,3,2)********
		//后面
		{ XMFLOAT3(-1.25f, -0.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(-0.25f, -0.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, -1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(-0.25f, -0.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(-1.25f, -0.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(-1.25f, -0.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(-1.25f, -1.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },

		//*******(4,3,3)********
		//后面
		{ XMFLOAT3(0.25f, -0.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(1.25f, -0.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, -1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(1.25f, -0.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(0.25f, -0.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, -0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(0.25f, -0.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -0.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(0.25f, -1.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//*******(4,3,4)********
		//后面
		{ XMFLOAT3(1.75f, -0.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(2.75f, -0.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, -1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(2.75f, -0.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(1.75f, -0.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, -0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(1.75f, -0.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -0.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(1.75f, -1.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//*******(4,4,1)********
		//后面
		{ XMFLOAT3(-2.75f, -1.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(-1.75f, -1.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, -2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(-1.75f, -1.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(-2.75f, -1.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(-2.75f, -1.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(-2.75f, -2.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//*******(4,4,2)********
		//后面
		{ XMFLOAT3(-1.25f, -1.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(-0.25f, -1.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, -2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(-0.25f, -1.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(-1.25f, -1.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(-1.25f, -1.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(-1.25f, -2.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//*******(4,4,3)********
		//后面
		{ XMFLOAT3(0.25f, -1.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(1.25f, -1.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, -2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(1.25f, -1.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(0.25f, -1.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, -1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(0.25f, -1.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(0.25f, -2.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//*******(4,4,4)********
		//后面
		{ XMFLOAT3(1.75f, -1.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//右面
		{ XMFLOAT3(2.75f, -1.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, -2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//前面
		{ XMFLOAT3(2.75f, -1.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//左面
		{ XMFLOAT3(1.75f, -1.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, -1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//上面
		{ XMFLOAT3(1.75f, -1.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//下面
		{ XMFLOAT3(1.75f, -2.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },


		{ XMFLOAT3(1.75f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },


		/*
		//Goal  or  Miss
		{ XMFLOAT3(-11.8f, 11.8f, 2.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(11.8f, 11.8f, 2.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-11.8f, -11.8f, 2.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(11.8f, 11.8f, 2.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(11.8f, -11.8, 2.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-11.8f, -11.8, 2.0f), XMFLOAT2(0.0f, 1.0f) },
		*/

		//Goal  or  Miss
		//前面
		{ XMFLOAT3(-8.0f, 8.0f, 8.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(8.0f, 8.0f, 8.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-8.0f, -8.0f, 8.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(8.0f, 8.0f, 8.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(8.0f, -8.0f, 8.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-8.0f, -8.0f, 8.0f), XMFLOAT2(0.0f, 1.0f) },

		//Goal  or  Miss
		//后面
		{ XMFLOAT3(-8.0f, 8.0f, -8.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(8.0f, 8.0f, -8.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-8.0f, -8.0f, -8.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(8.0f, 8.0f, -8.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(8.0f, -8.0f, -8.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-8.0f, -8.0f, -8.0f), XMFLOAT2(0.0f, 1.0f) },

		//Goal  or  Miss
		//左面
		{ XMFLOAT3(-8.0f, 8.0f, 8.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-8.0f, 8.0f, -8.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-8.0f, -8.0f, 8.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-8.0f, 8.0f, -8.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-8.0f, -8.0f, -8.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-8.0f, -8.0f, 8.0f), XMFLOAT2(0.0f, 1.0f) },

		//Goal  or  Miss
		//右面
		{ XMFLOAT3(8.0f, 8.0f, 8.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(8.0f, 8.0f, -8.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(8.0f, -8.0f, 8.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(8.0f, 8.0f, -8.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(8.0f, -8.0f, -8.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(8.0f, -8.0f, 8.0f), XMFLOAT2(0.0f, 1.0f) },

		//Goal  or  Miss
		//上面
		{ XMFLOAT3(-8.0f, 8.0f, 8.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(8.0f, 8.0f, 8.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-8.0f, 8.0f, -8.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(8.0f, 8.0f, 8.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(8.0f, 8.0f, -8.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-8.0f, 8.0f, -8.0f), XMFLOAT2(0.0f, 1.0f) },

		//Goal  or  Miss
		//下面
		{ XMFLOAT3(-8.0f, -8.0f, 8.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(8.0f, -8.0f, 8.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-8.0f, -8.0f, -8.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(8.0f, -8.0f, 8.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(8.0f, -8.0f, -8.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-8.0f, -8.0f, -8.0f), XMFLOAT2(0.0f, 1.0f) },




	};
	UINT vertexCount = ARRAYSIZE(vertices);
	//创建顶点缓存，方法同实验4一样
	//首先声明一个D3D11_BUFFER_DESC的对象bd
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex)* vertexCount;      //注意：由于这里定义了24个顶点所以要乘以24
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;  //注意：这里表示创建的是顶点缓存
	bd.CPUAccessFlags = 0;

	//声明一个D3D11_SUBRESOURCE_DATA数据用于初始化子资源
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;         //设置需要初始化的数据，这里的数据就是顶点数组
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	//声明一个ID3D11Buffer对象作为顶点缓存
	ID3D11Buffer* vertexBuffer;
	//调用CreateBuffer创建顶点缓存
	hr = device->CreateBuffer(&bd, &InitData, &vertexBuffer);
	if (FAILED(hr))
	{
		::MessageBox(NULL, L"创建VertexBuffer失败", L"Error", MB_OK);
		return hr;
	}

	UINT stride = sizeof(Vertex);                 //获取Vertex的大小作为跨度
	UINT offset = 0;                              //设置偏移量为0
	//设置顶点缓存，参数的解释见实验4
	immediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	//指定图元类型，D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST表示图元为三角形
	immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//*************第四步创建顶点缓存****************************************************
	return true;
}

void Cleanup()
{
	//释放全局指针
	if (renderTargetView) renderTargetView->Release();
	if (immediateContext) immediateContext->Release();
	if (swapChain) swapChain->Release();
	if (device) device->Release();

	if (vertexLayout) vertexLayout->Release();
	if (effect) effect->Release();

	if (depthStencilView) depthStencilView->Release();
	if (texture[0]) texture[0]->Release();
	if (texture[1]) texture[1]->Release();
	if (texture[2]) texture[2]->Release();
	if (texture[3]) texture[3]->Release();
	if (texture[4]) texture[4]->Release();
	if (texture[5]) texture[5]->Release();
	if (texture[6]) texture[6]->Release();
	if (texture[7]) texture[7]->Release();
	if (texture[8]) texture[8]->Release();
	if (texture[9]) texture[9]->Release();
	if (texture[10]) texture[10]->Release();
	if (texture[11]) texture[11]->Release();
	if (texture[12]) texture[12]->Release();
	if (textureGoal) textureGoal->Release();
	if (textureMiss) textureMiss->Release();
	if (textureBackground) textureBackground->Release();


	//if (blendStateAlpha) blendStateAlpha->Release();
	//if (NoCullRS) NoCullRS->Release();

}

bool Display(float timeDelta)
{
	if (device)
	{
		//声明一个数组存放颜色信息，4个元素分别表示红，绿，蓝以及alpha
		float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
		immediateContext->ClearRenderTargetView(renderTargetView, ClearColor);
		//清除当前绑定的深度模板视图
		immediateContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
		//先指定混合因子，一般不用它，除非在上面混合因子指定为使用blend factor
		float BlendFactor[] = { 0, 0, 0, 0 };

		//********************第一部分：设置3个坐标系及光照的外部变量****************************
		//通过键盘的上下左右键来改变虚拟摄像头方向.



		if (Lbtndown) //响应键盘左方向键
		{
			angle -= 3.0f * XM_PI / 2000;
			dangle += 3.0f * XM_PI / 2000;

		}
		if (Rbtndown) //响应键盘右方向键
		{
			angle += 3.0f * XM_PI / 2000;
			dangle += 3.0f * XM_PI / 2000;

		}
		if (Ubtndown)    //响应键盘上方向键
		{
			height += 15.0f * timeDelta;
			dheight += 15.0f * timeDelta;
		}
		if (Dbtndown)  //响应键盘下方向键
		{
			height -= 15.0f * timeDelta;
			dheight += 15.0f * timeDelta;
		}

		if (dangle >= XM_PI / 2)
		{
			Lbtndown = false;
			Rbtndown = false;
			dangle = 0;
		}
		if (dheight >= 8.0f)
		{
			Dbtndown = false;
			Ubtndown = false;
			dheight = 0;
		}
		if (height >= 8.0f) height = 8.0;
		if (height <= -8.0f) height = -8.0;

		//初始化世界矩阵
		world = XMMatrixIdentity();
		XMVECTOR Eye = XMVectorSet(cosf(angle) * 6.0f, height, sinf(angle) * 6.0f, 0.0f);//相机位置
		XMVECTOR At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);  //目标位置
		XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);  //up
		view = XMMatrixLookAtLH(Eye, At, Up);   //设置观察坐标系
		//设置投影矩阵
		projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, 800.0f / 600.0f, 0.01f, 100.0f);
		//将坐标变换矩阵的常量缓存中的矩阵和坐标设置到Effect框架中---------------------
		effect->GetVariableByName("World")->AsMatrix()->SetMatrix((float*)&world);  //设置世界坐标系
		effect->GetVariableByName("View")->AsMatrix()->SetMatrix((float*)&view);    //设置观察坐标系
		effect->GetVariableByName("Projection")->AsMatrix()->SetMatrix((float*)&projection); //设置投影坐标系
		effect->GetVariableByName("EyePosition")->AsMatrix()->SetMatrix((float*)&Eye); //设置视点
		//********************第一部分：设置3个坐标系及光照的外部变量****************************

		//定义一个D3DX11_TECHNIQUE_DESC对象来描述technique
		D3DX11_TECHNIQUE_DESC techDesc;
		technique->GetDesc(&techDesc);    //获取technique的描述
		//获取通道（PASS）把它设置到设备上下文中。
		//这里由于只有一个通道所以其索引为0
		technique->GetPassByIndex(0)->Apply(0, immediateContext);


		if (::GetAsyncKeyState('A') & 0x8000f) //响应键盘左方向键
		{

			if (flag1 == 1)
			{
				MoveXF();
			}

			if (flag1 == 2)
			{
				MoveZZ();
			}
			if (flag1 == 3)
			{
				MoveXZ();
			}
			if (flag1 == 4)
			{
				MoveZF();
			}


		}

		if (::GetAsyncKeyState('D') & 0x8000f) //响应键盘右方向键
		{
			if (flag1 == 1)
			{
				MoveXZ();
			}

			if (flag1 == 2)
			{
				MoveZF();
			}

			if (flag1 == 3)
			{
				MoveXF();
			}

			if (flag1 == 4)
			{
				MoveZZ();
			}


		}
		if (::GetAsyncKeyState('S') & 0x8000f) //响应键盘左方向键
		{
			if (height <3 && height >-3)
			{
				MoveYF();
			}
			if (height >= 3)
			{
				if (flag1 == 1)
				{
					MoveZF();
				}
				if (flag1 == 2)
				{
					MoveXF();
				}
				if (flag1 == 3)
				{
					MoveZZ();
				}
				if (flag1 == 4)
				{
					MoveXZ();
				}

			}
			if (height <= -3)
			{
				if (flag1 == 1)
				{
					MoveZZ();
				}
				if (flag1 == 2)
				{
					MoveXZ();
				}
				if (flag1 == 3)
				{
					MoveZF();
				}
				if (flag1 == 4)
				{
					MoveXF();
				}
			}




		}
		if (::GetAsyncKeyState('W') & 0x8000f) //响应键盘右方向键
		{
			if (height <3 && height >-3)
			{
				MoveYZ();
			}
			if (height >= 3)
			{
				if (flag1 == 1)
				{
					MoveZZ();
				}
				if (flag1 == 2)
				{
					MoveXZ();
				}
				if (flag1 == 3)
				{
					MoveZF();
				}
				if (flag1 == 4)
				{
					MoveXF();
				}

			}
			if (height <= -3)
			{
				if (flag1 == 1)
				{
					MoveZF();
				}
				if (flag1 == 2)
				{
					MoveXF();
				}
				if (flag1 == 3)
				{
					MoveZZ();
				}
				if (flag1 == 4)
				{
					MoveXZ();
				}
			}

		}


		if (!goSucceed&&!goDie)
		{
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					for (int k = 0; k < 4; k++)
					{

						world = XMMatrixIdentity();
						effect->GetVariableByName("World")->AsMatrix()->SetMatrix((float*)&world);  //设置世界坐标系
						effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(texture[cubesite[i][j][k][1]]);
						technique->GetPassByIndex(0)->Apply(0, immediateContext);
						immediateContext->Draw(36, cubesite[i][j][k][0]);   //绘制立方体



					}
				}
			}
		}
		else if (goSucceed)
		{

			immediateContext->RSSetState(NoCullRS);
			angle = -XM_PI / 2;
			height = 0.0f;
			flag1 = 1;
			effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(textureGoal);
			technique->GetPassByIndex(0)->Apply(0, immediateContext);
			immediateContext->Draw(6, 2304);

		}
		else if (goDie)
		{

			immediateContext->RSSetState(NoCullRS);
			angle = -XM_PI / 2;
			height = 0.0f;
			flag1 = 1;
			effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(textureMiss);
			technique->GetPassByIndex(0)->Apply(0, immediateContext);
			immediateContext->Draw(6, 2304);

		}

		//生成背景

		immediateContext->RSSetState(NoCullRS);
		effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(textureBackground);
		technique->GetPassByIndex(0)->Apply(0, immediateContext);
		immediateContext->Draw(6, 2304);

		immediateContext->RSSetState(NoCullRS);
		effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(textureBackground);
		technique->GetPassByIndex(0)->Apply(0, immediateContext);
		immediateContext->Draw(6, 2310);

		immediateContext->RSSetState(NoCullRS);
		effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(textureBackground);
		technique->GetPassByIndex(0)->Apply(0, immediateContext);
		immediateContext->Draw(6, 2316);

		immediateContext->RSSetState(NoCullRS);
		effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(textureBackground);
		technique->GetPassByIndex(0)->Apply(0, immediateContext);
		immediateContext->Draw(6, 2322);

		immediateContext->RSSetState(NoCullRS);
		effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(textureBackground);
		technique->GetPassByIndex(0)->Apply(0, immediateContext);
		immediateContext->Draw(6, 2328);

		immediateContext->RSSetState(NoCullRS);
		effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(textureBackground);
		technique->GetPassByIndex(0)->Apply(0, immediateContext);
		immediateContext->Draw(6, 2334);
		//********************第二部分：绘制各个物体*************************************************


		swapChain->Present(0, 0);


	}
	return true;
}
//**************框架函数******************


//
// 回调函数
//
LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		::PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		if (wParam == VK_F1)  //重新开始游戏
		{
			InitCubesite();
			SetRandom();
		}
		if (wParam == VK_F2)  //播放背景音乐
			PlaySound(L"Techno_1.wav", NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
		if (wParam == VK_F3)   //停止播放背景音乐
			PlaySound(L"nosound.wav", NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
		if (wParam == VK_ESCAPE)
			::DestroyWindow(hwnd);
		if (wParam == VK_LEFT)
		{
			Lbtndown = true;
			flag1++;
			if (flag1 > 4)
			{
				flag1 = 1;
				angle = -2*XM_PI;
			}
		}
		if (wParam == VK_RIGHT)
		{
			Rbtndown = true;
			flag1--;
			if (flag1 < 1)
				flag1 = 4;
		}

		if (wParam == VK_UP)
			Ubtndown = true;
		if (wParam == VK_DOWN)
			Dbtndown = true;
		break;
	}
	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

//
// 主函数WinMain
//
int WINAPI WinMain(HINSTANCE hinstance,
	HINSTANCE prevInstance,
	PSTR cmdLine,
	int showCmd)
{

	//初始化
	//**注意**:最上面声明的IDirect3DDevice9指针，在这里作为参数传给InitD3D函数
	if (!d3d::InitD3D(hinstance,
		800,
		600,
		&renderTargetView,
		&immediateContext,
		&swapChain,
		&device,
		&depthStencilBuffer,
		&depthStencilView))// [out]The created device.
	{
		::MessageBox(0, L"InitD3D() - FAILED", 0, 0);
		return 0;
	}

	if (!Setup())
	{
		::MessageBox(0, L"Setup() - FAILED", 0, 0);
		return 0;
	}

	InitCubesite();
	SetRandom();

	d3d::EnterMsgLoop(Display);

	Cleanup();

	return 0;
}
