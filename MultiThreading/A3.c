#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_CHARS 2048 // max chars when reading file each line
#define SPLIT_SEP ","  // split string

/* structure for passing data to threads */
typedef struct
{
    int starting_index; // array starting index
    int ending_index;   // array ending index
} parameters;

void *sorter(void *params);
void *merger(void *params);
void quick_sort(int left, int right);

// for read file
int *global_array;
// for the last merger step
int *sorted_array;
// storage length of global array
int length_of_array;

// read a file named `IntegerList.txt`
// then put it`s items in global var
void read_file(const char *file_name)
{
    FILE *fin;
    // open file only read used by fopen
    fin = fopen(file_name, "r");
    if (!fin)
    {
        perror("open file fail");
        return;
    }
    char *buf = (char *)malloc(sizeof(char) * MAX_CHARS);
    fgets(buf, MAX_CHARS, fin); // get a line
    char *token;
    token = strtok(buf, SPLIT_SEP);
    global_array = (int *)malloc(sizeof(int) * 500);
    if (!global_array)
    {
        perror("malloc error");
        return;
    }
    while (token != NULL)
    {
        // string to int val
        int num = atoi(token);
        global_array[length_of_array] = num;
        length_of_array++;
        // next
        token = strtok(NULL, SPLIT_SEP);
    }
    fclose(fin);
}
// write sorted array in file
void write_file(const char *file_name)
{
    FILE *fout;
    fout = fopen(file_name, "w");
    if (!fout)
    {
        perror("created file fail");
        return;
    }
    for (int i = 0; i < length_of_array - 1; i++)
    {
        fprintf(fout, "%d,", sorted_array[i]);
    }
    fprintf(fout, "%d\n", sorted_array[length_of_array - 1]);
    fclose(fout);
}
int main(int argc, const char *argv[])
{
    length_of_array = 0; // init global var

    read_file("IntegerList.txt");

    int mid = length_of_array / 2;
    // constructing params
    parameters *p1 = (parameters *)malloc(sizeof(parameters));
    parameters *p2 = (parameters *)malloc(sizeof(parameters));
    p1->starting_index = 0, p1->ending_index = mid;
    p2->starting_index = mid, p2->ending_index = length_of_array;

    pthread_t th1, th2;
    // created the first sorting thread
    if (pthread_create(&th1, NULL, sorter, (void *)p1) != 0)
    {
        perror("the first sorting creating fail");
        exit(1);
    }
    // created the second sorting thread
    if (pthread_create(&th2, NULL, sorter, (void *)p2) != 0)
    {
        perror("the second sorting creating fail");
        exit(1);
    }
    void *retval;
    // waitting thread1 joined
    int joined1 = pthread_join(th1, &retval);
    if (joined1 != 0)
    {
        perror("cannot join with thread1\n");
        exit(1);
    }
    // for (int i = p1->starting_index; i < p1->ending_index; i++)
    // {
    //     printf("%d ", global_array[i]);
    // }
    // thread1 joined ...
    int joined2 = pthread_join(th2, &retval);
    if (joined2 != 0)
    {
        perror("cannot join with thread2\n");
        exit(1);
    }
    // thread2 joined ...
    // for (int i = p2->starting_index; i < p2->ending_index; i++)
    // {
    //     printf("%d ", global_array[i]);
    // }
    // created the merging thread
    pthread_t th3;
    if (pthread_create(&th3, NULL, merger, (void *)p1) != 0)
    {
        perror("cannot join with thread2\n");
        exit(1);
    }
    int joined3 = pthread_join(th3, &retval);
    if (joined3 != 0)
    {
        perror("cannot join with thread3\n");
        exit(1);
    }
    // thread 3 joined ...

    write_file("SortedIntegerList.txt");

    // in the last, free these gloabl vars memeory
    if (global_array)
    {
        free(global_array);
    }
    if (sorted_array)
    {
        free(sorted_array);
    }
    free(p1), free(p2);
}

//Using quick sort in this case
void *sorter(void *params)
{
    // params may be NULL, then skip this condition
    if (!params)
    {
        return NULL;
    }
    // type of params must be parameters pointer
    parameters *p = (parameters *)(params);
    quick_sort(p->starting_index, p->ending_index - 1);
    return NULL;
}

void quick_sort(int left, int right)
{
    int first = left;
    int last = right;
    int key = global_array[first];
    if (left >= right)
        return;
    while (first < last)
    {
        while (first < last && global_array[last] > key)
        {
            last--;
        }
        global_array[first] = global_array[last];

        while (first < last && global_array[first] < key)
        {
            first++;
        }
        global_array[last] = global_array[first];
    }
    global_array[first] = key;

    quick_sort(left, first - 1);
    quick_sort(first + 1, right);
}

void *merger(void *params)
{
    // the merger global func, used by two pointer algorithm

    // params may be NULL, then skip this condition
    if (!params)
    {
        return NULL;
    }
    // type of params must be parameters pointer
    parameters *p = (parameters *)(params);
    sorted_array = (int *)malloc(sizeof(int) * (length_of_array + 1));
    if (!sorted_array)
    {
        perror("malloc error");
        return NULL;
    }
    int left_index = p->starting_index, righ_index = p->ending_index;
    int index = 0;
    while (left_index < p->ending_index && righ_index < length_of_array)
    {
        if (global_array[left_index] <= global_array[righ_index])
        {
            sorted_array[index] = global_array[left_index];
            index++, left_index++;
        }
        else
        {
            sorted_array[index] = global_array[righ_index];
            index++, righ_index++;
        }
    }
    // but the right pointer was not ending, then contiune
    while (righ_index < length_of_array)
    {
        sorted_array[index] = global_array[righ_index];
        index++, righ_index++;
    }
    // but the left pointer was not ending, then contiune
    while (left_index < p->ending_index)
    {
        sorted_array[index] = global_array[left_index];
        index++, left_index++;
    }
    return NULL;
}
