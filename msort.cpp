#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

// Слияние двух отсортированных участков массива
void msort(long int start, long int step, int* mas2, long int end);

// Разделяем массив между потоками
// Каждый поток будет сортировать свой участок массива, после чего участки сольются
DWORD WINAPI thread_proc(void* param);

int K;// Колличество потоков
long int N;// Размер массива

int Len;// Длинна участка массива, сортеруемых каждым потокам
int* mas;// Сортеруемый массив

HANDLE* threads;// Массив потоков

int main()
{
    DWORD finished, started;

    // Чтение входных данных
    FILE* file = fopen("input.txt", "r");

    fscanf(file, "%d", &K);
    fscanf(file, "%d", &N);
    
    mas = (int*)calloc(N, sizeof(int));
    for (int i = 0; i < N; i++)
        fscanf(file, "%ld", &mas[i]);

    fclose(file);

    // Расчёт длинны участков массива, сортеруемых каждым потокам
    Len = N / K + N % K;

    // Создание потоков
    threads = (HANDLE*)calloc(K, sizeof(HANDLE));
    for (int i = 0; i < K; i++)
        threads[i] = CreateThread(0, 0, thread_proc, (void*)((char*)0 + i), 0, 0);

    started = GetTickCount();

    // Ожидание завершения всех потоков
    WaitForMultipleObjects(K, threads, TRUE, INFINITE);

    finished = GetTickCount();

    // Закрытие потоков
    for (int i = 0; i < K; i++)
        CloseHandle(threads[i]);

    // Вывод результатов
    FILE* output = fopen("output.txt", "w");
    FILE* time_f = fopen("time.txt", "w");

    fprintf(time_f, "%u", finished - started);
    fprintf(output, "%d\n%d\n", K, N);

    for (int i = 0; i < N; i++)
        fprintf(output, "%d ", mas[i]);

    fclose(output);
    fclose(time_f);

    free(mas);
}

// Разделяем массив между потоками
// Каждый поток будет сортировать свой участок массива, после чего участки сольются
DWORD WINAPI thread_proc(void* param) {
    int thread_number = (char*)param - (char*)0;// Номер потока
    long int Idx = Len * thread_number;// Левая граница участка массива выделенного потоку

    int step = 2;
    int* mas2 = (int*)calloc(N, sizeof(int));

    // Поток сортерует выделенный ему участок участок массива 
    while (step / 2 < Len)
    {
        memcpy(mas2, mas, N * sizeof(int));

        for (int j = Idx; j < Idx + Len && j < N; j += step)
            msort(j, step, mas2, Idx + Len);

        step *= 2;
    }

    /* Отсортированные участки будут сливаться. Четные потоки ожидают
    завершения нечётных, и выполняют слияние двух участков*/

    // Поток нечётный завершаем работу потока
    if (thread_number % 2 != 0)
        return 0;

    int thread_step = 1;// Шаг до ожидаемого потока (ждём завершения потока с номером thread_number + thread_step)
    while (thread_number + thread_step < K)// Ожидаемый поток не существует
    {
        WaitForSingleObject(threads[thread_number + thread_step], INFINITE);

        thread_step *= 2;

        memcpy(mas2, mas, N * sizeof(int));
        msort(Idx, Len * thread_step, mas2, Idx + Len * thread_step);
    }


    free(mas2);
    return 0;
}

// Слияние двух отсортированных участков массива
void msort(long int start, long int step, int* mas2, long int end) {
    long int lIndex = start;
    long int rIndex = start + step / 2;
    long int wIndex = start;

    // Граница участка выходит за границу массива
    if (end > N)
        end = N;

    while (lIndex < start + step / 2 && rIndex < start + step && rIndex < end) {
        if (mas2[lIndex] > mas2[rIndex]) {
            mas[wIndex] = mas2[rIndex];

            rIndex++;
            wIndex++;
        }
        else {
            mas[wIndex] = mas2[lIndex];

            lIndex++;
            wIndex++;
        }
    }

    while (lIndex < end && lIndex < start + step / 2) {
        mas[wIndex] = mas2[lIndex];

        lIndex++;
        wIndex++;
    }
    while (rIndex < end && rIndex < start + step) {
        mas[wIndex] = mas2[rIndex];

        rIndex++;
        wIndex++;
    }
}
