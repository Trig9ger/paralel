import numpy as np

def read_matrix(file_name: str)->list[list[int]]:
    matrix = []

    text = []

    with open(file_name, 'r', encoding='utf-8') as file:
        text = file.readlines()

    n = len(text)

    for i in text:
        row = []
        number = 0
        neg = 0
        for j in i:
            if j.isdigit():
                number += number*10 + int(j)
            elif j == '-':
                neg = 1
            else:
                row.append(number*(-1)**neg)
                number = 0
                neg = 0
        matrix.append(row)

    return matrix

def main():
    fir_matrix = read_matrix("matr1.txt")
    sec_matrix = read_matrix("matr2.txt")

    first = np.array(fir_matrix)
    second = np.array(sec_matrix)

    result = first @ second

    np.savetxt("true_matr_result.txt", result, delimiter=' ', fmt='%d')

if __name__ == "__main__":
    main()