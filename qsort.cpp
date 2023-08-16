#define _CRT_SECURE_NO_WARNINGS

#include <pthread.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <sys/times.h>
#include <ctime>
#include <errno.h>

// Разделяем массив между потоками
// Каждый поток будет сортировать свой участок массива быстрой сортировкой
void* separation(void* param);

// Быстрая сортировка
void qsort(int* mas_, long int size);

// Слияние отсортированных участков массива
void msort(long int start, long int Len, int* mas2, long int end);

long int Len;// Длинна участка массива сортеруемого на потоках
long int N;// Колличество элементов массива
int k;// Колличество потоков
int* mas;// Сортеруемыймассив
pthread_t* threads;// Массив потоков

int main()
{
    // Читаем данные
    FILE* file = fopen("input.txt", "r");

    fscanf(file, "%d", &k);
    fscanf(file, "%ld", &N);

    mas = (int*)calloc(N, sizeof(int));
    for (int i = 0; i < N; i++)
        fscanf(file, "%d", &mas[i]);
    
    fclose(file);

    // Расчёт длинны участков массива, сортеруемых каждым потокам
    if(k<=N)
    	Len = N / k + N%k/k;
    else
	    Len = 1;
    
    // Создание потоков
    threads = (pthread_t*)calloc(k, sizeof(pthread_t));
    for (int i = 0; i < k; i++)
        pthread_create(&threads[i], 0, separation, (void*)((char*)0 + i));
    
    time_t t = clock();

    // Ожидание нулового потока (нулевой поток завершится последним)
    pthread_join(threads[0], 0);

    t = clock() - t;

    // Файл с отсортированным массивом
    FILE* output = fopen("output.txt", "a");
    // Файл с временем работы программы
    FILE* time_f = fopen("time.txt", "a");

    // Запись результатов работы
    fprintf(time_f, "%d", t);
    fprintf(output, "%d\n%ld\n", k, N);

    for (int i = 0; i < N; i++)
        fprintf(output, "%d ", mas[i]);
    
    fclose(output);
    fclose(time_f);
}

// Разделяем массив между потоками
// Каждый поток будет сортировать свой участок массива быстрой сортировкой
void* separation(void* param) {
    // Номер потока
    int thread_number = (char*)param - (char*)0;

    // Индекс начала участка массива потока i
    long int Idx = Len * thread_number;

    // Выход за пределы массива
    if(Idx>=N)
        return 0;

    /* Если участок массива выходит за его приделы
    или это последний поток, поток получает
    участок от Idx, до конца массива*/ 
    if (Idx + Len <= N && thread_number<k-1)
        qsort(&mas[Idx], Len);
    else
        qsort(&mas[Idx], N - Idx);
    
    /* Отсортированные участки будут сливаться. Четные потоки ожидают 
    завершения нечётных, и выполняют слияние двух участков*/

    // Поток нечётный завершаем работу потока
    if(thread_number % 2 != 0)
	    return 0;
    
    int* mas2 = (int*)calloc(N, sizeof(int));// Копия сортеруемого массива
    int thread_step = 1;// Шаг до ожидаемого потока (ждём завершения потока с номером thread_number + thread_step)

    while (thread_number + thread_step < k)// Ожидаемый поток не существует
    {
        pthread_join(threads[thread_number + thread_step], 0);

        thread_step *= 2;

        memcpy(mas2, mas, N * sizeof(int));
        msort(Idx, Len * thread_step, mas2, Idx + Len * thread_step);
    }

    free(mas2);
    return 0;
}

// Быстрая сортировка
void qsort(int* mas_, long int size) {
    if(size<0)
	    return;
    
    int pivot = mas_[size / 2];
    long int rIndex = size - 1;
    long int lIndex = 0;
    int tmp;

    while (lIndex <= rIndex){
        while (lIndex < size && mas_[lIndex] < pivot)
            lIndex++;
        
        while (rIndex >= 0 && mas_[rIndex] > pivot)
            rIndex--;
        
        if (lIndex <= rIndex) {
            tmp = mas_[lIndex];

            mas_[lIndex] = mas_[rIndex];
            mas_[rIndex] = tmp;

            lIndex++;
            rIndex--;
        }
    }

    if (rIndex > 0)
        qsort(mas_, rIndex + 1);
    
    if (lIndex < size)
        qsort(&mas_[lIndex], size - lIndex);
    
    return;
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
