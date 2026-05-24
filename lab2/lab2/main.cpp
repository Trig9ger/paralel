#include <iostream>
#include <fstream>
#include <random>
#include <string>
#include <cstdlib>
#include <chrono>

using namespace std;

vector<vector<int>> rand_matrix(int n, string file_name) {
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<> dist(-5, 5);

	vector<vector<int>> matrix(n, vector<int>(n));

	for (int i = 0; i < n; ++i) {
		generate(matrix[i].begin(), matrix[i].end(), [&]() { return dist(gen); });
	};
	return matrix;
};

void save_matrix(string file_name, vector<vector<int>> matrix) {
	size_t n = matrix.size();

	ofstream out;
	out.open(file_name);

	if (out.is_open()) {
		for (int i = 0; i < n; ++i) {
			for (int j = 0; j < n; ++j) {
				out << matrix[i][j];
				if (j == n - 1) {
					out << '\n';
				}
				else if (j == n - 1 && i == n - 1) {
					continue;
				}
				else {
					out << ' ';
				}
			};
		};
		out.close();;
	}
	else {
		throw invalid_argument("Can't open file " + file_name);
	};
}

vector<vector<int>> mult_matrix(vector<vector<int>> fir_matrix, vector<vector<int>> sec_matrix, int threads) {
	size_t n = fir_matrix.size();
	vector<vector<int>> matrix(n, vector<int>(n, 0));

#pragma omp parallel for num_threads(threads) shared(fir_matrix, sec_matrix, matrix, n)
	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < n; ++j) {
			double temp = 0;
			for (int k = 0; k < n; ++k) {
				temp += fir_matrix[i][k] * sec_matrix[k][j];
			}
			matrix[i][j] = temp;
		}
	}

	return matrix;
};

int main() {
	try {
		vector<string> globals;
		ifstream in;
		in.open("globals.txt");
		if (in.is_open()) {
			string line;
			while (getline(in, line)) {
				globals.push_back(line);
			}
		}
		in.close();

		int m_size = stoi(globals[4]);
		vector<int> vec_threads = { 2, 4, 8};


		vector<vector<int>> fir_matrix = rand_matrix(m_size, globals[0]);
		vector<vector<int>> sec_matrix = rand_matrix(m_size, globals[1]);

		save_matrix(globals[0], fir_matrix);
		save_matrix(globals[1], sec_matrix);

		auto start = chrono::high_resolution_clock::now();

		vector<vector<int>> res_matrix = mult_matrix(fir_matrix, sec_matrix, 1);

		auto end = std::chrono::high_resolution_clock::now();
		chrono::duration<double> duration = end - start;

		save_matrix(globals[2], res_matrix);

		system("py check.py");

		ofstream out;
		out.open(globals[3]); 

		out << "| " << m_size << " | " << 1 << " | " << duration.count() * 1000000 << " | " << duration.count() << " | \n";

		for (int threads : vec_threads) {
			cout << threads;
			auto start = chrono::high_resolution_clock::now();

			mult_matrix(fir_matrix, sec_matrix, threads);

			auto end = std::chrono::high_resolution_clock::now();
			chrono::duration<double> duration = end - start;

			out << "| " << m_size << " | " << threads << " | " << duration.count() * 1000000 << " | " << duration.count() << " | \n";

		}

		out.close();

	}
	catch (invalid_argument e) {
		cerr << e.what();
	}
	catch (length_error e) {
		cerr << e.what();
	};
};