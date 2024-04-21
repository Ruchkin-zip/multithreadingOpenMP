#include <iostream>
#include <iomanip>
#include <omp.h>
#include <chrono>
#include <locale.h>

using namespace std;
using namespace std::chrono;
typedef high_resolution_clock::time_point timeStamp;

struct vector {
    int* vec;
    int len;
    vector(int _len, bool gen = true) {
        len = _len;
        vec = new int[len];
        for (int i = 0; i < len; i++)
            if (gen)
                vec[i] = rand() % 10;
            else
                vec[i] = 0;
    }
    vector(const vector& anotherVector) {
        len = anotherVector.len;
        vec = new int[len];
        for (int i = 0; i < len; i++)
            vec[i] = anotherVector.vec[i];
    }

    ~vector() {
        delete vec;
    }
    void print() {
        cout << "Вектор = {";
        for (int i = 0; i < len; i++)
            cout << setw(3) << vec[i] << " ";
        cout << "}\r\n";
    }
    int& operator[](int inx) {
        return this->vec[inx];
    }

};

struct matrix {
    int** mat;
    int m, n;
    matrix(int _m, int _n, bool gen = true) {
        m = _m;
        n = _n;
        int tmp = 0;
        mat = new int* [m];
        for (int i = 0; i < m; i++) {
            mat[i] = new int[n];
            for (int j = 0; j < n; j++)
                if (gen)
                    mat[i][j] = rand() % 10;
                else
                    mat[i][j] = 0;
        }
    }
    ~matrix() {
        for (int i = 0; i < m; i++)
            delete mat[i];
        delete mat;
    }
    void print() {
        cout << "Matrix" << endl;
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < n; j++)
                cout << setw(3) << mat[i][j] << " ";
            cout << endl;
        }
        cout << endl;
    }
    int matrixElement(int m, int n) {
        return this->mat[m][n];
    }
};


// Однопоточное выполнение
vector multSingle(matrix& mat, vector& vec) {
    vector output(vec.len, false);
    if (mat.n != vec.len) {
        cout << "Ошибка: умножение невозможно" << endl;
        return output;
    }
    for (int i = 0; i < mat.m; i++)
        for (int j = 0; j < mat.n; j++)
            output[i] = output[i] + mat.matrixElement(i, j) * vec[j];
    return output;
}

// Параллельное выполнение с директивой for
vector multParallelFor(matrix& mat, vector& vec) {
    vector output(vec.len, false);
    if (mat.n != vec.len) {
        cout << "Ошибка: умножение невозможно" << endl;
        return output;
    }
    int i, j;
    #pragma omp parallel for private(i,j) shared(output,mat,vec)
    for (i = 0; i < mat.m; i++)
        for (j = 0; j < mat.n; j++)
            output[i] = output[i] + mat.matrixElement(i, j) * vec[j];

    return output;
}


// Параллельное выполнение по секциям
vector multParallelSection(matrix& mat, vector& vec) {
    vector output(vec.len, false);
    if (mat.n != vec.len) {
        cout << "Error: dims mismatch" << endl;
        return output;
    }
    int i, j;
    #pragma omp parallel private(i,j) shared(output,mat,vec)
    {
        int thID = omp_get_thread_num(); // получение номера потока
        int thCT = omp_get_max_threads(); // получение количества потоков
        for (i = omp_get_thread_num(); i < mat.m; i += thCT)
            for (j = 0; j < mat.n; j++)
                output[i] = output[i] + mat.matrixElement(i, j) * vec[j];
    }
    return output;
}

int main(int argc, char** argv) {

    setlocale(LC_ALL, "Rus");
    int iterations = 3; //количество повторений
    int mat_s, mat_c, size; //размерность матрицы
    int vec_l; //размерность вектора
    int method; //выбранный метод (1 - без, 2 - omp, 3 - ручное)
    int num_threads; //количество потоков

    /*
    cout << "Размерность матрицы:\nстроки = ";
    cin >> mat_s;
    cout << "столбцы = ";
    cin >> mat_c;
    cout << "Размерность вектора:\nстолбцы = ";
    cin >> vec_l;
    cout << "Введите метод подсчета: ";
    cin >> method;
    cout << "Введите количество потоков: ";
    cin >> num_threads;
    omp_set_num_threads(num_threads);
    srand(time(0));
    vector vec(vec_l);
    matrix mat(mat_s, mat_c);
    int j; //итератор по повторным вычислениям
    */


    cout << "Размерность : ";
    cin >> size;
    cout << "Введите метод подсчета: ";
    cin >> method;
    cout << "Введите количество потоков: ";
    cin >> num_threads;
    omp_set_num_threads(num_threads);
    srand(time(0));
    vector vec(size);
    matrix mat(size, size);
    int j; //итератор по повторным вычислениям

    timeStamp t1 = high_resolution_clock::now();
    switch (method) {
    case 1:
        for (j = 0; j < iterations; j++)
            vector result = multSingle(mat, vec);
        break;
    case 2:
        for (j = 0; j < iterations; j++)
            vector result = multParallelFor(mat, vec);
        break;
    case 3:
        for (j = 0; j < iterations; j++)
            vector result = multParallelSection(mat, vec);
        break;
    }
    timeStamp t2 = high_resolution_clock::now();
    duration<double> timeTook = duration_cast<duration<double>>(t2 - t1);
    cout << "Время = " << timeTook.count() / static_cast<double>(iterations) << endl;
    return 0;
}