#include "ConcentrationGraphics.h"

void ConcentrationGraphics::Initialize(unsigned int screenWidth, unsigned int screenHeight, bool vsync, HWND hwnd, bool fullScreen)
{
    HRESULT result;

    // vsync(��������ȭ) ������ �����մϴ�.
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

    // ��� ��忡�� ȭ��ũ�⿡ �´� ��带 ã���ϴ�.
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

    // ���� �׷���ī���� �޸� �뷮(MB)�� ����
    videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

    // �׷���ī���� �̸��� char�� ���ڿ� �迭�� �ٲ� �� �����մϴ�.
    unsigned long long stringLength;
    int error = wcstombs_s(&stringLength, videoCardDescription, 128, adapterDesc.Description, 128);
    assert(error == 0);
    
    // output ����
    for (size_t i = 0; i < vOutputs.size(); i++)
    {
        vOutputs[i]->Release();
    }
    vOutputs.clear();

    // adapters ����
    for (size_t i = 0; i < vAdapters.size(); i++)
    {
        vAdapters[i]->Release();
    }
    vAdapters.clear();

    // factory ����
    pFactory->Release();
    pFactory = nullptr;


    // ���� ü�� description�� �ʱ�ȭ�մϴ�.
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

    // �ϳ��� ����۸��� ����ϵ��� �մϴ�.
    swapChainDesc.BufferCount = 1;

    // ������� �ʺ�� ���̸� �����մϴ�.
    swapChainDesc.BufferDesc.Width = screenWidth;
    swapChainDesc.BufferDesc.Height = screenHeight;

    // ����۷� �Ϲ����� 32bit�� �����̽��� �����մϴ�.
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    


    // ������� ���ΰ�ħ ������ �����մϴ�.
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

    // ������� �뵵�� �����մϴ�.
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

    // �������� �̷���� �������� �ڵ��� �����մϴ�.
    swapChainDesc.OutputWindow = hwnd;

    // ��Ƽ���ø��� ���ϴ�.
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;

    // ������ ��� �Ǵ� Ǯ��ũ�� ��带 �����մϴ�.
    if (fullScreen)
    {
        swapChainDesc.Windowed = false;
    }
    else
    {
        swapChainDesc.Windowed = true;
    }

    // ��ĵ������ ���İ� ��ĵ���̴��� �������� ��������(unspecified) �����մϴ�.
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    // ��µ� ������ ������� ������ �������� �մϴ�.
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    // �߰� �ɼ� �÷��׸� ������� �ʽ��ϴ�.
    swapChainDesc.Flags = 0;

    // ���� ������ DirectX 11�� �����մϴ�.
    D3D_FEATURE_LEVEL featureLevel;
    featureLevel = D3D_FEATURE_LEVEL_11_0;


    // ���� ü��, Direct3D ����̽�, Direct3D ����̽� ���ؽ�Ʈ�� �����մϴ�.
    result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1,
        D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &device, NULL, &deviceContext);
    assert(SUCCEEDED(result));


    // ������� �����͸� �����ɴϴ�.
    ID3D11Texture2D* backBufferPtr;
    result = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
    assert(SUCCEEDED(result));

    // ������� �����ͷ� ����Ÿ�� �並 �����մϴ�.
    result = device->CreateRenderTargetView(backBufferPtr, NULL, &renderTargetView);
    assert(SUCCEEDED(result));

    // ����� �����͸� ���̻� ������� �����Ƿ� �Ҵ� �����մϴ�.
    backBufferPtr->Release();
    backBufferPtr = nullptr;


    // ���� ������ description�� �ʱ�ȭ�մϴ�.
    D3D11_TEXTURE2D_DESC depthBufferDesc;
    ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

    // ���� ������ description�� �ۼ��մϴ�.
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

    // description�� ����Ͽ� ���� ������ �ؽ��ĸ� �����մϴ�.
    result = device->CreateTexture2D(&depthBufferDesc, NULL, &depthStencilBuffer);
    assert(SUCCEEDED(result));


    // ���ٽ� ������ description�� �ʱ�ȭ�մϴ�.
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
    ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

    // ���ٽ� ������ description�� �ۼ��մϴ�.
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


    // ���ٽ� ������ description�� �ʱ�ȭ�մϴ�.
    ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

    // ���ٽ� ������ description�� �ۼ��մϴ�.
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

    // ����-���ٽ� ���¸� �����մϴ�.
    result = device->CreateDepthStencilState(&depthStencilDesc, &depthStencilState);
    assert(SUCCEEDED(result));


    // ����-���ٽ� ���¸� �����մϴ�.
    deviceContext->OMSetDepthStencilState(depthStencilState, 1);


    // ����-���ٽ� ���� description�� �ʱ�ȭ�մϴ�.
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

    // ����-���ٽ� ���� description�� �ۼ��մϴ�.
    depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    // ����-���ٽ� �並 �����մϴ�.
    result = device->CreateDepthStencilView(depthStencilBuffer, &depthStencilViewDesc, &depthStencilView);
    assert(SUCCEEDED(result));


    // ����Ÿ�� ��� ����-���ٽ� ���۸� ���� ��� ���������ο� ���ε��մϴ�.
    deviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);


    // � ������ ��� �׸� ������ �����ϴ� ������ȭ�� description�� �ۼ��մϴ�.
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

    // �ۼ��� description���κ��� ������ȭ�� ���¸� �����մϴ�.
    result = device->CreateRasterizerState(&rasterDesc, &rasterState);
    assert(SUCCEEDED(result));

    // ������ȭ�� ���¸� �����մϴ�.
    deviceContext->RSSetState(rasterState);


    // �������� ���� ����Ʈ�� �����մϴ�.
    D3D11_VIEWPORT viewport;
    viewport.Width = (float)screenWidth;
    viewport.Height = (float)screenHeight;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;

    // ����Ʈ�� �����մϴ�.
    deviceContext->RSSetViewports(1, &viewport);

}

void ConcentrationGraphics::Finalize()
{
    // �����ϱ� ���� �̷��� ������ ���� �ٲ��� ������ ����ü���� �Ҵ� ������ �� ���ܰ� �߻��մϴ�.
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
