#include <iostream>
#include <windows.h>
#include <thread>
#include <mutex> // std::mutex 
#include <future>
#include <chrono>
#include <Windows.h>
#include <mmsystem.h>

using namespace std;
using namespace std::chrono;
#pragma warning(disable : 4996) .
#pragma comment(lib, "Winmm.lib")
std::mutex mtx; // mutex для критической секции (в функции THfunction)
std::mutex interMtx; // mutex intersection

LONG WINAPI WndProc(HWND, UINT, WPARAM, LPARAM); //предварительное объявление
												 //общие для двух потоков переменные:
int WINDOW_SIZES[2] = { 1300, 750 };
int XbeginFooter = 10,
YbeginFooter = WINDOW_SIZES[1] / 4.81 - 200,
XendFooter = WINDOW_SIZES[0] * 2 / 11 - 40,
YendFooter = WINDOW_SIZES[1] / 4.01 - 200,

wallXB = WINDOW_SIZES[0] / 2.36 - 300,
wallYB = WINDOW_SIZES[1] / 7.5 - 300,
wallXE = WINDOW_SIZES[0] / 1.85 - 300,
wallYE = WINDOW_SIZES[1] / 3 - 300,
verticalX = 700, verticalBY = -130, verticalEY = -10,
horizontalXB = 750, horizontalXE = 700, horizontalY = -60;

int triangle[6] = { 535,WINDOW_SIZES[1] / 5 - 300, 435,WINDOW_SIZES[1] / 3 - 300,635,WINDOW_SIZES[1] / 3 - 300 };
int footerConstructionSleeve[8] = { 100,-10,100,YbeginFooter - 5,XbeginFooter,YbeginFooter - 5,XendFooter,YbeginFooter - 5 };
int wallsConstructionSleeve[8] = { 320,-10,320,wallYB - 5,wallXB ,wallYB - 5,wallXE ,wallYB - 5 };
int roofConstructionSleeve[7] = { 535,-10,535,triangle[1] - 5,635,435,triangle[1] + 95 };
int furerConstructionSleeve[4] = { 700,-10,700,verticalBY };

int detals[4] = { 6,10,3,2 };
bool canBuildOrNot[4] = { 0,0,0,0 };
bool readyToGo[4] = { 0,0,0,0 };
int speed = 6;

void DrawBall(HDC hdc, RECT rectClient, int left, int top, int right, int bottom, HBRUSH hBrush)
{
	SelectObject(hdc, hBrush); //взяли кисть в руку
	Ellipse(hdc, left, top, right, bottom);  //рисуем круг
	DeleteObject(hBrush); // удаление желтой кисти
}

