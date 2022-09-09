#include <iostream>
#include <windows.h>
#include <thread>
#include <mutex> // std::mutex 
#include <ctime>
#include <future>
#include <chrono>

using namespace std;
using namespace std::chrono;

std::mutex mtx; // mutex для критической секции (в функции THfunction)
std::mutex interMtx; // mutex intersection
std::mutex interMtx2; // mutex intersection

LONG WINAPI WndProc(HWND, UINT, WPARAM, LPARAM); //предварительное объявление

int r = 17;// радиус мячика
int WINDOW_SIZES[2] = { 1000, 750 };
												 //общие для двух потоков переменные:
int X[4] = { WINDOW_SIZES[0] - r, 0,WINDOW_SIZES[0] - r,0 }; // координата  X
int Y[4] = { 0, 0,0,0 };
int speed[4] = { 4, -4,2,-2 };
bool changeColor = true;
int width;
int height;
int shifty = 170;
int shiftx = 250;
bool sverxy = false;
double  k = tan((rand() % 90 - 45) * 3.14 / 180);

bool doexal;

void godHelpMe() {
	while (true) {
			if (X[0] < 300 && X[0]>240) {
				Y[3] +=3;
			}
			if (X[0] > 700 && X[0] < 760)
				Y[3] -= 3;
	}
}
void yellowBall()
{
	bool locked = false;
	bool unlocked = false;
	int x = r;
	while (true) {
		x += speed[0];
		X[0] = x;
		if (X[0] < 300 && X[0]>240) {
			Y[0] += speed[0];
		}
		if (X[0] > 700 && X[0]<760)
			Y[0] -= speed[0];

		Sleep(20); 
		if (x >= WINDOW_SIZES[0] + r) {
			Y[0] = 0;
			x = r;
			locked = false;
			unlocked = false;
			speed[0] = rand()%20+10;
			continue;
		}
		if (!locked && x > 200) {
			interMtx.lock();
			locked = true;
			doexal = true;
		}
		if (!unlocked &&x >750) {
			interMtx.unlock();
			unlocked = true;
			doexal = false;
		}
	}
}

