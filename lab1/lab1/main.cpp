#include <iostream>
#include <fstream>
#include <random>
#include <string>
#include <cstdlib>
#include <chrono>

using namespace std;

void save_rand_matcrix(int n, string file_name) {
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<> dist(-5, 5);

	ofstream out;
	out.open(file_name);

	if (out.is_open()) {
		for (int i = 0; i < n; ++i) {
			for (int j = 0; j < n; ++j) {
				out << dist(gen);
				if (j == n-1) {
					out << '\n';
				}
				else if (j == n-1 && i == n - 1) {
					continue;
				}
				else { 
					out << ' '; 
				}
			};
		};
		out.close();

		cout << file_name << " is created." << '\n';
	}
	else {
		throw invalid_argument("Can't open file " + file_name);
	};
};

void save_matrix(string file_name, vector<vector<int>> matrix) {
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<> dist(0, 10);

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
		out.close();

		cout << file_name << " is created." << '\n';
	}
	else {
		throw invalid_argument("Can't open file " + file_name);
	};
}

vector<vector<int>> read_matrix(string file_name) {
	ifstream in;
	in.open(file_name);


	if (in.is_open()) {
		string line;
		vector<vector<int>> matrix;
		string number;

		while (getline(in, line)) {
			vector<int> row;
			for (char c : line) {
				if (isdigit(c) || c == '-') {
					number += c;
				}
				else if (!number.empty()) {
					row.push_back(stoi(number));
					number.clear();
				};
			}; 
			if (!number.empty()) {
				row.push_back(stoi(number));
				number.clear();
			};
			matrix.push_back(row);
		};
		in.close();

		for (size_t i = 0; i < matrix.size(); ++i) {
			if (matrix[i].size() != matrix.size()) {
				throw length_error("Size of matrix is incorect in file " + file_name);
			};
		};
		
		return matrix;
	}
	else {
		throw invalid_argument("Can't open file " + file_name);
	};
};

vector<vector<int>> mult_matrix(vector<vector<int>> fir_matrix, vector<vector<int>> sec_matrix) {
	if (fir_matrix.size() != sec_matrix.size()) {
		throw length_error("Matrixes size is defferent");
	}; 

	size_t n = fir_matrix.size();
	vector<vector<int>> matrix(n, vector<int>(n, 0));

	auto start = chrono::high_resolution_clock::now();
	for (size_t i = 0; i < n; ++i) {
		for (size_t k = 0; k < n; ++k) {
			double temp = fir_matrix[i][k];
			for (size_t j = 0; j < n; ++j) {
				matrix[i][j] += temp * sec_matrix[k][j];
			}
		}
	}
	auto end = chrono::high_resolution_clock::now();

	chrono::duration<double> duration = end - start;

	cout << duration.count() << '\n';

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

		save_rand_matcrix(m_size, globals[0]);
		save_rand_matcrix(m_size, globals[1]);

		vector<vector<int>> fir_matrix = read_matrix(globals[0]);
		vector<vector<int>> sec_matrix = read_matrix(globals[1]);

		auto start = chrono::high_resolution_clock::now();

		vector<vector<int>> res_matrix = mult_matrix(fir_matrix, sec_matrix);

		auto end = std::chrono::high_resolution_clock::now();

		save_matrix(globals[2], res_matrix);

		system("py check.py");

		chrono::duration<double> duration = end - start;

		ofstream out;
		out.open(globals[3]);

		out << "Time of work: " << duration.count() << '\n';
		out << "Size of matrix: " << res_matrix.size();

		out.close();

		cout <<"| " << m_size << " | " << m_size * m_size << " | " << duration.count() * 1000000 << " | " << duration.count() << " |";

	} catch (invalid_argument e) {
		cerr << e.what();
	} catch (length_error e) {
		cerr << e.what();
	};
};