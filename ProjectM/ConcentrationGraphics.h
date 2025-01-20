#pragma once

#include <Windows.h>

#include <dxgi.h>
#include <d3d11.h>
#include <DirectXMath.h>

#include <vector>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

class ConcentrationGraphics
{
public:
	void Initialize(unsigned int screenWidth, unsigned int screenHeight, bool vsync, HWND hwnd, bool fullScreen);
    void Finalize();

    void BeginDraw();
    void EndDraw();

private:


private:
    bool vsyncEnabled;
    int videoCardMemory;
    char videoCardDescription[128];
    IDXGISwapChain* swapChain;
    ID3D11Device* device;
    ID3D11DeviceContext* deviceContext;
    ID3D11RenderTargetView* renderTargetView;
    ID3D11Texture2D* depthStencilBuffer;
    ID3D11DepthStencilState* depthStencilState;
    ID3D11DepthStencilView* depthStencilView;
    ID3D11RasterizerState* rasterState;
    //DirectX::XMMATRIX m_projectionMatrix;
    //DirectX::XMMATRIX m_worldMatrix;
    //DirectX::XMMATRIX m_orthoMatrix;

    //singleton
public:
    static ConcentrationGraphics& GetInstance();

private:
    ConcentrationGraphics();
    ~ConcentrationGraphics();
    ConcentrationGraphics(const ConcentrationGraphics& ref) = delete;
    ConcentrationGraphics& operator=(const ConcentrationGraphics& ref) = delete;

};

