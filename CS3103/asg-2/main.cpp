#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <iomanip>

using namespace std;
#define M 4
#define N 2
#define NUM_QUANTIZER 2

double *generate_frame_vector(int l);

struct ConcurrentQueue
{
    double *arr[5]; // circular array with double* type
    int head; // points to the first element of the queue
    int tail; // points to the last element of the queue
    int size; // number of current item
    int capacity; // actual physical size of the queue
    pthread_mutex_t mtx;

    ConcurrentQueue() : head(-1), tail(-1), size(0), capacity(5) {}

    // push the data onto the queue, thread-safe operation
    void push(double *data)
    {
        if (is_full())
            return;

        int s = pthread_mutex_lock(&mtx);

        int l = M * N;
        
        if (is_empty())
            head = 0;

        tail = (tail + 1) % capacity;
        arr[tail] = data;
        size++;
        s = pthread_mutex_unlock(&mtx);
    }

    // pop the data out of the queue, thread-safe operation
    void pop()
    {
        if (is_empty())
            return;

        int s = pthread_mutex_lock(&mtx);

        if (head == tail)
        {
            head = -1;
            tail = -1;
        }
        else
        {

            head = (head + 1) % capacity;
        }
        size--;
        s = pthread_mutex_unlock(&mtx);
    }

    // get the first item from the queue
    double *front()
    {
        if (size == 0)
            return nullptr;

        return arr[head];
    }

    bool is_empty()
    {
        return size == 0;
    }

    bool is_full()
    {
        return size == capacity;
    }
};

ConcurrentQueue cache;

void *camera(void *args)
{
    cout << "Camera thread starts" << endl;
    double *v;
    int l = M * N;
    int *interval = (int *)args;
    do
    {
        if (!cache.is_full()) // if the cache is not full
        {
            v = generate_frame_vector(l);

            if (v) // if v is not nullptr
            {
                cache.push(v);
            }
        }

        sleep(*interval);
    } while (v);
    cout << "Camera thread ends" << endl;
}

void *quantizer(void *arg)
{
    double *t;
    int cnt = 0;
    do
    {
        if (!cache.is_empty())
        {
            t = cache.front();
            cache.pop();

            int l = M * N;

            // quantization
            for (int i = 0; i < l; i++)
            {
                t[i] = (t[i] > 0.5) ? 1 : 0;
            }

            // print out the result
            cout << fixed << setprecision(1);
            for (int i = 0; i < l; i++)
            {
                cout << t[i] << " ";
            }
            cout << endl;
            
            cnt = 0;
            sleep(3);
        }
        else
        {
            cnt++;
            sleep(1);
        }
    } while (cnt < 3);
}

int main(int argc, char *argv[])
{

    if (argc == 2)
    {
        int interval = atoi(argv[1]);

        int c_err, j_err;
        
        pthread_t camera_thread;
        pthread_t quantizer_threads[NUM_QUANTIZER];

        // thread creation
        c_err = pthread_create(&camera_thread, NULL, camera, (void *)&interval);
        if (c_err)
        {
            cout << "Error when creating camera thread!" << endl;
            exit(-1);
        }

        for (int i = 0; i < NUM_QUANTIZER; i++)
        {
            c_err = pthread_create(&quantizer_threads[i], NULL, quantizer, NULL);
            if (c_err)
            {
                cout << "Error when creating quantizer thread" << i + 1 << "!" << endl;
                exit(-1);
            }
        }
        // wait for the thread finish
        j_err = pthread_join(camera_thread, NULL);
        if (j_err)
        {
            cout << "Error when joining camera thread!" << endl;
            exit(-1);
        }

        for (int i = 0; i < NUM_QUANTIZER; i++)
        {
            j_err = pthread_join(quantizer_threads[i], NULL);
            if (j_err)
            {
                cout << "Error when joining quantizer thread" << i + 1 << "!" << endl;
                exit(-1);
            }
        }
    }

    return 0;
}