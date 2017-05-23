#include "d3dUtility.h"
#include <stdlib.h>
#include <time.h>
//����ȫ�ֵ�ָ��
ID3D11Device* device = NULL;//D3D11�豸�ӿ�
IDXGISwapChain* swapChain = NULL;//�������ӿ�
ID3D11DeviceContext* immediateContext = NULL;
ID3D11RenderTargetView* renderTargetView = NULL;//��ȾĿ����ͼ  

//Effect���ȫ��ָ��
ID3D11InputLayout* vertexLayout;
ID3DX11Effect* effect;
ID3DX11EffectTechnique* technique;

//������������ϵ����
XMMATRIX world;         //��������任�ľ���
XMMATRIX view;          //���ڹ۲�任�ľ���
XMMATRIX projection;    //����ͶӰ�任�ľ���

ID3D11DepthStencilView* depthStencilView;  //���ģ����ͼ
ID3D11Texture2D* depthStencilBuffer;       //��Ȼ���


ID3D11ShaderResourceView* texture[13];   //4096
ID3D11ShaderResourceView* textureMiss;   //ʧ��
ID3D11ShaderResourceView* textureGoal;   //�ɹ��ﵽ4096
ID3D11ShaderResourceView* textureBackground;   //����ͼƬ

static float angle = -XM_PI / 2;   //����һ����̬�������ڼ�¼�Ƕ�
static float dangle = 0.0f;
static float height = 0.0f;        //����һ����̬�������ڼ�¼�߶�
static float dheight = 0.0f;
//
ID3D11RasterizerState* NoCullRS;             //��������״̬



//�жϰ����Ƿ���
bool Lbtndown = false;
bool Rbtndown = false;
bool Ubtndown = false;
bool Dbtndown = false;

//�ж��Ƿ�ﵽ4096�����Ǵﵽ������������
bool goSucceed = false;
bool goDie = false;

//����һ������
int cubesite[4][4][4][2];

//��¼����������ת����
int flag1 = 1;

//����һ������ṹ����������������ͷ���������������
struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT2 Tex;
};

//��ʼ��64�������壬������Ϊ0
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

