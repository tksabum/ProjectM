#include <windows.h>

#include "ConcentrationEngine.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	ConcentrationEngine::GetInstance().Initialize(hInstance, L"ProjectM", 960u, 540u);
	ConcentrationEngine::GetInstance().Run(nCmdShow);
	ConcentrationEngine::GetInstance().Finalize();

	return 0;
}