#include <iostream>
#include <iomanip>
#include <omp.h>
#include <chrono>
#include <locale.h>


using namespace std;
using namespace std::chrono;

typedef high_resolution_clock::time_point timeStamp;

double count_error(double A[], double B[], int N_SIZE) {
    double maxDif = 0;
    double currentDif = 0;
    int i;
    #pragma omp parallel for shared(maxDif)
    for (i = 0; i < N_SIZE; i++) {
        // ���� ������ 0, �� ����� � ��������������� ������
        currentDif = (A[i] - B[i] < 0) ? -(A[i] - B[i]) : A[i] - B[i];
        if (currentDif > maxDif)
            maxDif = currentDif;
    }
    return maxDif;
}

int main() {

    setlocale(LC_ALL, "Rus");
    int i, j;
    cout << "������ ������� � �������: ";
    int N_SIZE;
    cin >> N_SIZE;

    cout << "�������� ������� � ������ (1 ��� 0): ";
    int showInput;
    cin >> showInput;

    // �������������
    double** Amat = new double* [N_SIZE];
    for (i = 0; i < N_SIZE; i++)
        Amat[i] = new double[N_SIZE];
    double* Bvec = new double[N_SIZE];
    double* Xvector = new double[N_SIZE];
    double* XvectorPrev = new double[N_SIZE];

    // ���������
    for (i = 0; i < N_SIZE; i++) {
        for (j = 0; j < N_SIZE; j++) {
            Amat[i][j] = (double)(rand() % 80 - 40) / 100.0;
        }
        Bvec[i] = (double)(rand() % 40 - 20) / 100.0;
    }
    int sgn = 1;
    for (i = 0; i < N_SIZE; i++) {
        Amat[i][i] = sgn * ((rand() % 10) / 5.0 + 1.0);
        sgn *= -1;
    }

    // ����� ���������������� ������
    if (showInput) {
        cout << "\n������ ��������� Ax = B\n";
        cout << "A =\n";
        for (i = 0; i < N_SIZE; i++) {
            for (j = 0; j < N_SIZE; j++)
                cout << setw(6) << Amat[i][j] << " ";
            cout << "\n";
        }
        cout << "B =\n";
        for (i = 0; i < N_SIZE; i++)
            cout << setw(6) << Bvec[i] << " ";
        cout << "\n";
    }

    int maxIters = 10;
    int currentIter;
    timeStamp t1, t2;
    double error = 100;
    double eps = 0.01;

    t1 = high_resolution_clock::now();

    // ������ ������ ��� �������� ����������� ����
    // ��� ����� ������ �������� �� ����� ������ ����
    for (i = 0; i < N_SIZE; i++)
        XvectorPrev[i] = Bvec[i] / Amat[i][i];

    for (currentIter = 0; (currentIter < maxIters) && (error >= eps); currentIter++) {

        // ���� ��� �� ������ ��������, ������������� ������ � � ����������� ����
        if (currentIter > 0)
            for (i = 0; i < N_SIZE; i++)
                XvectorPrev[i] = Xvector[i];

        // ������������ ������ � ��� �������� ����
        #pragma omp parallel for
        for (i = 0; i < N_SIZE; i++) {
            Xvector[i] = Bvec[i];
            for (j = 0; j < N_SIZE; j++)
                if (i != j)
                    Xvector[i] -= Amat[i][j] * XvectorPrev[j];
            Xvector[i] /= Amat[i][i];
        }

        // ������������ ����������� - ������� ��������� ������������
        error = count_error(Xvector, XvectorPrev, N_SIZE);
    }

    t2 = high_resolution_clock::now();
    duration<double> time = duration_cast<duration<double>>(t2 - t1);

    if (showInput) {
        cout.precision(3);
        cout << "\nx =\n";
        for (i = 0; i < N_SIZE; i++)
            cout << fixed << setw(6) << Xvector[i] << " ";
        cout << "\n";
    }
    cout << "\n���������� ��������, �� ������� ���� ������� �������: " << currentIter - 1 << endl;
    cout << "�����������, �� ������� ������������: " << error << endl;
    cout << "����� ����������: " << time.count() << endl;

    delete[] Bvec;
    for (i = 0; i < N_SIZE; i++)
        delete Amat[i];
    delete[] Amat;

    return 0;
}