//ÿ�ƶ�һ�Σ��������������������
void SetRandom()
{
	int nothingNum = 0;

	//�ж��Ƿ�ﵽ��4096�����Լ�δ�������������
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

	//����δ������������Ŀ��ȷ�������ƶ���Ҫ��������������Ŀ
	int newNum = 3;
	switch (nothingNum)
	{
	case 0:
	{
			  //��64�������嶼���������ж��Ƿ��пɺϲ���������
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
			  //�����û�пɺϲ��������壬�򱾴���Ϸ����
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

	//���������
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

			for (int k = 3; k >-1; k--)//���ݷ���������һ�����������α���
			{
				if (cubesite[k][j][i][1] == 0)//����ǰλ�õ���ͼΪ0ʱ����ʾΪ�գ�͸����Ū������������������ͼ���ۼƿյĸ���move
				{
					move++;
				}
				if (cubesite[k][j][i][1] > 0)//����ǰλ�õ���ͼ��Ϊ0ʱ����ǰ��move��λ�õķ��齻����ͼ
				{
					swap = cubesite[k][j][i][1];
					cubesite[k][j][i][1] = cubesite[k + move][j][i][1];
					cubesite[k + move][j][i][1] = swap;
					if (move != 0)
					{
						flag++;
					}
					if (k<3 && (k + move)<3)//��������жϺ��������ͼƬ��ͼ�಻��ͬ����ͬ�����λ�÷�����ͼ���ֻ�λԭ���������Լ�����
					{
						if (cubesite[k + move][j][i][1] == cubesite[k + move + 1][j][i][1])
						{
							cubesite[k + move + 1][j][i][1]++;//��ͼ����Ϊ�˺�ͼƬ�ļ�·����Ӧ����2�Ĵη������
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
	Sleep(500);  //��ֹ�˰��������Ӧ
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

			for (int k = 0; k < 4; k++)//���ݷ���������һ�����������α���
			{
				if (cubesite[k][j][i][1] == 0)//����ǰλ�õ���ͼΪ0ʱ����ʾΪ�գ�͸����Ū������������������ͼ���ۼƿյĸ���move
				{
					move++;
				}
				if (cubesite[k][j][i][1] > 0)//����ǰλ�õ���ͼ��Ϊ0ʱ����ǰ��move��λ�õķ��齻����ͼ
				{
					swap = cubesite[k][j][i][1];
					cubesite[k][j][i][1] = cubesite[k - move][j][i][1];
					cubesite[k - move][j][i][1] = swap;
					if (move != 0)
					{
						flag++;
					}
					if (k>0 && (k - move)>0)//��������жϺ��������ͼƬ��ͼ�಻��ͬ����ͬ�����λ�÷�����ͼ���ֻ�λԭ���������Լ�����
					{
						if (cubesite[k - move][j][i][1] == cubesite[k - move - 1][j][i][1])
						{
							cubesite[k - move - 1][j][i][1]++;//��ͼ����Ϊ�˺�ͼƬ�ļ�·����Ӧ����2�Ĵη������
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
	Sleep(500);  //��ֹ�˰��������Ӧ
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

			for (int k = 3; k >-1; k--)//���ݷ���������һ�����������α���
			{
				if (cubesite[i][j][k][1] == 0)// ����ǰλ�õ���ͼΪ0ʱ����ʾΪ�գ�͸����Ū������������������ͼ���ۼƿյĸ���move
				{
					move++;
				}
				if (cubesite[i][j][k][1] > 0)//����ǰλ�õ���ͼ��Ϊ0ʱ����ǰ��move��λ�õķ��齻����ͼ
				{
					swap = cubesite[i][j][k][1];
					cubesite[i][j][k][1] = cubesite[i][j][k + move][1];
					cubesite[i][j][k + move][1] = swap;
					if (move != 0)
					{
						flag++;
					}
					if (k<3 && (k + move)<3)//��������жϺ��������ͼƬ��ͼ�಻��ͬ����ͬ�����λ�÷�����ͼ���ֻ�λԭ���������Լ�����
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
	Sleep(500);  //��ֹ�˰��������Ӧ

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

			for (int k = 0; k < 4; k++)//���ݷ���������һ�����������α���
			{
				if (cubesite[i][j][k][1] == 0)//����ǰλ�õ���ͼΪ0ʱ����ʾΪ�գ�͸����Ū������������������ͼ���ۼƿյĸ���move
				{
					move++;
				}
				if (cubesite[i][j][k][1] > 0)//����ǰλ�õ���ͼ��Ϊ0ʱ����ǰ��move��λ�õķ��齻����ͼ
				{
					swap = cubesite[i][j][k][1];
					cubesite[i][j][k][1] = cubesite[i][j][k - move][1];
					cubesite[i][j][k - move][1] = swap;
					if (move != 0)
					{
						flag++;
					}
					if (k>0 && (k - move)>0)//��������жϺ��������ͼƬ��ͼ�಻��ͬ����ͬ�����λ�÷�����ͼ���ֻ�λԭ���������Լ�����
					{
						if (cubesite[i][j][k - move][1] == cubesite[i][j][k - move - 1][1])
						{
							cubesite[i][j][k - move - 1][1]++;//��ͼ����Ϊ�˺�ͼƬ�ļ�·����Ӧ����2�Ĵη������
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
	Sleep(500);  //��ֹ�˰��������Ӧ
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

			for (int k = 0; k < 4; k++)//���ݷ���������һ�����������α���
			{
				if (cubesite[i][k][j][1] == 0)//����ǰλ�õ���ͼΪ0ʱ����ʾΪ�գ�͸����Ū������������������ͼ���ۼƿյĸ���move
				{
					move++;
				}
				if (cubesite[i][k][j][1] > 0)//����ǰλ�õ���ͼ��Ϊ0ʱ����ǰ��move��λ�õķ��齻����ͼ
				{
					swap = cubesite[i][k][j][1];
					cubesite[i][k][j][1] = cubesite[i][k - move][j][1];
					cubesite[i][k - move][j][1] = swap;
					if (move != 0)
					{
						flag++;
					}
					if (k>0 && (k - move)>0)//��������жϺ��������ͼƬ��ͼ�಻��ͬ����ͬ�����λ�÷�����ͼ���ֻ�λԭ���������Լ�����
					{
						if (cubesite[i][k - move][j][1] == cubesite[i][k - move - 1][j][1])
						{
							cubesite[i][k - move - 1][j][1]++;//��ͼ����Ϊ�˺�ͼƬ�ļ�·����Ӧ����2�Ĵη������
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
	Sleep(500);  //��ֹ�˰��������Ӧ
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

			for (int k = 3; k >-1; k--)//���ݷ���������һ�����������α���
			{
				if (cubesite[i][k][j][1] == 0)//����ǰλ�õ���ͼΪ0ʱ����ʾΪ�գ�͸����Ū������������������ͼ���ۼƿյĸ���move
				{
					move++;
				}
				if (cubesite[i][k][j][1] > 0)//����ǰλ�õ���ͼ��Ϊ0ʱ����ǰ��move��λ�õķ��齻����ͼ
				{
					swap = cubesite[i][k][j][1];
					cubesite[i][k][j][1] = cubesite[i][k + move][j][1];
					cubesite[i][k + move][j][1] = swap;
					if (move != 0)
					{
						flag++;
					}
					if (k<3 && (k + move)<3)//��������жϺ��������ͼƬ��ͼ�಻��ͬ����ͬ�����λ�÷�����ͼ���ֻ�λԭ���������Լ�����
					{
						if (cubesite[i][k + move][j][1] == cubesite[i][k + move + 1][j][1])
						{
							cubesite[i][k + move + 1][j][1]++;//��ͼ����Ϊ�˺�ͼƬ�ļ�·����Ӧ����2�Ĵη������
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
	Sleep(500);  //��ֹ�˰��������Ӧ
}
//**************����Ϊ��ܺ���******************
bool Setup()
{
	//������Ҫ����5����Ҫ����
	//��һ�������ⲿ�ļ�������fx�ļ���ͼ���ļ���
	//�ڶ�������������Ⱦ״̬
	//�������������벼��
	//���Ĳ��������㻺��
	//*************��һ�������ⲿ�ļ�������fx�ļ���ͼ���ļ���****************************
	HRESULT hr = S_OK;              //����HRESULT�Ķ������ڼ�¼���������Ƿ�ɹ�
	ID3DBlob* pTechBlob = NULL;     //����ID3DBlob�Ķ������ڴ�Ŵ��ļ���ȡ����Ϣ
	//������֮ǰ������.fx�ļ���ȡ��ɫ�������Ϣ
	hr = D3DX11CompileFromFile(L"Shader.fx", NULL, NULL, NULL, "fx_5_0",
		D3DCOMPILE_ENABLE_STRICTNESS, 0, NULL, &pTechBlob, NULL, NULL);
	if (FAILED(hr))
	{
		::MessageBox(NULL, L"fx�ļ�����ʧ��", L"Error", MB_OK); //�����ȡʧ�ܣ�����������Ϣ
		return hr;
	}
	//����D3DX11CreateEffectFromMemory����ID3DEffect����
	hr = D3DX11CreateEffectFromMemory(pTechBlob->GetBufferPointer(),
		pTechBlob->GetBufferSize(), 0, device, &effect);

	if (FAILED(hr))
	{
		::MessageBox(NULL, L"����Effectʧ��", L"Error", MB_OK);  //����ʧ�ܣ�����������Ϣ
		return hr;
	}
	//���ⲿͼ���ļ���������
	//����
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
	//*************��һ�������ⲿ�ļ�������fx�ļ���ͼ���ļ���****************************

	////*************�ڶ�������������Ⱦ״̬************************************************

	//�رձ�������
	D3D11_RASTERIZER_DESC ncDesc;        //��դ������
	ZeroMemory(&ncDesc, sizeof(ncDesc));  //�������
	ncDesc.CullMode = D3D11_CULL_NONE;   //�޳��ض�����������Σ����ﲻ�޳�����ȫ������
	ncDesc.FillMode = D3D11_FILL_SOLID;  //���ģʽ������Ϊ�������������
	ncDesc.FrontCounterClockwise = false;//�Ƿ�������ʱ��������������Ϊ����
	ncDesc.DepthClipEnable = true;       //������Ȳü�
	//����һ���رձ���������״̬������Ҫ�õ�ʱ������ø��豸������
	if (FAILED(device->CreateRasterizerState(&ncDesc, &NoCullRS)))
	{
		MessageBox(NULL, L"Create 'NoCull' rasterizer state failed!", L"Error", MB_OK);
		return false;
	}


	////*************�ڶ�������������Ⱦ״̬************************************************

	//*************�������������벼��****************************************************
	//��GetTechniqueByName��ȡID3DX11EffectTechnique�Ķ���
	//������Ĭ�ϵ�technique��Effect
	technique = effect->GetTechniqueByName("TexTech");                //Ĭ��Technique

	//D3DX11_PASS_DESC�ṹ��������һ��Effect Pass
	D3DX11_PASS_DESC PassDesc;
	//����GetPassByIndex��ȡEffect Pass
	//������GetDesc��ȡEffect Pass��������������PassDesc������
	technique->GetPassByIndex(0)->GetDesc(&PassDesc);

	//�������������벼��
	//�������Ƕ���һ��D3D11_INPUT_ELEMENT_DESC���飬
	//�������Ƕ���Ķ���ṹ����λ������ͷ��������������������������Ԫ��
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	//layoutԪ�ظ���
	UINT numElements = ARRAYSIZE(layout);
	//����CreateInputLayout�������벼��
	hr = device->CreateInputLayout(layout, numElements, PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize, &vertexLayout);
	//�������ɵ����벼�ֵ��豸��������
	immediateContext->IASetInputLayout(vertexLayout);
	if (FAILED(hr))
	{
		::MessageBox(NULL, L"����Input Layoutʧ��", L"Error", MB_OK);
		return hr;
	}
	//*************�������������벼��****************************************************

	//*************���Ĳ��������㻺��****************************************************
	//������Ҫ�������ӣ����ӣ��Լ�ˮ��Ķ���
	Vertex vertices[] =
	{
		//1-1-1
		//����
		{ XMFLOAT3(-2.75f, 2.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.75f, 2.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, 1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(-1.75f, 2.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, 2.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, 2.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, 1.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//1-1-2
		//����
		{ XMFLOAT3(-1.25f, 2.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-0.25f, 2.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, 1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(-0.25f, 2.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, 2.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, 2.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, 1.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//1-1-3
		//����
		{ XMFLOAT3(0.25f, 2.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.25f, 2.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, 1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(1.25f, 2.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, 2.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, 2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, 2.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 2.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, 1.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//1-1-4
		//����
		{ XMFLOAT3(1.75f, 2.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(2.75f, 2.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, 1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(2.75f, 2.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, 2.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, 2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, 2.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 2.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 2.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, 1.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 1.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//1-2-1
		//����
		{ XMFLOAT3(-2.75f, 1.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.75f, 1.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, 0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(-1.75f, 1.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, 1.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, 1.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, 0.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//1-2-2
		//����
		{ XMFLOAT3(-1.25f, 1.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-0.25f, 1.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, 0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(-0.25f, 1.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, 1.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, 1.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, 0.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },

		//1-2-3
		//����
		{ XMFLOAT3(0.25f, 1.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.25f, 1.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, 0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(1.25f, 1.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, 1.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, 1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, 1.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, 0.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//1-2-4
		//����
		{ XMFLOAT3(1.75f, 1.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(2.75f, 1.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, 0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(2.75f, 1.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, 1.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, 1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, 1.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 1.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, 0.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 0.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//1-3-1
		//����
		{ XMFLOAT3(-2.75f, -0.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.75f, -0.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, -1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(-1.75f, -0.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, -0.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, -0.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, -1.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//1-3-2
		//����
		{ XMFLOAT3(-1.25f, -0.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-0.25f, -0.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, -1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(-0.25f, -0.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, -0.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, -0.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, -1.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//1-3-3
		//����
		{ XMFLOAT3(0.25f, -0.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.25f, -0.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, -1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(1.25f, -0.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, -0.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, -0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, -0.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -0.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, -1.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//1-3-4
		//����
		{ XMFLOAT3(1.75f, -0.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(2.75f, -0.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, -1.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(2.75f, -0.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, -0.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, -0.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, -0.25f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -0.25f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -0.25f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -0.25f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, -1.25f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -1.25f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -1.25f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//1-4-1
		//����
		{ XMFLOAT3(-2.75f, -1.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.75f, -1.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, -2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(-1.75f, -1.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, -1.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, -1.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, -2.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//1-4-2
		//����
		{ XMFLOAT3(-1.25f, -1.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-0.25f, -1.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, -2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(-0.25f, -1.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, -1.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, -1.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, -2.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//1-4-3
		//����
		{ XMFLOAT3(0.25f, -1.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.25f, -1.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, -2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(1.25f, -1.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, -1.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, -1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, -1.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, -2.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//1-4-4
		//����
		{ XMFLOAT3(1.75f, -1.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(2.75f, -1.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, -2.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(2.75f, -1.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, -1.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, -1.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, -1.75f, -1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -1.75f, -2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -1.75f, -1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.75f, -2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, -2.75f, -2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -2.75f, -1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -2.75f, -2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, -1.75f), XMFLOAT2(1.0f, 1.0f) },
		//2-1-1
		//����
		{ XMFLOAT3(-2.75f, 2.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.75f, 2.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, 1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(-1.75f, 2.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, 2.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, 2.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, 1.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//2-1-2
		//����
		{ XMFLOAT3(-1.25f, 2.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-0.25f, 2.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, 1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(-0.25f, 2.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, 2.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, 2.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, 1.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//2-1-3
		//����
		{ XMFLOAT3(0.25f, 2.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.25f, 2.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, 1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(1.25f, 2.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, 2.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, 2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, 2.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 2.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, 1.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//2-1-4
		//����
		{ XMFLOAT3(1.75f, 2.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(2.75f, 2.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, 1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(2.75f, 2.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, 2.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, 2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, 2.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 2.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 2.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, 1.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 1.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//2-2-1
		//����
		{ XMFLOAT3(-2.75f, 1.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.75f, 1.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, 0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(-1.75f, 1.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, 1.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, 1.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, 0.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//2-2-2
		//����
		{ XMFLOAT3(-1.25f, 1.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-0.25f, 1.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, 0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(-0.25f, 1.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, 1.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, 1.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, 0.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//2-2-3
		//����
		{ XMFLOAT3(0.25f, 1.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.25f, 1.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, 0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(1.25f, 1.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, 1.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, 1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, 1.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, 0.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//2-2-4
		//����
		{ XMFLOAT3(1.75f, 1.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(2.75f, 1.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, 0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(2.75f, 1.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, 1.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, 1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, 1.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 1.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, 0.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 0.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//2-3-1
		//����
		{ XMFLOAT3(-2.75f, -0.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.75f, -0.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, -1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(-1.75f, -0.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, -0.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, -0.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, -1.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//2-3-2
		//����
		{ XMFLOAT3(-1.25f, -0.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-0.25f, -0.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, -1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(-0.25f, -0.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, -0.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, -0.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, -1.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//2-3-3
		//����
		{ XMFLOAT3(0.25f, -0.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.25f, -0.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, -1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(1.25f, -0.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, -0.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, -0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, -0.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -0.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, -1.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//2-3-4
		//����
		{ XMFLOAT3(1.75f, -0.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(2.75f, -0.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, -1.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(2.75f, -0.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, -0.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, -0.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, -0.25f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -0.25f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -0.25f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -0.25f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, -1.25f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -1.25f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -1.25f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//2-4-1
		//����
		{ XMFLOAT3(-2.75f, -1.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.75f, -1.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, -2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(-1.75f, -1.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, -1.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, -1.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, -2.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//2-4-2
		//����
		{ XMFLOAT3(-1.25f, -1.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-0.25f, -1.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, -2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(-0.25f, -1.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, -1.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, -1.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, -2.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//2-4-3
		//����
		{ XMFLOAT3(0.25f, -1.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.25f, -1.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, -2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(1.25f, -1.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, -1.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, -1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, -1.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, -2.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//2-4-4
		//����
		{ XMFLOAT3(1.75f, -1.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(2.75f, -1.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, -2.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(2.75f, -1.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, -1.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, -1.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, -1.75f, -0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -1.75f, -1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -1.75f, -0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.75f, -1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, -2.75f, -1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -2.75f, -0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -2.75f, -1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, -0.25f), XMFLOAT2(1.0f, 1.0f) },
		//*******(3,1,1)********
		//����
		{ XMFLOAT3(-2.75f, 2.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.75f, 2.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, 1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(-1.75f, 2.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, 2.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, 2.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, 1.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//*******(3,1,2)********
		//����
		{ XMFLOAT3(-1.25f, 2.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-0.25f, 2.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, 1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(-0.25f, 2.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, 2.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, 2.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, 1.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//*******(3,1,3)********
		//����
		{ XMFLOAT3(0.25f, 2.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.25f, 2.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, 1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(1.25f, 2.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, 2.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, 2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, 2.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 2.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, 1.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//*******(3,1,4)********
		//����
		{ XMFLOAT3(1.75f, 2.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(2.75f, 2.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, 1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(2.75f, 2.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, 2.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, 2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, 2.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 2.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 2.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, 1.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 1.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//*******(3,2,1)********
		//����
		{ XMFLOAT3(-2.75f, 1.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.75f, 1.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, 0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(-1.75f, 1.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, 1.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, 1.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, 0.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//*******(3,2,2)********
		//����
		{ XMFLOAT3(-1.25f, 1.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-0.25f, 1.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, 0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(-0.25f, 1.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, 1.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, 1.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, 0.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//*******(3,2,3)********
		//����
		{ XMFLOAT3(0.25f, 1.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.25f, 1.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, 0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(1.25f, 1.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, 1.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, 1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, 1.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, 0.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//*******(3,2,4)********
		//����
		{ XMFLOAT3(1.75f, 1.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(2.75f, 1.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, 0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(2.75f, 1.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, 1.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, 1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, 1.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 1.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, 0.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 0.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//*******(3,3,1)********
		//����
		{ XMFLOAT3(-2.75f, -0.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.75f, -0.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, -1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(-1.75f, -0.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, -0.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, -0.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, -1.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//*******(3,3,2)********
		//����
		{ XMFLOAT3(-1.25f, -0.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-0.25f, -0.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, -1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(-0.25f, -0.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, -0.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, -0.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, -1.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },

		//*******(3,3,3)********
		//����
		{ XMFLOAT3(0.25f, -0.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.25f, -0.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, -1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(1.25f, -0.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, -0.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, -0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, -0.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -0.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, -1.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//*******(3,3,4)********
		//����
		{ XMFLOAT3(1.75f, -0.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(2.75f, -0.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, -1.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(2.75f, -0.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, -0.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, -0.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, -0.25f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -0.25f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -0.25f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -0.25f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, -1.25f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -1.25f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -1.25f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//*******(3,4,1)********
		//����
		{ XMFLOAT3(-2.75f, -1.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.75f, -1.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, -2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(-1.75f, -1.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, -1.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, -1.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, -2.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//*******(3,4,2)********
		//����
		{ XMFLOAT3(-1.25f, -1.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-0.25f, -1.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, -2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(-0.25f, -1.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, -1.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, -1.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, -2.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },

		//*******(3,4,3)********
		//����
		{ XMFLOAT3(0.25f, -1.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.25f, -1.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, -2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(1.25f, -1.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, -1.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, -1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, -1.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, -2.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },

		//*******(3,4,4)********
		//����
		{ XMFLOAT3(1.75f, -1.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(2.75f, -1.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, -2.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(2.75f, -1.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, -1.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, -1.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, -1.75f, 1.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -1.75f, 0.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -1.75f, 1.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.75f, 0.25f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, -2.75f, 0.25f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -2.75f, 1.25f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -2.75f, 0.25f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, 1.25f), XMFLOAT2(1.0f, 1.0f) },

		//*******(4,1,1)********
		//����
		{ XMFLOAT3(-2.75f, 2.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.75f, 2.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, 1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(-1.75f, 2.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, 2.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, 2.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 2.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, 1.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//*******(4,1,2)********
		//����
		{ XMFLOAT3(-1.25f, 2.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-0.25f, 2.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, 1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(-0.25f, 2.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, 2.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, 2.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 2.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, 1.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//*******(4,1,3)********
		//����
		{ XMFLOAT3(0.25f, 2.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.25f, 2.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, 1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(1.25f, 2.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, 2.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, 2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, 2.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 2.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, 1.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//*******(4,1,4)********
		//����
		{ XMFLOAT3(1.75f, 2.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(2.75f, 2.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, 1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(2.75f, 2.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, 2.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, 2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, 2.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 2.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 2.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, 1.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 1.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//*******(4,2,1)********
		//����
		{ XMFLOAT3(-2.75f, 1.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.75f, 1.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, 0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(-1.75f, 1.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, 1.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, 1.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 1.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, 0.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, 0.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//*******(4,2,2)********
		//����
		{ XMFLOAT3(-1.25f, 1.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-0.25f, 1.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, 0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(-0.25f, 1.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, 1.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, 1.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 1.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, 0.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, 0.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//*******(4,2,3)********
		//����
		{ XMFLOAT3(0.25f, 1.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.25f, 1.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, 0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(1.25f, 1.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, 1.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, 1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, 1.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 1.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, 0.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, 0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, 0.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//*******(4,2,4)********
		//����
		{ XMFLOAT3(1.75f, 1.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(2.75f, 1.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, 0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(2.75f, 1.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, 1.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, 1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, 1.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 1.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 1.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, 0.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, 0.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, 0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, 0.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//*******(4,3,1)********
		//����
		{ XMFLOAT3(-2.75f, -0.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.75f, -0.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, -1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(-1.75f, -0.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, -0.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, -0.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -0.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, -1.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//*******(4,3,2)********
		//����
		{ XMFLOAT3(-1.25f, -0.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-0.25f, -0.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, -1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(-0.25f, -0.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, -0.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, -0.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -0.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, -1.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },

		//*******(4,3,3)********
		//����
		{ XMFLOAT3(0.25f, -0.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.25f, -0.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, -1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(1.25f, -0.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, -0.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, -0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, -0.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -0.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, -1.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//*******(4,3,4)********
		//����
		{ XMFLOAT3(1.75f, -0.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(2.75f, -0.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, -1.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(2.75f, -0.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, -0.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, -0.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, -0.25f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -0.25f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -0.25f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -0.25f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, -1.25f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -1.25f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -1.25f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.25f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//*******(4,4,1)********
		//����
		{ XMFLOAT3(-2.75f, -1.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.75f, -1.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, -2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(-1.75f, -1.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.75f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, -1.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, -1.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -1.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-2.75f, -2.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-2.75f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-2.75f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.75f, -2.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//*******(4,4,2)********
		//����
		{ XMFLOAT3(-1.25f, -1.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-0.25f, -1.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, -2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(-0.25f, -1.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, -1.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, -1.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -1.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(-1.25f, -2.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.25f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.25f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.25f, -2.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//*******(4,4,3)********
		//����
		{ XMFLOAT3(0.25f, -1.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.25f, -1.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, -2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(1.25f, -1.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.25f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, -1.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, -1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, -1.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -1.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(0.25f, -2.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.25f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(0.25f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.25f, -2.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.25f, -2.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//*******(4,4,4)********
		//����
		{ XMFLOAT3(1.75f, -1.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(2.75f, -1.75f, 1.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, -2.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//ǰ��
		{ XMFLOAT3(2.75f, -1.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(2.75f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, 2.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, -1.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -2.75f, 2.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.75f, -1.75f, 1.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -2.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
		{ XMFLOAT3(1.75f, -1.75f, 2.75f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.75f, -1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.75f, -1.75f, 1.75f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(2.75f, -1.75f, 2.75f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(2.75f, -1.75f, 1.75f), XMFLOAT2(1.0f, 1.0f) },
		//����
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
		//ǰ��
		{ XMFLOAT3(-8.0f, 8.0f, 8.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(8.0f, 8.0f, 8.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-8.0f, -8.0f, 8.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(8.0f, 8.0f, 8.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(8.0f, -8.0f, 8.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-8.0f, -8.0f, 8.0f), XMFLOAT2(0.0f, 1.0f) },

		//Goal  or  Miss
		//����
		{ XMFLOAT3(-8.0f, 8.0f, -8.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(8.0f, 8.0f, -8.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-8.0f, -8.0f, -8.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(8.0f, 8.0f, -8.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(8.0f, -8.0f, -8.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-8.0f, -8.0f, -8.0f), XMFLOAT2(0.0f, 1.0f) },

		//Goal  or  Miss
		//����
		{ XMFLOAT3(-8.0f, 8.0f, 8.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-8.0f, 8.0f, -8.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-8.0f, -8.0f, 8.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-8.0f, 8.0f, -8.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-8.0f, -8.0f, -8.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-8.0f, -8.0f, 8.0f), XMFLOAT2(0.0f, 1.0f) },

		//Goal  or  Miss
		//����
		{ XMFLOAT3(8.0f, 8.0f, 8.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(8.0f, 8.0f, -8.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(8.0f, -8.0f, 8.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(8.0f, 8.0f, -8.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(8.0f, -8.0f, -8.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(8.0f, -8.0f, 8.0f), XMFLOAT2(0.0f, 1.0f) },

		//Goal  or  Miss
		//����
		{ XMFLOAT3(-8.0f, 8.0f, 8.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(8.0f, 8.0f, 8.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-8.0f, 8.0f, -8.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(8.0f, 8.0f, 8.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(8.0f, 8.0f, -8.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-8.0f, 8.0f, -8.0f), XMFLOAT2(0.0f, 1.0f) },

		//Goal  or  Miss
		//����
		{ XMFLOAT3(-8.0f, -8.0f, 8.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(8.0f, -8.0f, 8.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-8.0f, -8.0f, -8.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(8.0f, -8.0f, 8.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(8.0f, -8.0f, -8.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-8.0f, -8.0f, -8.0f), XMFLOAT2(0.0f, 1.0f) },




	};
	UINT vertexCount = ARRAYSIZE(vertices);
	//�������㻺�棬����ͬʵ��4һ��
	//��������һ��D3D11_BUFFER_DESC�Ķ���bd
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex)* vertexCount;      //ע�⣺�������ﶨ����24����������Ҫ����24
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;  //ע�⣺�����ʾ�������Ƕ��㻺��
	bd.CPUAccessFlags = 0;

	//����һ��D3D11_SUBRESOURCE_DATA�������ڳ�ʼ������Դ
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;         //������Ҫ��ʼ�������ݣ���������ݾ��Ƕ�������
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	//����һ��ID3D11Buffer������Ϊ���㻺��
	ID3D11Buffer* vertexBuffer;
	//����CreateBuffer�������㻺��
	hr = device->CreateBuffer(&bd, &InitData, &vertexBuffer);
	if (FAILED(hr))
	{
		::MessageBox(NULL, L"����VertexBufferʧ��", L"Error", MB_OK);
		return hr;
	}

	UINT stride = sizeof(Vertex);                 //��ȡVertex�Ĵ�С��Ϊ���
	UINT offset = 0;                              //����ƫ����Ϊ0
	//���ö��㻺�棬�����Ľ��ͼ�ʵ��4
	immediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	//ָ��ͼԪ���ͣ�D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST��ʾͼԪΪ������
	immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//*************���Ĳ��������㻺��****************************************************
	return true;
}

void Cleanup()
{
	//�ͷ�ȫ��ָ��
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
		//����һ����������ɫ��Ϣ��4��Ԫ�طֱ��ʾ�죬�̣����Լ�alpha
		float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
		immediateContext->ClearRenderTargetView(renderTargetView, ClearColor);
		//�����ǰ�󶨵����ģ����ͼ
		immediateContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
		//��ָ��������ӣ�һ�㲻����������������������ָ��Ϊʹ��blend factor
		float BlendFactor[] = { 0, 0, 0, 0 };

		//********************��һ���֣�����3������ϵ�����յ��ⲿ����****************************
		//ͨ�����̵��������Ҽ����ı���������ͷ����.



		if (Lbtndown) //��Ӧ���������
		{
			angle -= 3.0f * XM_PI / 2000;
			dangle += 3.0f * XM_PI / 2000;

		}
		if (Rbtndown) //��Ӧ�����ҷ����
		{
			angle += 3.0f * XM_PI / 2000;
			dangle += 3.0f * XM_PI / 2000;

		}
		if (Ubtndown)    //��Ӧ�����Ϸ����
		{
			height += 15.0f * timeDelta;
			dheight += 15.0f * timeDelta;
		}
		if (Dbtndown)  //��Ӧ�����·����
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

		//��ʼ���������
		world = XMMatrixIdentity();
		XMVECTOR Eye = XMVectorSet(cosf(angle) * 6.0f, height, sinf(angle) * 6.0f, 0.0f);//���λ��
		XMVECTOR At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);  //Ŀ��λ��
		XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);  //up
		view = XMMatrixLookAtLH(Eye, At, Up);   //���ù۲�����ϵ
		//����ͶӰ����
		projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, 800.0f / 600.0f, 0.01f, 100.0f);
		//������任����ĳ��������еľ�����������õ�Effect�����---------------------
		effect->GetVariableByName("World")->AsMatrix()->SetMatrix((float*)&world);  //������������ϵ
		effect->GetVariableByName("View")->AsMatrix()->SetMatrix((float*)&view);    //���ù۲�����ϵ
		effect->GetVariableByName("Projection")->AsMatrix()->SetMatrix((float*)&projection); //����ͶӰ����ϵ
		effect->GetVariableByName("EyePosition")->AsMatrix()->SetMatrix((float*)&Eye); //�����ӵ�
		//********************��һ���֣�����3������ϵ�����յ��ⲿ����****************************

		//����һ��D3DX11_TECHNIQUE_DESC����������technique
		D3DX11_TECHNIQUE_DESC techDesc;
		technique->GetDesc(&techDesc);    //��ȡtechnique������
		//��ȡͨ����PASS���������õ��豸�������С�
		//��������ֻ��һ��ͨ������������Ϊ0
		technique->GetPassByIndex(0)->Apply(0, immediateContext);


		if (::GetAsyncKeyState('A') & 0x8000f) //��Ӧ���������
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

		if (::GetAsyncKeyState('D') & 0x8000f) //��Ӧ�����ҷ����
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
		if (::GetAsyncKeyState('S') & 0x8000f) //��Ӧ���������
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
		if (::GetAsyncKeyState('W') & 0x8000f) //��Ӧ�����ҷ����
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
						effect->GetVariableByName("World")->AsMatrix()->SetMatrix((float*)&world);  //������������ϵ
						effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(texture[cubesite[i][j][k][1]]);
						technique->GetPassByIndex(0)->Apply(0, immediateContext);
						immediateContext->Draw(36, cubesite[i][j][k][0]);   //����������



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

		//���ɱ���

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
		//********************�ڶ����֣����Ƹ�������*************************************************


		swapChain->Present(0, 0);


	}
	return true;
}
//**************��ܺ���******************


//
// �ص�����
//
LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		::PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		if (wParam == VK_F1)  //���¿�ʼ��Ϸ
		{
			InitCubesite();
			SetRandom();
		}
		if (wParam == VK_F2)  //���ű�������
			PlaySound(L"Techno_1.wav", NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
		if (wParam == VK_F3)   //ֹͣ���ű�������
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
// ������WinMain
//
int WINAPI WinMain(HINSTANCE hinstance,
	HINSTANCE prevInstance,
	PSTR cmdLine,
	int showCmd)
{

	//��ʼ��
	//**ע��**:������������IDirect3DDevice9ָ�룬��������Ϊ��������InitD3D����
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
