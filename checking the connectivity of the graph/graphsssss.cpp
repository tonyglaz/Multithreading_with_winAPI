#include <iostream>
#include <thread>
#include <chrono>

using namespace std;

const int THREADS = 4;

// печатаем матрицу
void printMatrix(bool* matrix, int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            cout << matrix[i * size + j];
        }
        cout << endl;
    }
    cout << endl;
}

// половина вершин двудольного графа в первом мно-ве, вторая - во втором
void initM(bool* matrix, int size) {
    for (int i = 0; i < size / 2; i++) {
        for (int j = size / 2; j < size; j++) {
            matrix[i * size + j] = 1;
            matrix[j * size + i] = 1;
        }
    }
}

// удаляем существующие ребра
void removeEdge(bool* matrix, int size, int count) {
    int removed = 0;
    do {
        int i = rand() % size;
        int j = rand() % size;
        if (matrix[i * size + j]) {
            matrix[i * size + j] = 0;
            matrix[j * size + i] = 0;
            removed++;
        }
    } while (removed < count);
}


// вычисляем блок матрицы С с iStart строки по iStop строку и с jStart столбца по jStop столбец
void multThread(bool* A, bool* B, int size, bool* C, int iStart, int jStart, int iStop, int jStop) {
    for (int i = iStart; i < iStop; i++) {
        for (int j = jStart; j < jStop; j++) {
            for (int k = 0; k < size; k++) {
                C[i * size + j] = B[i * size + j];
                if (A[i * size + k] && B[j * size + k]) {
                    C[i * size + j] = true;
                    break;
                }
            }
        }
    }
}

// однопоточное перемножение матриц (в кольце 2)
// https://docplayer.ru/80120428-Aleksandr-ohotin-28-dekabrya-2017-g.html
// страница 2
// пункт 1.3
void seqMatrixMult(int size, bool* A, bool* B, bool* C) {
    multThread(A, B, size, C, 0, 0, size, size);
}


// делим матрицу по горизрнтали на THREADS прямоугольных фрагментов, каждый считаем в своём потоке
void parMatrixMult(int size, bool* A, bool* B, bool* C) {
    thread** arr = new thread * [THREADS];
    for (int i = 0; i < THREADS - 1; i++) {
        arr[i] = new thread(multThread, A, B, size, C, i * (size / THREADS), 0, (i + 1) * (size / THREADS), size);
    }
    arr[THREADS - 1] = new thread(multThread, A, B, size, C, (THREADS - 1) * (size / THREADS), 0, size, size);
    for (int i = 0; i < THREADS; i++) {
        if (arr[i]->joinable()) arr[i]->join();
    }
}

// принимаем матрицу транзитивного замыкания (если в ней все единицы - граф связный)
bool isTransitiveClosureConnected(bool* matrix, int size) {
    int s = 0;
    for (int i = 0; i < size * size; i++)
        if (matrix[i])
            s++;
    return s == size * size;
}

// последовательный вариант
// матрица А - наша матрица смежности, матрицы tmpB и tmpC - нулевые 
// (нужны, чтобы не тратить время на выделение памяти)
// возводим изначальную матрицу в степень log2(n), чтобы достоверно отыскать все пути в графе
// т.е. получить матрицу транзитивного замыкания
bool isGraphConnectedSeq(int size, bool* A, bool* tmpB, bool* tmpC) {
    bool* B = tmpB;
    bool* C = tmpC;
    seqMatrixMult(size, A, A, B);
    for (int i = 0; i < (int)log2(size) + 1; i++) {
        seqMatrixMult(size, A, B, C);
        swap(B, C);
    }
    return isTransitiveClosureConnected(B, size);
}

// параллельный вариант
bool isGraphConnectedPar(int size, bool* A, bool* tmpB, bool* tmpC) {
    bool* B = tmpB;
    bool* C = tmpC;
    parMatrixMult(size, A, A, B);
    for (int i = 0; i < (int)log2(size) + 1; i++) {
        parMatrixMult(size, A, B, C);
        swap(B, C);
    }
    return isTransitiveClosureConnected(B, size);
}


int main() {
    int size;
    cout << "Enter matrix size: ";
    cin >> size;

    int edgesToDelete;
    cout << "Enter edges to delete count: ";
    cin >> edgesToDelete;

    bool* A = new bool[size * size];
    memset(A, 0, size * size * sizeof(bool));
    initM(A, size);
    removeEdge(A, size, edgesToDelete);

    //printMatrix(matrix, size);

    bool* B = new bool[size * size];
    bool* C = new bool[size * size];

    bool isConnected = false;

    cout << "Sequential connection check..." << endl;
    auto seqStart = chrono::steady_clock::now();
    isConnected = isGraphConnectedSeq(size, A, B, C);
    auto seqStop = chrono::steady_clock::now();
    cout << (isConnected ? "Connected" : "Disconnected") << endl;
    cout << "Duration: " << chrono::duration_cast<chrono::milliseconds>(seqStop - seqStart).count() << " ms" << endl;

    memset(B, 0, size * size * sizeof(bool));
    memset(C, 0, size * size * sizeof(bool));

    isConnected = false;

    cout << "Parallel connection check..." << endl;
    seqStart = chrono::steady_clock::now();
    isConnected = isGraphConnectedPar(size, A, B, C);
    seqStop = chrono::steady_clock::now();
    cout << (isConnected ? "Connected" : "Disconnected") << endl;
    cout << "Duration: " << chrono::duration_cast<chrono::milliseconds>(seqStop - seqStart).count() << " ms" << endl;


    return 0;
}