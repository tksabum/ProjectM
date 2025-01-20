#include "ConcentrationGraphics.h"

void ConcentrationGraphics::Initialize(unsigned int screenWidth, unsigned int screenHeight, bool vsync, HWND hwnd, bool fullScreen)
{
    HRESULT result;

    // vsync(수직동기화) 설정을 저장합니다.
    vsyncEnabled = vsync;

    // Create IDXGIFactory
    IDXGIFactory* pFactory;
    result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&pFactory));
    assert(SUCCEEDED(result));

    // Create IDXGIAdapter
    IDXGIAdapter* pAdapter;
    std::vector <IDXGIAdapter*> vAdapters;
    for (UINT i = 0; pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; i++)
    {
        vAdapters.push_back(pAdapter);
    }
    assert(0 < vAdapters.size());

    pAdapter = vAdapters[0];

    // Outputs
    IDXGIOutput* pOutput;
    std::vector<IDXGIOutput*> vOutputs;
    for (UINT i = 0; pAdapter->EnumOutputs(i, &pOutput) != DXGI_ERROR_NOT_FOUND; i++)
    {
        vOutputs.push_back(pOutput);
    }
    assert(0 < vOutputs.size());
    pOutput = vOutputs[0];

    // Get Display Mode
    UINT num = 0;
    DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
    UINT flags = DXGI_ENUM_MODES_INTERLACED;

    result = pOutput->GetDisplayModeList(format, flags, &num, nullptr);
    assert(SUCCEEDED(result));

    std::vector<DXGI_MODE_DESC> modeDesc(num);
    result = pOutput->GetDisplayModeList(format, flags, &num, modeDesc.data());
    assert(SUCCEEDED(result));

    unsigned int numerator = 0u;
    unsigned int denominator = 0u;

    // 모든 모드에서 화면크기에 맞는 모드를 찾습니다.
    for (size_t i = 0; i < modeDesc.size(); i++)
    {
        if (modeDesc[i].Width == screenWidth)
        {
            if (modeDesc[i].Height == screenHeight)
            {
                numerator = modeDesc[i].RefreshRate.Numerator;
                denominator = modeDesc[i].RefreshRate.Denominator;
            }
        }
    }


    // Get adapter description
    DXGI_ADAPTER_DESC adapterDesc;
    result = pAdapter->GetDesc(&adapterDesc);
    assert(SUCCEEDED(result));

    // 현재 그래픽카드의 메모리 용량(MB)을 저장
    videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

    // 그래픽카드의 이름을 char형 문자열 배열로 바꾼 뒤 저장합니다.
    unsigned long long stringLength;
    int error = wcstombs_s(&stringLength, videoCardDescription, 128, adapterDesc.Description, 128);
    assert(error == 0);
    
    // output 해제
    for (size_t i = 0; i < vOutputs.size(); i++)
    {
        vOutputs[i]->Release();
    }
    vOutputs.clear();

    // adapters 해제
    for (size_t i = 0; i < vAdapters.size(); i++)
    {
        vAdapters[i]->Release();
    }
    vAdapters.clear();

    // factory 해제
    pFactory->Release();
    pFactory = nullptr;


    // 스왑 체인 description을 초기화합니다.
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

    // 하나의 백버퍼만을 사용하도록 합니다.
    swapChainDesc.BufferCount = 1;

    // 백버퍼의 너비와 높이를 설정합니다.
    swapChainDesc.BufferDesc.Width = screenWidth;
    swapChainDesc.BufferDesc.Height = screenHeight;

    // 백버퍼로 일반적인 32bit의 서페이스를 지정합니다.
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    


    // 백버퍼의 새로고침 비율을 설정합니다.
    if (vsyncEnabled)
    {
        swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
    }
    else
    {
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    }

    // 백버퍼의 용도를 설정합니다.
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

    // 렌더링이 이루어질 윈도우의 핸들을 설정합니다.
    swapChainDesc.OutputWindow = hwnd;

    // 멀티샘플링을 끕니다.
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;

    // 윈도우 모드 또는 풀스크린 모드를 설정합니다.
    if (fullScreen)
    {
        swapChainDesc.Windowed = false;
    }
    else
    {
        swapChainDesc.Windowed = true;
    }

    // 스캔라인의 정렬과 스캔라이닝을 지정되지 않음으로(unspecified) 설정합니다.
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    // 출력된 이후의 백버퍼의 내용을 버리도록 합니다.
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    // 추가 옵션 플래그를 사용하지 않습니다.
    swapChainDesc.Flags = 0;

    // 피쳐 레벨을 DirectX 11로 설정합니다.
    D3D_FEATURE_LEVEL featureLevel;
    featureLevel = D3D_FEATURE_LEVEL_11_0;


    // 스왑 체인, Direct3D 디바이스, Direct3D 디바이스 컨텍스트를 생성합니다.
    result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1,
        D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &device, NULL, &deviceContext);
    assert(SUCCEEDED(result));


    // 백버퍼의 포인터를 가져옵니다.
    ID3D11Texture2D* backBufferPtr;
    result = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
    assert(SUCCEEDED(result));

    // 백버퍼의 포인터로 렌더타겟 뷰를 생성합니다.
    result = device->CreateRenderTargetView(backBufferPtr, NULL, &renderTargetView);
    assert(SUCCEEDED(result));

    // 백버퍼 포인터를 더이상 사용하지 않으므로 할당 해제합니다.
    backBufferPtr->Release();
    backBufferPtr = nullptr;


    // 깊이 버퍼의 description을 초기화합니다.
    D3D11_TEXTURE2D_DESC depthBufferDesc;
    ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

    // 깊이 버퍼의 description을 작성합니다.
    depthBufferDesc.Width = screenWidth;
    depthBufferDesc.Height = screenHeight;
    depthBufferDesc.MipLevels = 1;
    depthBufferDesc.ArraySize = 1;
    depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthBufferDesc.SampleDesc.Count = 1;
    depthBufferDesc.SampleDesc.Quality = 0;
    depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthBufferDesc.CPUAccessFlags = 0;
    depthBufferDesc.MiscFlags = 0;

    // description을 사용하여 깊이 버퍼의 텍스쳐를 생성합니다.
    result = device->CreateTexture2D(&depthBufferDesc, NULL, &depthStencilBuffer);
    assert(SUCCEEDED(result));


    // 스텐실 상태의 description을 초기화합니다.
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
    ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

    // 스텐실 상태의 description을 작성합니다.
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

    depthStencilDesc.StencilEnable = true;
    depthStencilDesc.StencilReadMask = 0xFF;
    depthStencilDesc.StencilWriteMask = 0xFF;

    // Stencil operations if pixel is front-facing.
    depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Stencil operations if pixel is back-facing.
    depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;


    // 스텐실 상태의 description을 초기화합니다.
    ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

    // 스텐실 상태의 description을 작성합니다.
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

    depthStencilDesc.StencilEnable = true;
    depthStencilDesc.StencilReadMask = 0xFF;
    depthStencilDesc.StencilWriteMask = 0xFF;

    // Stencil operations if pixel is front-facing.
    depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Stencil operations if pixel is back-facing.
    depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // 깊이-스텐실 상태를 생성합니다.
    result = device->CreateDepthStencilState(&depthStencilDesc, &depthStencilState);
    assert(SUCCEEDED(result));


    // 깊이-스텐실 상태를 설정합니다.
    deviceContext->OMSetDepthStencilState(depthStencilState, 1);


    // 깊이-스텐실 뷰의 description을 초기화합니다.
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

    // 깊이-스텐실 뷰의 description을 작성합니다.
    depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    // 깊이-스텐실 뷰를 생성합니다.
    result = device->CreateDepthStencilView(depthStencilBuffer, &depthStencilViewDesc, &depthStencilView);
    assert(SUCCEEDED(result));


    // 렌더타겟 뷰와 깊이-스텐실 버퍼를 각각 출력 파이프라인에 바인딩합니다.
    deviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);


    // 어떤 도형을 어떻게 그릴 것인지 결정하는 래스터화기 description을 작성합니다.
    D3D11_RASTERIZER_DESC rasterDesc;
    rasterDesc.AntialiasedLineEnable = false;
    rasterDesc.CullMode = D3D11_CULL_BACK;
    rasterDesc.DepthBias = 0;
    rasterDesc.DepthBiasClamp = 0.0f;
    rasterDesc.DepthClipEnable = true;
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.FrontCounterClockwise = false;
    rasterDesc.MultisampleEnable = false;
    rasterDesc.ScissorEnable = false;
    rasterDesc.SlopeScaledDepthBias = 0.0f;

    // 작성한 description으로부터 래스터화기 상태를 생성합니다.
    result = device->CreateRasterizerState(&rasterDesc, &rasterState);
    assert(SUCCEEDED(result));

    // 래스터화기 상태를 설정합니다.
    deviceContext->RSSetState(rasterState);


    // 렌더링을 위한 뷰포트를 설정합니다.
    D3D11_VIEWPORT viewport;
    viewport.Width = (float)screenWidth;
    viewport.Height = (float)screenHeight;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;

    // 뷰포트를 생성합니다.
    deviceContext->RSSetViewports(1, &viewport);

}

