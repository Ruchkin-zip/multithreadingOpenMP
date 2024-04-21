#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"
#include <locale.h>
#include <iomanip>

using namespace std;

int main(int argc, char** argv)
{
    setlocale(LC_ALL, "Rus");
    int i, j, k;
    int* equation_tr_list;
    double** A;      // матрица А
    double* b;      // столбец В
    double* tmp;    // временный вектор
    double* x;      // искомый вектор Х
    double sum = 0.0;
    int n, show;
    cout << "Размерность: ";
    cin >> n;
    cout << "Показывать данные (0 - нет, 1 - да): ";
    cin >> show;
    int calling_process_rank, available_process_num;
    double time;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &calling_process_rank);       // ранг вызывающего процесса
    MPI_Comm_size(MPI_COMM_WORLD, &available_process_num);      // общее количество доступных процессов

    if (calling_process_rank == 0)
    {
        double rand_num;
        A = new double*[n];
        b = new double[n];
        // Заполнение матрицы и вектора случайными значениями исключая нуль
        for (i = 0; i < n; i++)
        {
            A[i] = new double[n];
            for (j = 0; j < n; j++) {
                do
                {
                    rand_num = rand() % 41 - 20;
                } while (rand_num == 0.0);
                A[i][j] = rand_num;
            }
            
            do
            {
                rand_num = rand() % 41 - 20;
            } while (rand_num == 0.0);
            b[i] = rand_num;
        }
        // вывод исходных данных если требуется
        if (show == 1) {

            cout << "\nA =\n";
            for (i = 0; i < n; i++) {
                for (j = 0; j < n; j++) 
                    cout << setw(6) << A[i][j] << " ";
                cout << "\n";
            }

            cout << "B =\n";
            for (i = 0; i < n; i++)
                cout << setw(6) << b[i] << "\n";
            cout << "\n";

        }
    }

    // старт таймера
    time = MPI_Wtime();

    // рассылка
    MPI_Bcast(&A, n * n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(&b, n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // циклическое распределение строк по процессам 
    equation_tr_list = new int[n];
    for (i = 0; i < n; i++)
    {
        equation_tr_list[i] = i % available_process_num;
    }

    // прямой ход алгоритма - упрощение уравнений
    tmp = new double[n];
    for (k = 0; k < n; k++)
    {
        // рассылка
        MPI_Bcast(&A[k][k], n - k, MPI_DOUBLE, equation_tr_list[k], MPI_COMM_WORLD);
        MPI_Bcast(&b[k], 1, MPI_DOUBLE, equation_tr_list[k], MPI_COMM_WORLD);
        for (i = k + 1; i < n; i++)
        {
            if (equation_tr_list[i] == calling_process_rank)
            {
                tmp[i] = A[i][k] / A[k][k];
            }
        }
        for (i = k + 1; i < n; i++)
        {
            if (equation_tr_list[i] == calling_process_rank)
            {
                for (j = 0; j < n; j++)
                {
                    A[i][j] = A[i][j] - (tmp[i] * A[k][j]);
                }
                b[i] = b[i] - (tmp[i] * b[k]);
            }
        }
    }

    x = new double[n];
    // обратный ход - поиск переменных
    if (calling_process_rank == 0)
    {
        x[n - 1] = b[n - 1] / A[n - 1][n - 1];
        for (i = n - 2; i >= 0; i--)
        {
            sum = 0;

            for (j = i + 1; j < n; j++)
            {
                sum = sum + A[i][j] * x[j];
            }
            x[i] = (b[i] - sum) / A[i][i];
        }

        // остановка таймера
        time = MPI_Wtime() - time;
        
        // вывод вектора Х, если требуется
        if (show == 1) {
            cout << "X =\n";
            for (i = 0; i < n; i++)
                cout << setw(6) << "x" << i << " = " << setw(6) << setprecision(3) << x[i] << "\n";
        }
        
        // вывод результата
        cout << "\nВремя: " << time << "\n";
    }

    MPI_Finalize();
    return(0);
}