void DrawRoof(HDC hdc) {
	HBRUSH hBrush;
	hBrush = CreateSolidBrush(RGB(0, 124, 174));
	SelectObject(hdc, hBrush);
	POINT vertices[] = { {triangle[0], triangle[1]} ,{triangle[2], triangle[3]} ,{triangle[4], triangle[5]} };
	Polygon(hdc, vertices, sizeof(vertices) / sizeof(vertices[0]));
	DeleteObject(hBrush); // удаление желтой кисти
}
void DrawF(HDC hdc, int l1, int l2, int l3, int l4, int l5, int l6) {

	HBRUSH hBrush;
	hBrush = CreateSolidBrush(RGB(255, 15, 192));
	SelectObject(hdc, hBrush);
	POINT vertices[] = { l1,l2,l3,l4,l5,l6 };
	Polygon(hdc, vertices, sizeof(vertices) / sizeof(vertices[0]));

	DeleteObject(hBrush);
}
void drawFooter(HDC hdc, int left, int top, int right, int bottom) {
	HBRUSH hBrush;
	hBrush = CreateSolidBrush(RGB(255, 255, 0));
	SelectObject(hdc, hBrush);
	Rectangle(hdc, left, top, right, bottom);
	DeleteObject(hBrush);
}
void drawWalls(HDC hdc, int left, int top, int right, int bot) {
	HBRUSH hBrush;
	hBrush = CreateSolidBrush(RGB(255, 10, 10));
	SelectObject(hdc, hBrush);
	Rectangle(hdc, left, top, right, bot);
	DeleteObject(hBrush);
}
void drawFurer(HDC hdc) {
	MoveToEx(hdc, verticalX, verticalBY, 0);
	LineTo(hdc, verticalX, verticalEY);
	MoveToEx(hdc, verticalX, verticalBY, 0);
	LineTo(hdc, horizontalXB, horizontalY);
	MoveToEx(hdc, horizontalXB, horizontalY, 0);
	LineTo(hdc, horizontalXE, horizontalY);
}
void buildGermany(HDC hdc) {
	do {
		Sleep(40);
	} while (canBuildOrNot[2] == false);
	while (true) {
		if (verticalX >= WINDOW_SIZES[0] + 100) {
			canBuildOrNot[3] = 0;
			Sleep(8000);
			detals[3]--;
			verticalX = 700, verticalBY = -130, verticalEY = -10,
				horizontalXB = 750, horizontalXE = 700, horizontalY = -60;
			furerConstructionSleeve[0] = 700;
			furerConstructionSleeve[1] = -10;
			furerConstructionSleeve[2] = 700;
			furerConstructionSleeve[3] = verticalBY;
		}
		else {
			if (verticalEY > triangle[1]) {
				readyToGo[3] = true;
				verticalX += speed;
				horizontalXB += speed;
				horizontalXE += speed;
				canBuildOrNot[3] = 1;
			}
			else {
				readyToGo[3] = false;
				canBuildOrNot[3] = 0;
				verticalBY += 4;
				verticalEY += 4;
				horizontalY += 4;
			}
		}
		Sleep(60);
	}
}
void buildRoof(HDC hdc) {
	do {
		Sleep(40);
	} while (canBuildOrNot[1] == false);
	while (true) {
		if (triangle[2] >= WINDOW_SIZES[0]) {
			canBuildOrNot[2] = 0;
			Sleep(4000);
			detals[2]--;
			triangle[0] = 535;
			triangle[1] = WINDOW_SIZES[1] / 5 - 300;
			triangle[2] = 435;
			triangle[3] = WINDOW_SIZES[1] / 3 - 300;
			triangle[4] = 635;
			triangle[5] = WINDOW_SIZES[1] / 3 - 300;
			roofConstructionSleeve[0] = 535;
			roofConstructionSleeve[1] = -10;
			roofConstructionSleeve[2] = 535;
			roofConstructionSleeve[3] = triangle[1] - 5;
			roofConstructionSleeve[4] = 635;
			roofConstructionSleeve[5] = 435;
			roofConstructionSleeve[6] = triangle[1] + 95;
		}
		else {
			if (triangle[3] > wallYB) {
				readyToGo[2] = true;
				triangle[0] += speed;
				triangle[2] += speed;
				triangle[4] += speed;
				canBuildOrNot[2] = 1;
				if (triangle[0] > verticalX) {
					while (!readyToGo[3])
						Sleep(1);
				}
			}
			else {
				readyToGo[2] = false;
				canBuildOrNot[2] = 0;
				triangle[1] += 4;
				triangle[3] += 4;
				triangle[5] += 4;
			}
		}
		Sleep(60);
	}
}
void buildWalls(HDC hdc) {
	do {
		Sleep(40);
	} while (canBuildOrNot[0] == false);
	while (true) {
		if (wallXB >= WINDOW_SIZES[0]) {
			canBuildOrNot[1] = 0;
			Sleep(2000);
			detals[1]--;
			wallXB = WINDOW_SIZES[0] / 2.36 - 300,
				wallYB = WINDOW_SIZES[1] / 7.5 - 300,
				wallXE = WINDOW_SIZES[0] / 1.85 - 300,
				wallYE = WINDOW_SIZES[1] / 3 - 300,
				wallsConstructionSleeve[0] = 320;
			wallsConstructionSleeve[1] = -10;
			wallsConstructionSleeve[2] = 320;
			wallsConstructionSleeve[3] = wallYB - 10;
			wallsConstructionSleeve[4] = wallXB;
			wallsConstructionSleeve[5] = wallYB - 10;
			wallsConstructionSleeve[6] = wallXE;
			wallsConstructionSleeve[7] = wallYB - 10;
		}
		else {
			if (wallYE > WINDOW_SIZES[1] * 5 / 6 - (YendFooter - YbeginFooter)) {
				wallXB += speed;
				wallXE += speed;
				readyToGo[1] = true;
				canBuildOrNot[1] = 1;
				if (wallXE > triangle[4] - 20) {
					while (!readyToGo[2])
						Sleep(1);
				}
				if (triangle[0] > verticalX) {
					while (!readyToGo[3])
						Sleep(1);
				}
			}
			else {
				readyToGo[1] = false;
				canBuildOrNot[1] = 0;
				wallYB += speed;
				wallYE += speed;
			}
		}
		Sleep(60);
	}
}
void  buildFooter(HDC hdc, HWND hwnd) {
	while (true) {
		if (XbeginFooter >= WINDOW_SIZES[0] - 30) {
			canBuildOrNot[0] = 0;
			detals[0]--;
			XbeginFooter = 10,
				YbeginFooter = WINDOW_SIZES[1] / 4.81 - 200,
				XendFooter = WINDOW_SIZES[0] * 2 / 11 - 40,
				YendFooter = WINDOW_SIZES[1] / 4.01 - 200;
			footerConstructionSleeve[0] = 100;
			footerConstructionSleeve[1] = -10;
			footerConstructionSleeve[2] = 100;
			footerConstructionSleeve[3] = YbeginFooter - 7;
			footerConstructionSleeve[4] = XbeginFooter;
			footerConstructionSleeve[5] = YbeginFooter - 7;
			footerConstructionSleeve[6] = XendFooter;
			footerConstructionSleeve[7] = YbeginFooter - 7;
		}
		else {
			if (YendFooter > WINDOW_SIZES[1] * 5 / 6) {
				XbeginFooter += speed;
				XendFooter += speed;
				canBuildOrNot[0] = 1;
				if (XendFooter > WINDOW_SIZES[0] / 2.36 - 125) {
					while (!readyToGo[1])
						Sleep(20);
				}
				if (wallXE > triangle[4] - 20) {
					while (!readyToGo[2])
						Sleep(20);
				}
				if (triangle[0] > verticalX) {
					while (!readyToGo[3])
						Sleep(20);
				}
			}
			else {
				canBuildOrNot[0] = 0;
				YbeginFooter += speed;
				YendFooter += speed;
			}
		}
		Sleep(60);
	}
}
void zvyk() {
	while (true) {
		PlaySound(L"C:/Users/Пользователь/Downloads/kekw.wav", NULL, SND_ASYNC);
		Sleep(6500);
	}
}
void upAndDownConstrructionFooter() {
	while (true) {
		if (YendFooter > WINDOW_SIZES[1] * 5 / 6) {
			if (footerConstructionSleeve[3] < -12 || footerConstructionSleeve[7] < -12 || footerConstructionSleeve[5] < -12)
				continue;
			footerConstructionSleeve[3] -= speed;
			footerConstructionSleeve[5] -= speed;
			footerConstructionSleeve[7] -= speed;
		}
		else {
			footerConstructionSleeve[3] += speed;
			footerConstructionSleeve[5] += speed;
			footerConstructionSleeve[7] += speed;
		}

		Sleep(60);

	}
}
void upAndDownConstrructionWalls() {
	do {
		Sleep(40);
	} while (canBuildOrNot[0] == false);
	while (true) {
		if (wallYE > WINDOW_SIZES[1] * 5 / 6 - (YendFooter - YbeginFooter)) {
			if (wallsConstructionSleeve[3] < -12 || wallsConstructionSleeve[7] < -12 || wallsConstructionSleeve[5] < -12)
				continue;
			wallsConstructionSleeve[3] -= speed;
			wallsConstructionSleeve[5] -= speed;
			wallsConstructionSleeve[7] -= speed;
		}
		else {
			wallsConstructionSleeve[3] += speed;
			wallsConstructionSleeve[5] += speed;
			wallsConstructionSleeve[7] += speed;
		}
		Sleep(60);
	}
}
void upAndDownConstrructionRoof() {
	do {
		Sleep(40);
	} while (canBuildOrNot[1] == false);
	while (true) {
		if (triangle[3] > wallYB) {
			roofConstructionSleeve[3] -= 4;
			roofConstructionSleeve[6] -= 4;
		}
		else {
			roofConstructionSleeve[3] += 4;
			roofConstructionSleeve[6] += 4;
		}
		Sleep(60);
	}
}
void upAndDownConstrructionFurer() {
	do {
		Sleep(40);
	} while (canBuildOrNot[2] == false);
	while (true) {
		if (verticalEY > triangle[1]) {
			furerConstructionSleeve[3] -= speed;
		}
		else {
			furerConstructionSleeve[3] += 4;
		}
		Sleep(60);
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
	w.hbrBackground = CreateSolidBrush(RGB(128, 128, 128)); // цвет фона окна SpringGreen
	w.lpszClassName = L"My Class";
	RegisterClass(&w); // регистрация класса в системе
					   // создание потоков:
	HWND hwnd;
	hwnd = CreateWindow(L"My Class", L"PepeClown", //создание окна
		WS_OVERLAPPEDWINDOW, //тип окна - перекрывающее
		50, 50, WINDOW_SIZES[0], WINDOW_SIZES[1], NULL, NULL, //положение и размеры окна
		hInstance, NULL);  //ссылка на главный процесс
	int x = 0;
	ShowWindow(hwnd, nCmdShow); //вывод окна на экран
	InvalidateRect(hwnd, 0, 1); //подготовка к перерисовке
	UpdateWindow(hwnd);			//перерисовка содержания окна
	PAINTSTRUCT ps;
	HDC hdc;
	hdc = BeginPaint(hwnd, &ps);
	HBRUSH hBrush;
	std::thread TH1(buildFooter, hdc, hwnd);
	std::thread TH2(buildWalls, hdc);
	std::thread TH3(buildRoof, hdc);
	std::thread TH4(buildGermany, hdc);
	std::thread construction1(upAndDownConstrructionFooter);
	std::thread construction2(upAndDownConstrructionWalls);
	std::thread construction3(upAndDownConstrructionRoof);
	std::thread construction4(upAndDownConstrructionFurer);
	//std::thread Sound(zvyk);
	//Sound.detach();
	TH1.detach();
	TH2.detach();
	TH3.detach();
	while (true) {
		InvalidateRect(hwnd, 0, 1);
		UpdateWindow(hwnd);
		Sleep(40);
	}
	TH1.detach();
	//	THH.detach();
	TH4.detach();
	construction1.detach();
	construction2.detach();
	construction3.detach();
	construction4.detach();
}
// Оконная функция
LONG WINAPI WndProc(HWND hwnd, UINT Message,
	WPARAM wparam, LPARAM lparam)
{

	HDC hdc;
	PAINTSTRUCT ps;
	HPEN hpen;
	BITMAP bm;
	switch (Message)
	{
	case WM_PAINT:
	{
		HDC hdc;
		HBRUSH hBrush;
		hdc = BeginPaint(hwnd, &ps);
		int width = WINDOW_SIZES[0] = ps.rcPaint.right - ps.rcPaint.left;
		int height = WINDOW_SIZES[1] = ps.rcPaint.bottom - ps.rcPaint.top;
		hpen = CreatePen(PS_SOLID, 5, 0);
		SelectObject(hdc, hpen);
		DrawF(hdc, verticalX, verticalBY, horizontalXB, horizontalY, horizontalXE, horizontalY);
		hpen = CreatePen(PS_SOLID, 10, 0);
		SelectObject(hdc, hpen);

		MoveToEx(hdc, footerConstructionSleeve[0], footerConstructionSleeve[1], 0);
		LineTo(hdc, footerConstructionSleeve[2], footerConstructionSleeve[3]);
		MoveToEx(hdc, footerConstructionSleeve[4], footerConstructionSleeve[5], 0);
		LineTo(hdc, footerConstructionSleeve[6], footerConstructionSleeve[7]);

		MoveToEx(hdc, wallsConstructionSleeve[0], wallsConstructionSleeve[1], 0);
		LineTo(hdc, wallsConstructionSleeve[2], wallsConstructionSleeve[3]);
		MoveToEx(hdc, wallsConstructionSleeve[4], wallsConstructionSleeve[5], 0);
		LineTo(hdc, wallsConstructionSleeve[6], wallsConstructionSleeve[7]);

		MoveToEx(hdc, roofConstructionSleeve[0], roofConstructionSleeve[1], 0);
		LineTo(hdc, roofConstructionSleeve[2], roofConstructionSleeve[3]);
		MoveToEx(hdc, roofConstructionSleeve[2], roofConstructionSleeve[3], 0);
		LineTo(hdc, roofConstructionSleeve[4], roofConstructionSleeve[6]);
		MoveToEx(hdc, roofConstructionSleeve[2], roofConstructionSleeve[3], 0);
		LineTo(hdc, roofConstructionSleeve[5], roofConstructionSleeve[6]);

		DeleteObject(hpen);

		MoveToEx(hdc, furerConstructionSleeve[0], furerConstructionSleeve[1], 0);
		LineTo(hdc, furerConstructionSleeve[2], furerConstructionSleeve[3]);
		hpen = CreatePen(PS_SOLID, 20, 0); // черное перо 5px
		SelectObject(hdc, hpen);
		MoveToEx(hdc, width / 6, 0, 0);
		LineTo(hdc, width / 6, height * 4.5 / 6 - 40);
		MoveToEx(hdc, width * 2 / 6 - 5, 0, 0);
		LineTo(hdc, width * 2 / 6 - 5, height * 3.5 / 6 - 40);
		MoveToEx(hdc, width * 3 / 6 + 5, 0, 0);
		LineTo(hdc, width * 3 / 6 + 5, height * 2.6 / 6 - 40);
		MoveToEx(hdc, width * 4 / 6, 0, 0);
		LineTo(hdc, width * 4 / 6, height * 1.9 / 6 - 40);
		MoveToEx(hdc, width * 2, height * 5 / 6, 0);
		LineTo(hdc, width * 2, height * 5 / 6);
		MoveToEx(hdc, 0, height * 5 / 6, 0);
		LineTo(hdc, width, height * 5 / 6);
		DeleteObject(hpen);

		hpen = CreatePen(PS_SOLID, 3, 0); // черное перо 5px
		SelectObject(hdc, hpen);
		if (!canBuildOrNot[0] == 0)
			hBrush = CreateSolidBrush(RGB(0, 255, 0));
		else
			hBrush = CreateSolidBrush(RGB(255, 0, 0));
		DrawBall(hdc, ps.rcPaint, width / 6 - 10, height * 4.5 / 6 - 80, width / 6 + 10, height * 4.5 / 6 - 60, hBrush);

		if (!canBuildOrNot[1] == 0)
			hBrush = CreateSolidBrush(RGB(0, 255, 0));
		else
			hBrush = CreateSolidBrush(RGB(255, 0, 0));
		DrawBall(hdc, ps.rcPaint, width * 2 / 6 - 15, height * 3.5 / 6 - 80, width * 2 / 6 + 5, height * 3.5 / 6 - 60, hBrush);

		if (!canBuildOrNot[2] == 0)
			hBrush = CreateSolidBrush(RGB(0, 255, 0));
		else
			hBrush = CreateSolidBrush(RGB(255, 0, 0));
		DrawBall(hdc, ps.rcPaint, width * 3 / 6 - 5, height * 2.6 / 6 - 80, width * 3 / 6 + 15, height * 2.6 / 6 - 60, hBrush);

		if (!canBuildOrNot[3] == 0)
			hBrush = CreateSolidBrush(RGB(0, 255, 0));
		else
			hBrush = CreateSolidBrush(RGB(255, 0, 0));
		DrawBall(hdc, ps.rcPaint, width * 4 / 6 - 10, height * 1.9 / 6 - 80, width * 4 / 6 + 10, height * 1.9 / 6 - 60, hBrush);

		if (detals[0] == 0) {
			SetTextColor(hdc, RGB(255, 0, 0));
			hBrush = CreateSolidBrush(RGB(0, 0, 0));
			SelectObject(hdc, hBrush);
			Rectangle(hdc, 0, height * 5 / 6, width, height);
			Rectangle(hdc, 0, 0, width, 20);
			TextOut(hdc, 320, height / 2 + 250, L"Невозможно построить деталь: фундамент.Строительство окончено", 62);
			EndPaint(hwnd, &ps);
			Sleep(100000);
		}
		else
			drawFooter(hdc, XbeginFooter, YbeginFooter, XendFooter, YendFooter);

		if (detals[1] == 0) {
			SetTextColor(hdc, RGB(255, 0, 0));
			hBrush = CreateSolidBrush(RGB(0, 0, 0));
			SelectObject(hdc, hBrush);
			Rectangle(hdc, 0, height * 5 / 6, width, height);
			Rectangle(hdc, 0, 0, width, 20);
			TextOut(hdc, 320, height / 2 + 250, L"Невозможно построить деталь: стены.Строительство окончено", 58);
			EndPaint(hwnd, &ps);
			Sleep(100000);
		}
		else {
			drawWalls(hdc, wallXB, wallYB, wallXE, wallYE);
			TextOut(hdc, wallXB + 20, wallYB + 20, L"КубГУ", 5);
			hBrush = CreateSolidBrush(RGB(0, 255, 0));
			SelectObject(hdc, hBrush);
			Rectangle(hdc, wallXB + 60, wallYB + 50, wallXE - 20, wallYE);
		}

		if (detals[2] == 0) {
			hBrush = CreateSolidBrush(RGB(0, 0, 0));
			SelectObject(hdc, hBrush);
			Rectangle(hdc, 0, height * 5 / 6, width, height);
			Rectangle(hdc, 0, 0, width, 20);
			SetTextColor(hdc, RGB(255, 0, 0));
			TextOut(hdc, 320, height / 2 + 250, L"Невозможно построить деталь: крыша.Строительство окончено", 58);
			EndPaint(hwnd, &ps);
			Sleep(100000);
		}
		else
			DrawRoof(hdc);

		if (detals[3] == 0) {
			SetTextColor(hdc, RGB(255, 0, 0));
			hBrush = CreateSolidBrush(RGB(0, 0, 0));
			SelectObject(hdc, hBrush);
			Rectangle(hdc, 0, height * 5 / 6, width, height);
			Rectangle(hdc, 0, 0, width, 20);
			TextOut(hdc, 320, height / 2 + 250, L"Невозможно построить деталь: флюгер.Строительство окончено", 59);
			EndPaint(hwnd, &ps);
			Sleep(100000);
		}
		else {
			drawFurer(hdc);
		}
		hBrush = CreateSolidBrush(RGB(0, 0, 0));
		SelectObject(hdc, hBrush);
		Rectangle(hdc, 0, height * 5 / 6, width, height);
		Rectangle(hdc, 0, 0, width, 20);
		hBrush = CreateSolidBrush(RGB(125, 125, 195));
		SelectObject(hdc, hBrush);
		Rectangle(hdc, 0, height * 5 / 6, width, height * 5 / 6 + 30);
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