void blueBall() {
	bool resColor =1;
	bool locked = false;
	bool unlocked = false;
	int x = WINDOW_SIZES[0] - r;
	while (true) {
		x += speed[1];
		X[1] = x;
		Sleep(20);
		if (x < 0) {
			Y[1] = 0;
			x = WINDOW_SIZES[0] - r;
			locked = false;
			unlocked = false;
			speed[1] = rand() % 20 - 26;
			continue;
		}
		if (!locked &&x < 750 ) {
			interMtx.lock();
			locked = true;
		}
		if (!unlocked && x < 150) {
			interMtx.unlock();
			unlocked = true;
		}

	}
}
// Функция рисования мячика
void DrawBall(HDC hdc, RECT rectClient, int left, int top, int right, int bottom, HBRUSH hBrush)
{
	SelectObject(hdc, hBrush); //взяли кисть в руку
	Ellipse(hdc, left, top, right, bottom);  //рисуем круг
	DeleteObject(hBrush); // удаление желтой кисти
}
void drawRepair(HDC hdc, RECT rectClient, int left, int top, int right, int bottom) {
	HBRUSH hBrush;
	hBrush = CreateSolidBrush(RGB(128, 0, 0));
	SelectObject(hdc, hBrush); //взяли кисть в руку
	Rectangle(hdc, left, top, right, bottom);
	DeleteObject(hBrush); // удаление желтой кисти
}
void changeColorLight() {//левый низ правый верх
	while (true) {
		changeColor = !changeColor;
		Sleep(3000);
	}
}
void drawTrafficLight(HDC hdc, PAINTSTRUCT ps) {
	HBRUSH hBrush;
	int left = WINDOW_SIZES[0] / 2, top = 435, right = WINDOW_SIZES[0] / 2 + 20, bottom = 455;
	if (changeColor) {
		hBrush = CreateSolidBrush(RGB(255, 0, 0));
		DrawBall(hdc, ps.rcPaint, left - shiftx, top - shifty-40, right - shiftx, bottom - shifty-40, hBrush);
		hBrush = CreateSolidBrush(RGB(0, 255, 0));
		DrawBall(hdc, ps.rcPaint, left + shiftx, top , right + shiftx, bottom, hBrush);//левый верхний
	}
	else {
		hBrush = CreateSolidBrush(RGB(0, 255, 0));
		DrawBall(hdc, ps.rcPaint, left - shiftx, top - shifty-40, right - shiftx, bottom - shifty-40, hBrush);
		hBrush = CreateSolidBrush(RGB(255, 0, 0));
		DrawBall(hdc, ps.rcPaint, left + shiftx, top , right + shiftx, bottom , hBrush);//левый верхний
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

	WNDCLASS w; //структура данных для описания класса окон
				//регистрация класса окон для вывода результатов:
	memset(&w, 0, sizeof(WNDCLASS));
	w.style = CS_HREDRAW | CS_VREDRAW;
	w.lpfnWndProc = WndProc; // имя оконной функции
	w.hInstance = hInstance; // дескриптор (номер) процесса в Windows - главной функции
	w.hbrBackground = CreateSolidBrush(0x000FFFFFF); // цвет фона окна SpringGreen
	w.lpszClassName = L"My Class";
	RegisterClass(&w); // регистрация класса в системе
					   // создание потоков:
	HWND hwnd;
	hwnd = CreateWindow(L"My Class", L"PepeClown", //создание окна
		WS_OVERLAPPEDWINDOW, //тип окна - перекрывающее
		300, 50, WINDOW_SIZES[0], WINDOW_SIZES[1], NULL, NULL, //положение и размеры окна
		hInstance, NULL);  //ссылка на главный процесс
	int x = 0;
	ShowWindow(hwnd, nCmdShow); //вывод окна на экран
	InvalidateRect(hwnd, 0, 1); //подготовка к перерисовке
	UpdateWindow(hwnd);			//перерисовка содержания окна
								/*thread TH3(potok, hwnd);*/
	PAINTSTRUCT ps;
	HDC hdc;
	hdc = BeginPaint(hwnd, &ps);
	HBRUSH hBrush; // кисть для рисования
	std::thread TH0(yellowBall); //создать и запустить
	std::thread TH2(blueBall); //создать и запустить
	//std::thread TH1(THfunction, hInstance, w, 2, nCmdShow, hwnd); //создать и запустить
	//std::thread TH3(THfunction, hInstance, w, 3, nCmdShow, hwnd); //создать и запустить
	std::thread TL(changeColorLight);
	std::thread godestHelp(godHelpMe);
	TH0.detach();
	//TH1.detach();
	TH2.detach();
	godestHelp.detach();
	
	while (true) {
		InvalidateRect(hwnd, 0, 1); //подготовка к перерисовке
		UpdateWindow(hwnd);			//перерисовка содержания окна
		Sleep(40);
	}
//	TH3.join();
}
// Оконная функция
LONG WINAPI WndProc(HWND hwnd, UINT Message,
	WPARAM wparam, LPARAM lparam)
{

	HDC hdc;
	PAINTSTRUCT ps;
	HPEN hpen;
	switch (Message)
	{
	case WM_PAINT:
	{

		HDC hdc;
		hdc = BeginPaint(hwnd, &ps);
		HBRUSH hBrush;
		SetTextColor(hdc, 0x000000FF);
		int width = WINDOW_SIZES[0] = ps.rcPaint.right - ps.rcPaint.left;
		int height = WINDOW_SIZES[1] = ps.rcPaint.bottom - ps.rcPaint.top;
		hBrush = CreateSolidBrush(RGB(0, 0, 0));  
		DrawBall(hdc, ps.rcPaint, X[1], height / 1.85 + Y[1] - r, 2 * r + X[1], height / 1.85 + Y[1] + r, hBrush);
		hBrush = CreateSolidBrush(RGB(255, 0, 0));
		DrawBall(hdc, ps.rcPaint, X[1]+50, height / 1.85 + Y[1] - r, 2 * r + X[1]+ 50, height / 1.85 + Y[1] + r, hBrush);
	    hBrush = CreateSolidBrush(RGB(255, 255, 0)); 
		DrawBall(hdc, ps.rcPaint, X[1] + 100, height / 1.85 + Y[1] - r, 2 * r + X[1] + 100, height / 1.85 + Y[1] + r, hBrush); 
		hBrush = CreateSolidBrush(RGB(235, 235, 235));  
		DrawBall(hdc, ps.rcPaint, X[0], height / 2.30 - r+ Y[0], 2 * r + X[0], height / 2.30+ Y[0] + r, hBrush); //yellow
		hBrush = CreateSolidBrush(RGB(0, 0, 255));
	
		DrawBall(hdc, ps.rcPaint, X[0]-50, height / 2.30 - r + Y[0], 2 * r + X[0]-50, height / 2.30 + Y[0] + r, hBrush); //yellow
		hBrush = CreateSolidBrush(RGB(255,0, 0));
		DrawBall(hdc, ps.rcPaint, X[0] - 100, height / 2.30 - r + Y[0], 2 * r + X[0]- 100, height / 2.30 + Y[0] + r, hBrush); //yellow

		drawTrafficLight(hdc, ps);
		hpen = CreatePen(PS_SOLID, 5, 0); 
		SelectObject(hdc, hpen);
		int offset = 30 * 3 / 1.5;
		MoveToEx(hdc, 0, height / 2.25 - offset, 0);
		LineTo(hdc, width, height / 2.25- offset);

		HPEN hpen1;
		hpen1 = CreatePen(PS_DASH, 0 ,4); 
		SelectObject(hdc, hpen1);
		MoveToEx(hdc, 0, height / 2.08 , 0);
		LineTo(hdc, width, height / 2.08);

		hpen = CreatePen(PS_SOLID, 5, 0); 
		SelectObject(hdc, hpen);
		MoveToEx(hdc, 0, height / 1.95 + offset, 0);
		LineTo(hdc, width, height / 1.95 + offset);
		drawRepair(hdc, ps.rcPaint, width / 2.25 - 100, height / 2.75, width / 1.75 + 100, height / 2.10);
		EndPaint(hwnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, Message, wparam, lparam);
	}

	return 0;
}