#include <iostream>
#include <fstream>
#include <random>
#include <string>
#include <cuda_runtime.h>

using namespace std;

#define CUDA_CHECK(call)                                             \
    do {                                                             \
        cudaError_t error = call;                                    \
        if (error != cudaSuccess) {                                  \
            std::cerr << "CUDA error at " << __FILE__ << ":" << __LINE__ << " - " \
                      << cudaGetErrorString(error) << std::endl;     \
            exit(EXIT_FAILURE);                                      \
        }                                                            \
    } while (0)

__global__ void matrixMulSimple(const int* A, const int* B, int* C, int n) {
	int row = blockIdx.y * blockDim.y + threadIdx.y;
	int col = blockIdx.x * blockDim.x + threadIdx.x;

	if (row < n && col < n) {
		int sum = 0;
		for (int k = 0; k < n; k++) {
			sum += A[row * n + k] * B[k * n + col];
		}
		C[row * n + col] = sum;
	}
}


void read_globals(string file_name, vector<string>& globals) {
	ifstream in(file_name);
	if (in.is_open()) {
		string line;
		while (getline(in, line)) {
			globals.push_back(line);
		}
	}
	else {
		cout << "Error!!!! cant open " << file_name;
	}
}

void save_matrix(string file_name, int* matrix, int N) {
	ofstream out(file_name);
	if (out.is_open()) {
		for (int i = 0; i < N; ++i) {
			for (int j = 0; j < N; ++j) {
				out << matrix[i * N + j];
				if (j == N - 1) {
					out << '\n';
				}
				else if (j == N - 1 && i == N - 1) {
					continue;
				}
				else {
					out << ' ';
				}
			}
		}
	}
	else {
		cout << "Error!!!!!!!!1 cant open " << file_name;
	}
}

int main() {
	vector<string> globals;
	read_globals("globals.txt", globals);

	int N = stoi(globals[4]);

	int* fir_mat = new int[N * N];
	int* sec_mat = new int[N * N];
	int* res_mat = new int[N * N];

	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<> dist(-5, 5);

	for (int i = 0; i < N * N; ++i) {
		fir_mat[i] = dist(gen);
		sec_mat[i] = dist(gen);
	};

	save_matrix(globals[0], fir_mat, N);
	save_matrix(globals[1], sec_mat, N);

	int cuda_size = N * N * sizeof(int);

	int* d_A, * d_B, * d_C;

	CUDA_CHECK(cudaMalloc(&d_A, cuda_size));
	CUDA_CHECK(cudaMalloc(&d_B, cuda_size));
	CUDA_CHECK(cudaMalloc(&d_C, cuda_size));

	CUDA_CHECK(cudaMemcpy(d_A, fir_mat, cuda_size, cudaMemcpyHostToDevice));
	CUDA_CHECK(cudaMemcpy(d_B, sec_mat, cuda_size, cudaMemcpyHostToDevice));


	vector<int> block_sizes = { 8, 16, 32 };

	cout << "| " << N;

	for (int block_len : block_sizes) {
		dim3 blockSize(block_len, block_len);
		dim3 gridSize((N + block_len - 1) / block_len,
			(N + block_len - 1) / block_len);

		cudaEvent_t start, stop;
		CUDA_CHECK(cudaEventCreate(&start));
		CUDA_CHECK(cudaEventCreate(&stop));

		cudaDeviceSynchronize();

		CUDA_CHECK(cudaEventRecord(start));

		matrixMulSimple << <gridSize, blockSize >> > (d_A, d_B, d_C, N);
		cudaDeviceSynchronize();

		CUDA_CHECK(cudaEventRecord(stop));
		CUDA_CHECK(cudaEventSynchronize(stop));

		float time_ms = 0;
		CUDA_CHECK(cudaEventElapsedTime(&time_ms, start, stop));

		CUDA_CHECK(cudaEventDestroy(start));
		CUDA_CHECK(cudaEventDestroy(stop));

		cout << "| " << N << " | " << block_len << " | " << time_ms*1000 << " | " << time_ms/1000 << " |\n";
	}

	CUDA_CHECK(cudaMemcpy(res_mat, d_C, cuda_size, cudaMemcpyDeviceToHost));
	save_matrix(globals[2], res_mat, N);

	cudaFree(d_A);
	cudaFree(d_B);
	cudaFree(d_C);

	save_matrix(globals[2], res_mat, N);

	system("py check.py");

	delete[] fir_mat;
	delete[] sec_mat;
	delete[] res_mat;
}