// SoccerSim.cpp : Определяет точку входа для приложения.
//

#include "framework.h"
#include <iostream>
#include <windows.h>
#include <thread>
#include <mutex> // std::mutex 
#include <ctime>
#include <Mmsystem.h>
//these two headers are already included in the <Windows.h> header
#pragma comment(lib, "Winmm.lib")


using namespace std;
using namespace std::chrono;

HINSTANCE hInst;
HWND hwnd;

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

std::mutex mtx; // mutex для критической секции (в функции THfunction)
std::mutex interMtx; // mutex intersection


int width;
int height;
int adjustment;
int LEFT_BOUNDARY, right_line, back_line, BOTTOM_BOUNDARY;

int player_radius = 30; // Радиус футболистов
int ball_radius = 15; // Радиус мяча

int player_speed = 15;
int ball_speed = 25;

int player_x, player_y;
int player_x_after_goal, player_y_after_goal;
int gk_x, gk_y;
int def_x, def_y;
int ball_x, ball_y;
int player_direction[] = { 0, 0 };
int ball_direction;
double win_probability = 0.5;
bool pressed_keys[4] = { 0, 0, 0, 0 };
bool finished;
bool shotOrNot = false;
bool goal = false;
bool remember = false;
int remember_player_position[20000];

double fRand()
{
    srand(time(NULL));
    int useless = rand() % 666;
    double f = (double)rand() / RAND_MAX;
    return -0.4 + f * (0.4 - (-0.4));
}
double fkek = fRand();

int getAmountOfPressedKeys() {
    int counter = 0;
    for (int i = 0; i < 4; i++) {
        if (pressed_keys[i])
            counter++;
    }
    return counter;
}

void THBall() {
    ball_x = player_x;
    ball_y = player_y;

    while (true) {
        mtx.lock();
        if (finished) {
            mtx.unlock();
            break;
        }
        if (ball_x > right_line + ball_radius && ball_x < width - adjustment && ball_y < height * 9 / 10 - adjustment - back_line - ball_radius && ball_y > height / 10 + adjustment + back_line + ball_radius) {
            finished = true;
            goal = true;
            mtx.unlock();
            InvalidateRect(hwnd, 0, 1);
            UpdateWindow(hwnd);
            break;
        }
        if (ball_x < LEFT_BOUNDARY - ball_radius || ball_x > right_line + ball_radius || ball_y < back_line - ball_radius || ball_y > BOTTOM_BOUNDARY + ball_radius) {
            finished = true;
            mtx.unlock();
            InvalidateRect(hwnd, 0, 1);
            UpdateWindow(hwnd);
            break;
        }
        ball_x += ball_speed;
        ball_y += ball_speed * fkek;

        mtx.unlock();
        InvalidateRect(hwnd, 0, 1);
        UpdateWindow(hwnd);
        Sleep(15);
    }
}

void THPlayer(HWND hwnd) {
    while (true) {
        mtx.lock();
        if (!goal && finished) {
            mtx.unlock();
            break;
        }
        if (!shotOrNot && (player_x < LEFT_BOUNDARY - player_radius || player_x > right_line + player_radius || player_y < back_line - player_radius || player_y > BOTTOM_BOUNDARY + player_radius)) {
            finished = true;
            mtx.unlock();
            break;
        }
        player_x += player_direction[0] * player_speed;
        player_y += player_direction[1] * player_speed;
        mtx.unlock();
        InvalidateRect(hwnd, 0, 1);
        UpdateWindow(hwnd);
        Sleep(25);
    }
}

