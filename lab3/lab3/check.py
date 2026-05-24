import numpy as np

def read_matrix(file_name: str)->list[list[int]]:
    matrix = []

    with open(file_name, 'r', encoding='utf-8') as file:
        text = file.readlines()

    n = len(text)

    for i in text:
        row = []
        number = 0
        neg = 0
        for j in i:
            if j.isdigit():
                number = number*10 + int(j)
            elif j == '-':
                neg = 1
            else:
                row.append(number*(-1)**neg)
                number = 0
                neg = 0
        matrix.append(row)

    return matrix

def main():
    with open("globals.txt", "r", encoding='utf-8') as file:
        glob = file.readlines()

    fir_matrix = read_matrix(glob[0].removesuffix('\n'))
    sec_matrix = read_matrix(glob[1].removesuffix('\n'))
    cpp_matrix = read_matrix(glob[2].removesuffix('\n'))

    first = np.array(fir_matrix)
    second = np.array(sec_matrix)
    cpp_res = np.array(cpp_matrix)

    py_res = first @ second
    
    np.savetxt(glob[5], py_res, delimiter=' ', fmt='%d')

    if( np.array_equal(cpp_res, py_res) ):
        print("С++ матрица равна numpy матрице")
    else:
        print("Матрицы не равны")

if __name__ == "__main__":
    main()