void ConcentrationGraphics::Finalize()
{
    // 종료하기 전에 이렇게 윈도우 모드로 바꾸지 않으면 스왑체인을 할당 해제할 때 예외가 발생합니다.
    if (swapChain)
    {
        swapChain->SetFullscreenState(false, NULL);
    }

    if (rasterState)
    {
        rasterState->Release();
        rasterState = 0;
    }

    if (depthStencilView)
    {
        depthStencilView->Release();
        depthStencilView = 0;
    }

    if (depthStencilState)
    {
        depthStencilState->Release();
        depthStencilState = 0;
    }

    if (depthStencilBuffer)
    {
        depthStencilBuffer->Release();
        depthStencilBuffer = 0;
    }

    if (renderTargetView)
    {
        renderTargetView->Release();
        renderTargetView = 0;
    }

    if (deviceContext)
    {
        deviceContext->Release();
        deviceContext = 0;
    }

    if (device)
    {
        device->Release();
        device = 0;
    }

    if (swapChain)
    {
        swapChain->Release();
        swapChain = 0;
    }
}

void ConcentrationGraphics::BeginDraw()
{
}

void ConcentrationGraphics::EndDraw()
{
    float lightGray[4] = { 0.9f, 0.9f, 0.9f, 1.0f };
    float gray[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
    float black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

    deviceContext->ClearRenderTargetView(renderTargetView, lightGray);

    

    // Present the back buffer to the screen since rendering is complete.
    if (vsyncEnabled)
    {
        // Lock to screen refresh rate.
        swapChain->Present(1, 0);
    }
    else
    {
        // Present as fast as possible.
        swapChain->Present(0, 0);
    }
}

ConcentrationGraphics& ConcentrationGraphics::GetInstance()
{
    static ConcentrationGraphics instance;
    return instance;
}

ConcentrationGraphics::ConcentrationGraphics() :
    vsyncEnabled(true),
    videoCardMemory(),
    videoCardDescription(),
    swapChain(nullptr),
    device(nullptr),
    deviceContext(nullptr),
    renderTargetView(nullptr),
    depthStencilBuffer(nullptr),
    depthStencilState(nullptr),
    depthStencilView(nullptr),
    rasterState(nullptr)
{
}

ConcentrationGraphics::~ConcentrationGraphics()
{
}