void THGoalkeeper(HWND hwnd) {
    int temp_position;
    int difference;
    bool tried_to_catch = false;
    while (true) {
        mtx.lock();
        if (finished) {
            mtx.unlock();
            break;
        }
        if (!shotOrNot && (gk_x - player_x) * (gk_x - player_x) + (gk_y - player_y) * (gk_y - player_y) < (player_radius + player_radius) * (player_radius + player_radius)) {
            if (!tried_to_catch && (rand() % 101) > 30) {
                finished = true;
                mtx.unlock();
                break;
            }
            tried_to_catch = true;
        }
        else {
            tried_to_catch = false;
        }
        if ((gk_x - ball_x) * (gk_x - ball_x) + (gk_y - ball_y) * (gk_y - ball_y) < (player_radius + ball_radius) * (player_radius + ball_radius)) {
            if (!tried_to_catch && (rand() % 101) > win_probability * 100) {
                finished = true;
                mtx.unlock();
                break;
            }
            tried_to_catch = true;
        }
        if (!shotOrNot)
            difference = player_direction[1] * player_speed * (rand() % 2 ? (rand() % 301) / 100 : 1);
        else {
            int gk_direction = 0;
            switch (ball_direction)
            {
            case 1:
                gk_direction -= 1;
                break;
            case 2:
                gk_direction = ball_y > gk_y ? 1 : -1;
                break;
            case 3:
                gk_direction += 1;
                break;
            }
            difference = gk_direction * ball_speed * (rand() % 2 ? (rand() % 51) / 100 : 1);
        }
        temp_position = gk_y + difference;
        if ((player_x < width / 2 && !shotOrNot) || temp_position > height * 9 / 10 - adjustment - back_line - player_radius || temp_position < height / 10 + adjustment + back_line + player_radius) {
            mtx.unlock();
            continue;
        }
        gk_y = temp_position;
        mtx.unlock();
        InvalidateRect(hwnd, 0, 1);
        UpdateWindow(hwnd);

        Sleep(25);
    }
}

void THDefender(HWND hwnd) {
    int temp_position_x;
    int temp_position_y;
    int def_difference_x=0;
    int def_difference_y;
    int compare_result_x=0;
    int compare_result_y;
    bool tried_to_catch = false;
    while (true) {
        mtx.lock();
        if (finished) {
            mtx.unlock();
            break;
        }
        if (!shotOrNot && (def_x - player_x) * (def_x - player_x) + (def_y - player_y) * (def_y - player_y) < (player_radius + player_radius) * (player_radius + player_radius)) {
            if (!tried_to_catch && (rand() % 101) > 20) {
                finished = true;
                mtx.unlock();
                break;
            }
            tried_to_catch = true;
        }
        else {
            tried_to_catch = false;
        }
        if ((def_x - ball_x) * (def_x - ball_x) + (def_y - ball_y) * (def_y - ball_y) < (player_radius + ball_radius) * (player_radius + ball_radius)) {
            finished = true;
            tried_to_catch = true;
            mtx.unlock();
            break;
        }
        if (!shotOrNot) {
            if (player_x < width / 3) {

                if (def_y < height / 2)
                    def_y += player_speed;
                else if (def_y > height / 2)
                    def_y -= player_speed;
                mtx.unlock();
                InvalidateRect(hwnd, 0, 1);
                UpdateWindow(hwnd);
                Sleep(25);
                continue;
            }
            else {
                compare_result_y = 0;

                if (player_y > def_y)
                    compare_result_y = 1;
                else if (player_y < def_y)
                    compare_result_y = -1;

                double deflection = ((rand() % 101) < 30 ? (rand() % 81) / 100 : 1);

                def_difference_y = compare_result_y * player_speed * deflection;
            }
        }
        else {
            compare_result_y = 0;

            if (ball_y > def_y)
                compare_result_y = 1;
            else if (ball_y < def_y)
                compare_result_y = -1;

            def_difference_y = compare_result_y * player_speed * ((rand() % 101) < 30 ? (rand() % 151) / 100 : 1);
        }
        temp_position_x = def_x + def_difference_x;
        temp_position_y = def_y + def_difference_y;
        if (temp_position_y < back_line || temp_position_y > BOTTOM_BOUNDARY || temp_position_x > right_line || temp_position_x < LEFT_BOUNDARY) {
            mtx.unlock();
            continue;
        }
        def_x = temp_position_x;
        def_y = temp_position_y;
        mtx.unlock();
        InvalidateRect(hwnd, 0, 1);
        UpdateWindow(hwnd);

        Sleep(25);
    }
}
void playkVika() {
    while (!goal) {
        Sleep(1);
    }
    PlaySound(L"C:/Users/Пользователь/source/repos/penal'ti/penal'ti/vika.wav", NULL, SND_ASYNC);
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Разместите код здесь.

    MyRegisterClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    MSG msg;

    RECT rect;
    GetClientRect(hwnd, &rect);
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;
    adjustment = width / 10;

    LEFT_BOUNDARY = 0;
    right_line = width - adjustment * 1.5;
    back_line = player_radius * 0.4;
    BOTTOM_BOUNDARY = height - player_radius * 3;

    player_x = width / 10;
    player_y = height / 2;

    ball_x = (-1) * ball_radius;
    ball_y = (-1) * ball_radius;

    gk_x = right_line - player_radius;
    gk_y = height / 2;

    def_x = width * 4 / 7;
    def_y = height / 2;

    srand(time(NULL));

    thread player(THPlayer, hwnd);
    player.detach();
    thread goalkeeper(THGoalkeeper, hwnd);
    goalkeeper.detach();
    thread vika(playkVika);
    vika.detach();
    thread defender(THDefender, hwnd);
    defender.detach();
    // Цикл основного сообщения:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = CreateSolidBrush(RGB(34, 139, 34));
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"MyClass";
    wcex.hIconSm = NULL;

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

    RECT rect;
    rect.left = 0;
    rect.right = GetSystemMetrics(SM_CXSCREEN);
    rect.top = 0;
    rect.bottom = GetSystemMetrics(SM_CYSCREEN);

    DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
    AdjustWindowRect(&rect, dwStyle, false);

    hwnd = CreateWindowW(L"MyClass", L"MonkaS", dwStyle,
        CW_USEDEFAULT, 0, rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, hInstance, nullptr);

    if (!hwnd)
    {
        return FALSE;
    }

    ShowWindow(hwnd, SW_SHOWMAXIMIZED);
    UpdateWindow(hwnd);

    return TRUE;
}

