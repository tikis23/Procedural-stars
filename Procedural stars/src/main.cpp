#include "Application.h"
#include <Windows.h>

int start() {
	Application app;
	if (!app.Init())
		return 1;
	return app.Start();
}

#if _DEBUG
int main() {
	return start();
}
#else
int main() {
	return start();
}
//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
//	return start();
//}
#endif