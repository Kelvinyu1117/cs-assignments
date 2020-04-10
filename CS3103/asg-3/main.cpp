#include <iostream>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <iomanip>
#include <semaphore.h>
using namespace std;

#define M 4
#define N 2

double *generate_frame_vector(int l);

struct ConcurrentQueue
{
    double *arr[5];
    int head; // points to the first element of the queue
    int tail; // points to the last element of the queue
    int capacity;
    int size;
    sem_t num_of_items;
    sem_t num_of_empty_space;
    pthread_mutex_t mtx;
    pthread_mutex_t data_mtx[5];

    ConcurrentQueue()
    {
        head = -1;
        tail = -1;
        size = 0;
        capacity = 5;

        sem_init(&num_of_items, 0, 0);
        sem_init(&num_of_empty_space, 0, 5);
    }

    void push(double *data)
    {
        // precondition: the queue is not full

        int tmp = (tail + 1) % capacity;

        pthread_mutex_lock(&data_mtx[tmp]);
        arr[tmp] = data;
        tail = tmp;
        pthread_mutex_unlock(&data_mtx[tmp]);

        pthread_mutex_lock(&mtx);
        if (size == 0)
            head = 0;
        size++;
        pthread_mutex_unlock(&mtx);
    }

    void pop()
    {
        // precondition: the queue is not empty
        if (head == tail)
        {
            head = tail = -1;
        }
        else
        {
            int tmp = (head + 1) % capacity;
            pthread_mutex_lock(&data_mtx[tmp]);
            head = tmp;
            pthread_mutex_unlock(&data_mtx[tmp]);

            pthread_mutex_lock(&mtx);
            size--;
            pthread_mutex_unlock(&mtx);
        }
    }

    bool isEmpty()
    {
        pthread_mutex_lock(&mtx);
        bool res = size == 0;
        pthread_mutex_unlock(&mtx);

        return res;
    }

    bool isFull()
    {
        pthread_mutex_lock(&mtx);
        bool res = size == capacity;
        pthread_mutex_unlock(&mtx);

        return res;
    }

    double *front()
    {
        double *res;
        pthread_mutex_lock(&data_mtx[head]);
        if(size == 0)
            res = nullptr;
        else
            res = arr[head];
        pthread_mutex_unlock(&data_mtx[head]);

        return res;
    }
};

struct Record
{
    double orginal[M * N];
    double compressed_frame[M * N];

    void compression(double *data)
    {
        int L = M * N;

        for (int i = 0; i < L; i++)
        {
            cout << data[i] << " ";
        }
        cout << endl;

        for (int i = 0; i < L; i++)
        {
            orginal[i] = data[i];
            compressed_frame[i] = round(data[i] * 10);
            cout << compressed_frame[i] << " ";
        }

        cout << endl;
    }

    double cal_MSE()
    {
        double mse = 0.0;

        int L = M * N;

        for (int i = 0; i < L; i++)
        {
            mse += pow(orginal[i] - compressed_frame[i] / 10, 2);
        }

        mse /= L;

        return mse;
    }
};

ConcurrentQueue cache;
Record temp_recorder;
sem_t notify_start_transform, notify_start_estimate, notify_finish_estimate;
pthread_mutex_t recorder_mtx;

int produced_frames = 0;
int processed_frames = 0;
bool camera_finished = false;

void *camera(void *args)
{
    cout << "Camera thread starts" << endl;
    double *v;
    int l = M * N;
    int *interval = (int *)args;
    do
    {
        if (cache.isFull())
            sem_wait(&notify_finish_estimate);

        v = generate_frame_vector(l);

        if (v) // if v is not nullptr
        {
            cache.push(v);
            produced_frames++;
            sleep(*interval);
            sem_post(&notify_start_transform);
        }
        else
        {
            camera_finished = true;
        }

    } while (v);

    camera_finished = true;

    cout << "Camera thread ends" << endl;
}

void *transformer(void *arg)
{
    while (!camera_finished || camera_finished && processed_frames < produced_frames)
    {
        sem_wait(&notify_start_transform);
        sem_wait(&notify_finish_estimate);

        double *data = cache.front();
        if(data) {
            pthread_mutex_lock(&recorder_mtx);
            temp_recorder.compression(data);
            sleep(3);
            pthread_mutex_unlock(&recorder_mtx);

            sem_post(&notify_start_estimate);
        }
    }
}

void *estimator(void *arg)
{
    while (!camera_finished || camera_finished && processed_frames < produced_frames)
    {
        sem_wait(&notify_start_estimate);

        pthread_mutex_lock(&recorder_mtx);
        double mse = temp_recorder.cal_MSE();
        cout << "mse = " << mse << endl;
        processed_frames++;
        pthread_mutex_unlock(&recorder_mtx);

        cache.pop();

        sem_post(&notify_finish_estimate);
        sem_post(&notify_start_transform);
    }
}


void threads_creation(pthread_t &camera_thread, pthread_t &transformer_thread, pthread_t &estimator_thread, int interval) {
    int err;

    // create the thread
    err = pthread_create(&camera_thread, NULL, camera, (void *)&interval);
    if (err)
    {
        cout << "Error when creating camera thread!" << endl;
        exit(-1);
    }
    err = pthread_create(&transformer_thread, NULL, transformer, NULL);
    if (err)
    {
        cout << "Error when creating transformer thread!" << endl;
        exit(-1);
    }

    err = pthread_create(&estimator_thread, NULL, estimator, NULL);
    if (err)
    {
        cout << "Error when creating estimator thread!" << endl;
        exit(-1);
    }
}


void threads_joining(pthread_t &camera_thread, pthread_t &transformer_thread, pthread_t &estimator_thread) {
    // join the threads
    int err = pthread_join(camera_thread, NULL);
    if (err)
    {
        cout << "Error when joining camera thread!" << endl;
        exit(-1);
    }

    err = pthread_join(transformer_thread, NULL);
    if (err)
    {
        cout << "Error when joining transformer thread!" << endl;
        exit(-1);
    }

    err = pthread_join(estimator_thread, NULL);
    if (err)
    {
        cout << "Error when joining estimator thread!" << endl;
        exit(-1);
    }
}

int main(int argc, char *argv[])
{

    sem_init(&notify_start_transform, 0, 0);
    sem_init(&notify_start_estimate, 0, 0);
    sem_init(&notify_finish_estimate, 0, 1);

    pthread_t camera_thread, transformer_thread, estimator_thread;
    int interval = 3;
    
    threads_creation(camera_thread, transformer_thread, estimator_thread, interval);
    threads_joining(camera_thread, transformer_thread, estimator_thread);

    

    return 0;
}