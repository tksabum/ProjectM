#pragma once

#include <Windows.h>
#include <utility>

class ConcentrationEngine
{
public:
	void Initialize(_In_ HINSTANCE hInstance, LPCWSTR gamename, unsigned int width, unsigned int height, bool screenresizeable = true, bool notitlebar = true, bool minimizable = true, bool maximizable = true);

	void Finalize();

	void Run(_In_ int nCmdShow);

private:
	void run();

private:
	HWND hWnd;


private:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	//singleton
public:
	static ConcentrationEngine& GetInstance();

private:
	ConcentrationEngine();
	~ConcentrationEngine();
	ConcentrationEngine(const ConcentrationEngine& ref) = delete;
	ConcentrationEngine& operator=(const ConcentrationEngine& ref) = delete;

};