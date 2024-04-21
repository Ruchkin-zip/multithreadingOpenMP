#include "mpi.h"
#include "fstream"
#include <iostream>
#include <chrono>

double startT, stopT;

using namespace std::chrono;
typedef high_resolution_clock::time_point timeStamp;


// ������� ���� �������� �� ���������� �����������
int* mergeArrays(int* v1, int n1, int* v2, int n2)
{
    int i, j, k;
    int* result;

    result = new int[n1 + n2];
    i = 0;
    j = 0;
    k = 0;

    while (i < n1 && j < n2)
        if (v1[i] < v2[j]) {
            result[k] = v1[i];
            i++;
            k++;
        }
        else {
            result[k] = v2[j];
            j++;
            k++;
        }

    if (i == n1)
        while (j < n2) {
            result[k] = v2[j];
            j++;
            k++;
        }

    if (j == n2)
        while (i < n1) {
            result[k] = v1[i];
            i++;
            k++;
        }

    return result;
}

// ������� ����������
void bubble_sort(int* v, int n)
{
    int i, j, temp;
    for (i = n - 2; i >= 0; i--) {
        for (j = 0; j <= i; j++) {
            if (v[j] > v[j + 1]) {
                temp = v[j];
                v[j] = v[j+1];
                v[j+1] = temp;
            }
        }
    }
}

using namespace std;

int main(int argc, char** argv)
{
    int* original_array{}; // �������� ������
    int* result_array; // ��������� ����������
    int* sub;
    int calling_process_rank, available_process_num;
    int m, n, r, s, i, step;
    MPI_Status status;

    int method;
    cout << "Sort method (1 - parallel, 2 - sequental) : ";
    cin >> method;
    cout << "Array size : ";
    cin >> n;

    ofstream input_file("input.txt");
    ofstream output_file("output.txt");

    switch (method) {
    case 1:
    {
        MPI_Init(&argc, &argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &calling_process_rank); // ���� ����������� ��������
        MPI_Comm_size(MPI_COMM_WORLD, &available_process_num); // ����� ���������� ��������� ���������

        // ���� ������� ������� - �������
        if (calling_process_rank == 0) {
            s = n / available_process_num;
            r = n % available_process_num;
            original_array = new int[n + s - r]; // ��� �����, ����� ����� � ������� ����� �������� �� ���-�� ��������� � ���� ������

            // ��������� ���������� ������� ���������� ��� ������������ ���������� �����������
            srand(unsigned int(MPI_Wtime()));
            for (i = 0; i < n; i++)
            {
                original_array[i] = rand() % 15000; // ����� � ��������� �� 0 �� 14999
                input_file << original_array[i] << " ";
            }
            input_file.close();

            // ������ ����������
            startT = MPI_Wtime();
            // ��������� ������� ����� ��� ���������� ���� ��������� ���������
            MPI_Bcast(&s, 1, MPI_INT, 0, MPI_COMM_WORLD);
            // ������������� ���������� ���������� �����
            result_array = new int[s];
            // �������� ������� ������ �� �������� �������� ���� ���������
            MPI_Scatter(original_array, s, MPI_INT, result_array, s, MPI_INT, 0, MPI_COMM_WORLD);
            // ���������� ������� ��������� ������ �����
            bubble_sort(result_array, s);
        }
        else {
            MPI_Bcast(&s, 1, MPI_INT, 0, MPI_COMM_WORLD);
            // ���������� ���������� ���������� �����
            result_array = new int[s];
            MPI_Scatter(original_array, s, MPI_INT, result_array, s, MPI_INT, 0, MPI_COMM_WORLD);
            // ������ ��-�������-��������� ��������� ���� ���� ������� ������ s
            bubble_sort(result_array, s);
        }

        // ������ ������� ����� ���� �������� �� ����������� �����
        step = 1;

        // ����������� ��������������� ������ ��� ��������� �������-����������
        while (step < available_process_num) {
            if (calling_process_rank % 2 == 0) {
                // ���� ������� - ������, �� �� �������� ������ �� ������
                if (calling_process_rank + step < available_process_num) {
                    // ��������� ������� �����
                    MPI_Recv(&m, 1, MPI_INT, calling_process_rank + step, 0, MPI_COMM_WORLD, &status);
                    // ��������� ����� ��� ����
                    sub = new int[m];
                    // ��������� �����
                    MPI_Recv(sub, m, MPI_INT, calling_process_rank + step, 0, MPI_COMM_WORLD, &status);
                    // ������� ������������ ���������������� ������� � ����������
                    result_array = mergeArrays(result_array, s, sub, m);
                    s = s + m;
                }
            }
            else {
                // ���� ������� - ��������, �� �� ��������� �������� ������ ������
                int near = calling_process_rank - step;
                MPI_Send(&s, 1, MPI_INT, near, 0, MPI_COMM_WORLD);
                MPI_Send(result_array, s, MPI_INT, near, 0, MPI_COMM_WORLD);
                break;
            }

            // �������� �������� �� 2
            step = step + 2;
        }

        // ����� ���������� � ������ ���������
        if (calling_process_rank == 0) {
            stopT = MPI_Wtime();
            cout << "Parallel time: " << stopT - startT;
            cout << "\nNumber of process: " << available_process_num << "\nParallel time : " << stopT - startT << "\n";
            for (i = 0; i < n; i++) output_file << result_array[i] << " ";
            output_file.close();
        }

        MPI_Finalize();

        break;
    }
    case 2:
    {
        original_array = new int[n];

        srand(time(NULL));
        ofstream file("input.txt");
        for (i = 0; i < n; i++)
        {
            original_array[i] = rand() % 15000; // ����� � ��������� �� 0 �� 14999
            input_file << original_array[i] << " ";
        }
        input_file.close();

        timeStamp t1 = high_resolution_clock::now();
        bubble_sort(original_array, n);
        timeStamp t2 = high_resolution_clock::now();
        duration<double> timeTook = duration_cast<duration<double>>(t2 - t1);

        cout << "Time: " << timeTook.count();

        for (i = 0; i < n; i++) output_file << original_array[i] << " ";
        output_file.close();

        break;
    }

    }

    return 0;

}
