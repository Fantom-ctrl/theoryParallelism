# Sine Array 

## Описание

Программа:
- создаёт массив из 10^7 элементов (`float` или `double`)
- заполняет его значениями синуса
- используется ровно один период sin(x) на всю длину массива
- считает сумму всех элементов
- выводит результат в терминал

Тип массива выбирается **во время сборки**.

## Сборка

### Float (по умолчанию)

```bash
mkdir build
cd build
cmake ..
cmake --build .
./sine_array 
```
Выводит:
```bash
Array type: float
Sum: 0.349212
```

### Double

```bash
mkdir build
cd build
cmake -DUSE_DOUBLE=ON ..
cmake --build .
./sine_array
```
Выводит:
```bash
Array type: double
Sum: -6.76916e-10
```