void DrawBall(HDC hdc, RECT rectClient, int left, int top, int right, int bottom, HBRUSH hBrush)
{
    SelectObject(hdc, hBrush);
    Ellipse(hdc, left, top, right, bottom);
    DeleteObject(hBrush);
}

void DrawFootballGoals(HDC hdc)
{
    MoveToEx(hdc, right_line, height / 10 + adjustment + back_line, 0);
    LineTo(hdc, width - adjustment, height / 10 + adjustment + back_line);
    MoveToEx(hdc, width - adjustment, height / 10 + adjustment + back_line, 0);
    LineTo(hdc, width - adjustment, height * 9 / 10 - adjustment - back_line);
    MoveToEx(hdc, width - adjustment, height * 9 / 10 - adjustment - back_line, 0);
    LineTo(hdc, right_line, height * 9 / 10 - adjustment - back_line);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        HBRUSH hBrush; // кисть для рисования
        HPEN hpen;

        // Построение бьющего игрока, вратаря и защитника
        if (goal) {
            hBrush = CreateSolidBrush(RGB(255, 223, 196));
            DrawBall(hdc, ps.rcPaint, player_x - player_radius, player_y - player_radius, player_x + player_radius, player_y + player_radius, hBrush);
        }
        else {
            hBrush = CreateSolidBrush(RGB(0, 0, 255));
            DrawBall(hdc, ps.rcPaint, player_x - player_radius, player_y - player_radius, player_x + player_radius, player_y + player_radius, hBrush);
        }
        hBrush = CreateSolidBrush(RGB(255, 255, 0));
        DrawBall(hdc, ps.rcPaint, gk_x - player_radius, gk_y - player_radius, gk_x + player_radius, gk_y + player_radius, hBrush);
        DrawBall(hdc, ps.rcPaint, def_x - player_radius, def_y - player_radius, def_x + player_radius, def_y + player_radius, hBrush);

        // Построение мячика
        hpen = CreatePen(PS_SOLID, 5, 0x000000);
        hBrush = CreateSolidBrush(RGB(214, 214, 214));
        SelectObject(hdc, hpen);
        SelectObject(hdc, hBrush);
        DrawBall(hdc, ps.rcPaint, ball_x - ball_radius, ball_y - ball_radius, ball_x + ball_radius, ball_y + ball_radius, hBrush);

        hpen = CreatePen(PS_SOLID, 5, 0xFFFFFF);
        SelectObject(hdc, hpen);
        DrawFootballGoals(hdc);
        if (goal) {
            SetTextColor(hdc, 0xFFFFFF);
            SetBkColor(hdc, RGB(34, 139, 34));
            TextOut(hdc, width / 2-200,50, L"Теперь вы можете бегать по полю без футболки", 57);

            if (!remember) {
                player_x_after_goal = player_x;
                player_y_after_goal = player_y;
            }
            remember = true;
            hBrush = CreateSolidBrush(RGB(0, 0, 255));
            DrawBall(hdc, ps.rcPaint, player_x_after_goal - player_radius, player_y_after_goal - player_radius, player_x_after_goal + player_radius, player_y_after_goal + player_radius, hBrush);
        }

        DeleteObject(hpen);
        DeleteObject(hBrush);
        EndPaint(hWnd, &ps);
    }
    break;

    case WM_KEYDOWN:
        if (wParam == 0x32 && !shotOrNot)
        {
            mtx.lock();
            shotOrNot = true;
            
                win_probability = 0.8;
            mtx.unlock();
            thread THBall(THBall);
            ball_direction = 2;
            THBall.detach();
        }
        if (wParam == VK_SPACE)
        {
            player_direction[0] = 0;
            player_direction[1] = 0;
        }
        if (wParam == VK_UP || wParam == 0x57)
        {
            pressed_keys[0] = true;
            int amount_of_pressed_keys = getAmountOfPressedKeys();
            if (amount_of_pressed_keys == 1 || amount_of_pressed_keys > 2) {
                player_direction[0] = 0;
                player_direction[1] = -1;
            }
            else {
                if (pressed_keys[1]) {
                    player_direction[0] = 0;
                    player_direction[1] = -1;
                }
                else if (pressed_keys[2]) {
                    player_direction[0] = -1;
                    player_direction[1] = -1;
                }
                else if (pressed_keys[3]) {
                    player_direction[0] = 1;
                    player_direction[1] = -1;
                }

            }
        }

        else if (wParam == VK_DOWN || wParam == 0x53)
        {
            pressed_keys[1] = true;
            int amount_of_pressed_keys = getAmountOfPressedKeys();
            if (amount_of_pressed_keys == 1 || amount_of_pressed_keys > 2) {
                player_direction[0] = 0;
                player_direction[1] = 1;
            }
            else {
                if (pressed_keys[0]) {
                    player_direction[0] = 0;
                    player_direction[1] = 1;
                }
                else if (pressed_keys[2]) {
                    player_direction[0] = -1;
                    player_direction[1] = 1;
                }
                else if (pressed_keys[3]) {
                    player_direction[0] = 1;
                    player_direction[1] = 1;
                }

            }
        }

        else if (wParam == VK_LEFT || wParam == 0x41)
        {
            pressed_keys[2] = true;
            int amount_of_pressed_keys = getAmountOfPressedKeys();
            if (amount_of_pressed_keys == 1 || amount_of_pressed_keys > 2) {
                player_direction[0] = -1;
                player_direction[1] = 0;
            }
            else {
                if (pressed_keys[0]) {
                    player_direction[0] = -1;
                    player_direction[1] = -1;
                }
                else if (pressed_keys[1]) {
                    player_direction[0] = -1;
                    player_direction[1] = 1;
                }
                else if (pressed_keys[3]) {
                    player_direction[0] = -1;
                    player_direction[1] = 0;
                }

            }
        }

        else if (wParam == VK_RIGHT || wParam == 0x44)
        {
            pressed_keys[3] = true;
            int amount_of_pressed_keys = getAmountOfPressedKeys();
            if (amount_of_pressed_keys == 1 || amount_of_pressed_keys > 2) {
                player_direction[0] = 1;
                player_direction[1] = 0;
            }
            else {
                if (pressed_keys[0]) {
                    player_direction[0] = 1;
                    player_direction[1] = -1;
                }
                else if (pressed_keys[1]) {
                    player_direction[0] = 1;
                    player_direction[1] = 1;
                }
                else if (pressed_keys[2]) {
                    player_direction[0] = 1;
                    player_direction[1] = 0;
                }

            }
        }
        break;

    case WM_KEYUP:
        if (wParam == VK_UP || wParam == 0x57)
        {
            pressed_keys[0] = false;
        }
        else if (wParam == VK_DOWN || wParam == 0x53)
        {
            pressed_keys[1] = false;
        }
        else if (wParam == VK_LEFT || wParam == 0x41)
        {
            pressed_keys[2] = false;
        }
        else if (wParam == VK_RIGHT || wParam == 0x44)
        {
            pressed_keys[3] = false;
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}