#include<iostream>
#include<iomanip>
#include<omp.h>
#include<chrono>
#include<vector>

#define SHOW 0

using namespace std;
using namespace std::chrono;
typedef high_resolution_clock::time_point timeStamp;

void solveMatrix(int N, float* UPPER, float* CENTRAL, float* BOTTOM, float* F, float* x)
{
	float m;
	int i;
	#pragma omp parallel for shared(i)
	for (i = 1; i < N; i++) {
		m = BOTTOM[i] / CENTRAL[i - 1];
		CENTRAL[i] = CENTRAL[i] - m * UPPER[i - 1];
		F[i] = F[i] - m * F[i - 1];
	}

	x[N - 1] = F[N - 1] / CENTRAL[N - 1];

	#pragma omp parallel for shared(i)
	for (i = N - 2; i >= 0; i--)
		x[i] = (F[i] - UPPER[i] * x[i + 1]) / CENTRAL[i];
}

void generateMatrix(int N, float* UPPER, float* CENTRAL, float* BOTTOM) {
	int i, j;
	for (i = 0; i < N; i++) {
		UPPER[i] = static_cast<float>(rand() % 100);
		BOTTOM[i] = static_cast<float>(rand() % 100);
		CENTRAL[i] = static_cast<float>(rand() % 100);
	}
}

void generateVector(int N, float* vec) {
	int i;
	for (i = 0; i < N; i++)
		vec[i] = static_cast<float>(rand() % 100);
}

void printVector(int N, float* vec) {
	int i;
	cout << "[";
	for (i = 0; i < N; i++)
		cout << setw(7) << setprecision(3) << vec[i];
	cout << "]\n";
}

void printMatrix(int N, float* TOP, float* CENTRAL, float* BOTTOM) {
	int i, j;
	float** A;
	A = new float* [N];
	for (i = 0; i < N; i++) {
		A[i] = new float[N];
		for (j = 0; j < N; j++) {
			if (i == j) A[i][j] = CENTRAL[i];
			else if (j == i + 1) A[i][j] = TOP[i];
			else if (i == j + 1) A[i][j] = BOTTOM[i];
			else A[i][j] = 0;
		}
	}
	for (i = 0; i < N; i++) {
		cout << "[";
		for (j = 0; j < N; j++) {
			cout << setw(6) << A[i][j] << " ";
		}
		cout << "]\n";
	}
	delete A;
}

float* UPPER;
float* CENTRAL;
float* BOTTOM;
float* F;
float* X;

double test(int N, int M) {

	srand(time(NULL));
	timeStamp t1, t2;

	// устанавливаем количество потоков
	#ifdef _OPENMP
		omp_set_dynamic(0);
		omp_set_num_threads(M);
	#endif

	// выделяем память, генерируем матрицу и вектор
	UPPER = (float*)malloc(N * sizeof(float));
	CENTRAL = (float*)malloc(N * sizeof(float));
	BOTTOM = (float*)malloc(N * sizeof(float));
	F = (float*)malloc(N * sizeof(float));
	X = (float*)malloc(N * sizeof(float));
	generateVector(N, F);
	generateMatrix(N, UPPER, BOTTOM, CENTRAL);

	// вывод входных данных
	if (SHOW) {
		cout << "\nMatrix:\n";
		printMatrix(N, UPPER, CENTRAL, BOTTOM);
		cout << "\nVector:\n";
		printVector(N, F);
	}

	// расчет решения
	t1 = high_resolution_clock::now();
	solveMatrix(N, UPPER, CENTRAL, BOTTOM, F, X);
	t2 = high_resolution_clock::now();
	duration<double> dur = duration_cast<duration<double>>(t2 - t1);

	// вывод результата
	if (SHOW) {
		cout << endl << "Result:\n";
		printVector(N, X);
	}

	// очистка памяти
	free(X);
	free(F);
	free(UPPER);
	free(CENTRAL);
	free(BOTTOM);
	return dur.count();
}

int main(int argv, char* argc[]) {
	int N, T;
	cout << "Matrix size: ";
	cin >> N;
	cout << "Thread number: ";
	cin >> T;

	float time = test(N, T);
	cout << "\nTime: " << time << endl;
}
