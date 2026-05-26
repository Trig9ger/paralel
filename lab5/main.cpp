#include <iostream>
#include <fstream>
#include <random>
#include <string>
#include <cstdlib>
#include <chrono>
#include <mpi.h>
#include <sstream>

using namespace std;

vector<int> rand_matrix(int n) {
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<> dist(-5, 5);

	vector<int> matrix(n * n);
	generate(matrix.begin(), matrix.end(), [&]() { return dist(gen); });
	return matrix;
}

void save_matrix(const string& file_name, const vector<int>& matrix, int n) {
	ofstream out(file_name);
	if (out.is_open()) {
		for (int i = 0; i < n; ++i) {
			for (int j = 0; j < n; ++j) {
				out << matrix[i * n + j];
				if (j == n - 1) {
					out << '\n';
				}
				else if (j == n - 1 && i == n - 1) {
					continue;
				}
				else {
					out << ' ';
				}
			}
		}
		out.close();
	}
	else {
		cout << "Error!!!!!!!!!!! cant open " << file_name;
	}
}

vector<int> multiply_matrices_parallel(const vector<int>& A, const vector<int>& B, int m_size, int rank, int size, MPI_Comm comm) {
	int rows_per_process = m_size / size;
	vector<int> local_C(rows_per_process * m_size, 0);

	int start_row = rank * rows_per_process;
	int end_row = start_row + rows_per_process;

	for (int i = start_row; i < end_row; i++) {
		int local_i = i - start_row;
		for (int j = 0; j < m_size; j++) {
			int sum = 0;
			for (int k = 0; k < m_size; k++) {
				sum += A[i * m_size + k] * B[k * m_size + j];
			}
			local_C[local_i * m_size + j] = sum;
		}
	}

	vector<int> global_C;
	if (rank == 0) {
		global_C.resize(m_size * m_size);
	}

	MPI_Gather(
		local_C.data(),
		rows_per_process * m_size,
		MPI_INT,
		global_C.data(),
		rows_per_process * m_size,
		MPI_INT,
		0,
		comm
	);

	return global_C;
}

int main(int argc, char** argv) {
	MPI_Init(&argc, &argv);

	int num_procs;
	int rank;
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	int m_size = 0;
	vector<int> fir_matrix;
	vector<int> sec_matrix;
	vector<int> res_matrix;

	string file_matr1, file_matr2, file_res, file_data;
	int file_opened_successfully = 1;
	
	if (rank == 0) {
		ifstream in("globals.txt");
		if (in.is_open()) {
			vector<string> globals;
			string line;
			while (getline(in, line)) {
				if (!line.empty()) {
					globals.push_back(line);
				}
			}
			in.close();

			file_matr1 = globals[0];
			file_matr2 = globals[1];
			file_res = globals[2];
			file_data = globals[3];

			m_size = stoi(globals[4]);

			fir_matrix = rand_matrix(m_size);
			sec_matrix = rand_matrix(m_size);

			save_matrix(file_matr1, fir_matrix, m_size);
			save_matrix(file_matr2, sec_matrix, m_size);
		}
		else {
			cout << "Error!!!!! cant open globals.txt!" << endl;
			file_opened_successfully = 0;
		}
	}

	MPI_Bcast(&file_opened_successfully, 1, MPI_INT, 0, MPI_COMM_WORLD);
	if (!file_opened_successfully) {
		MPI_Finalize();
		return 1;
	}

	MPI_Bcast(&m_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

	if (rank != 0) {
		fir_matrix.resize(m_size * m_size);
		sec_matrix.resize(m_size * m_size);
	}

	MPI_Bcast(fir_matrix.data(), m_size * m_size, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(sec_matrix.data(), m_size * m_size, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);

	string data_text;

	int single_color = (rank == 0) ? 0 : 1;
	MPI_Comm single_comm;
	MPI_Comm_split(MPI_COMM_WORLD, single_color, rank, &single_comm);

	if (single_color == 0) {
		auto start = chrono::high_resolution_clock::now();

		res_matrix = multiply_matrices_parallel(fir_matrix, sec_matrix, m_size, 0, 1, single_comm);

		auto end = chrono::high_resolution_clock::now();
		chrono::duration<double> duration = end - start;

		stringstream ss;
		ss << "| " << m_size
			<< " | " << (1)
			<< " | " << (duration.count() * 1000000)
			<< " | " << duration.count() << " |\n";

		data_text += ss.str();

		save_matrix(file_res, res_matrix, m_size);
		system("py check.py");
	}
	MPI_Comm_free(&single_comm);

	vector<int> threads_c;
	int c = 2;
	while (c <= num_procs) {
		threads_c.push_back(c);
		c *= 2;
	}

	for (int threads : threads_c) {
		int color = (rank < threads) ? 0 : 1;
		MPI_Comm local_comm;
		MPI_Comm_split(MPI_COMM_WORLD, color, rank, &local_comm);

		if (color == 0) {
			int local_rank, local_size;
			MPI_Comm_rank(local_comm, &local_rank);
			MPI_Comm_size(local_comm, &local_size);

			MPI_Barrier(local_comm);
			auto start = chrono::high_resolution_clock::now();

			res_matrix = multiply_matrices_parallel(fir_matrix, sec_matrix, m_size, local_rank, local_size, local_comm);

			MPI_Barrier(local_comm);
			auto end = chrono::high_resolution_clock::now();
			chrono::duration<double> duration = end - start;

			if (local_rank == 0) {
				stringstream ss;
				ss << "| " << m_size
					<< " | " << (threads)
					<< " | " << (duration.count() * 1000000)
					<< " | " << duration.count() << " |\n";

				data_text += ss.str();
			}
		}
		MPI_Comm_free(&local_comm);
	}

	if (rank == 0) {
		ofstream out(file_data);
		out << data_text;
		out.close();
	}

	MPI_Finalize();
	return 0;
}
