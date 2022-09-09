#include <iostream>
#include <fstream>
#include <cmath>
#include <ctime>
#include "omp.h"
using namespace std;

int random(int n) {// генератор случайного целого числа из [1;n]
    return rand() % n + 1;
}

void random_init() { // необходимо, чтобы при разных запусках программы были различные "случайные" числа
    time_t t;
    time(&t);
    srand((unsigned int)t);
}

bool** create_static_objects(int n, int m) {
    bool** map = new bool* [n];
#pragma omp parallel for
    for (int i = 0; i < n; i++) {
        map[i] = new bool[m];
        for (int j = 0; j < m; j++)
            map[i][j] = 0;
    }
    int count = (random(6) + 6) * n * m / 100; // кол-во статичных объектов (7-12% от общего числа клеток)
#pragma omp parallel for
    for (int i = 0; i < count; i++) {
        int x, y;
        do {
            x = random(n) - 1;
            y = random(m) - 1;
        } while (map[x][y] == 1);
        map[x][y] = 1;
    }
    return map;
}

void printMap(bool** map, int n, int m) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++)
            cout << map[i][j] << ' ';
        cout << endl;
    }
    cout << endl;
}

void build_path(bool** static_map, bool** map, bool**& path, int n, int m, int& frames) {
#pragma omp parallel for
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            if (static_map[i][j] != map[i][j]) {
                path[i][j] = 1;
                frames++;
            }
}

void MNK(bool** path, int n, int m, int frames, float& k0, float& b0) {
    int* x = new int[frames];
    int* y = new int[frames];
    int frames_t = 0;
#pragma omp parallel for
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            if (path[i][j] == 1) {
#pragma omp critical
                {
                    x[frames_t] = i;
                    y[frames_t] = j;
                    frames_t++;
                }
            }
    float sum_x = 0, sum_y = 0, sum_x2 = 0, sum_xy = 0;
#pragma omp parallel for reduction (+: sum_x, sum_y, sum_x2, sum_xy)
    for (int i = 0; i < frames; i++) {
        sum_x += x[i];
        sum_y += y[i];
        sum_x2 += x[i] * x[i];
        sum_xy += x[i] * y[i];
    }
    sum_x /= frames;
    sum_y /= frames;
    sum_x2 /= frames;
    sum_xy /= frames;

    k0 = (sum_x * sum_y - sum_xy) / (sum_x * sum_x - sum_x2);
    b0 = sum_y - k0 * sum_x;
}


void main() {
    int stream;     // число потоков для параллельных участков
    cout << "Enter number of streams for parallel regions : ";
    cin >> stream;
    system("cls");
    omp_set_num_threads(stream);

    int n = 1200, m = 1500;                                 // размерность карты
    random_init();                                      // инициализируем рандомный seed

    bool** static_map = create_static_objects(n, m);     // создаем карту статичных объектов

    bool** path = new bool* [n], ** map = new bool* [n];   // карта для отрисовки траектории марсохода (по исходным кадрам), карта для отрисовки кадров
#pragma omp parallel for
    for (int i = 0; i < n; i++) {
        path[i] = new bool[m];
        map[i] = new bool[m];
        for (int j = 0; j < m; j++) {
            path[i][j] = 0;
            map[i][j] = static_map[i][j];
        }
    }

    float k0, b0;               // предсказываемые коэффициенты в уравнении, по ткорому летит марсоход
    float k = random(3) - 0.6;  // коэффициенты в уравнении y=kx+b, по которому летит марсоход
    float b = random(n / 8);
    int step = (int)(n / 24);
    int frames = 0;             // кол-во кадров


    for (int i = 1; (i < n) && ((k * i + b) < m); i += step) {
        map[i][(int)(k * i + b)] = 1;
        build_path(static_map, map, path, n, m, frames);
        map[i][(int)(k * i + b)] = static_map[i][(int)(k * i + b)];
    }

    MNK(path, n, m, frames, k0, b0);

    cout << "Real equation of trajectory: y = " << k << " * x + " << b << endl;
    cout << "Estimated equation of trajectory: y = " << k0 << " * x + " << b0 << endl;


}