#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

using namespace std;
#define M 4
#define N 2

#define NUM_QUANTIZER 2
double *generate_frame_vector(int l);

struct ConcurrentQueue
{
    double *arr[5];

    unsigned head;
    unsigned tail;
    unsigned size;
    unsigned capacity;
    pthread_mutex_t mtx;

    ConcurrentQueue() : head(-1), tail(-1), size(0), capacity(5) {}
   
    void push(double *data)
    {
        while (is_full())
            ;

        int s = pthread_mutex_lock(&mtx);
        if (is_empty())
            head = 0;

        for (int i = 0; i < M*N; i++)
        {
            cout << data[i] << " ";
        }

        cout << endl;
        tail = (tail + 1) % capacity;
        arr[tail] = data;
        size++;
        s = pthread_mutex_unlock(&mtx);
    }

    void pop()
    {
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

    void print_queue()
    {
        cout << "Current Item in Queue: "
             << "size = " << size << endl;
        cout << "-------------------------" << endl;

        for (int i = 0, j = head; i < size; i++)
        {
            cout << arr[j] << " ";
            j = (j + 1) % size;
        }
        cout << endl
             << "-------------------------" << endl;
    }
};

ConcurrentQueue cache;
static pthread_mutex_t cnt_mtx = PTHREAD_MUTEX_INITIALIZER; // for locking cnt when the quantizer update the cnt

void print_frame_vector(double *vec, int l)
{
    for (int i = 0; i < l; i++)
    {

        cout << vec[i] << "     ";
    }
    cout << endl;
}

void test_gen_vector()
{
    int l = M * N;
    double *v;
    do
    {
        v = generate_frame_vector(l);
        if (v)
            print_frame_vector(v, l);
    } while (v);
}

void *camera(void *args)
{
    double *v;
    int l = M * N;
    int *interval = (int *)args;
    do
    {
        if (!cache.is_full())
        {
            v = generate_frame_vector(l);

            if (v)
            {
                cache.push(v);
            }
        }

        sleep(*interval);
    } while (v);
}

void *quantizer(void *arg)
{
    double *t;
    int cnt = 0;
    do
    {
        if(!cache.is_empty()) {
            t = cache.front();
            cache.pop();
            int l = M * N;
            for (int i = 0; i < l; i++)
            {
                t[i] = (t[i] > 0.5) ? 1 : 0;
            }

            cnt = 0;

            for (int i = 0; i < l; i++)
            {
                cout << t[i] << " ";
            }
            
            cout << endl;
            
            sleep(3);
        }else {
            cnt++;
            sleep(1);
        }
    } while (cnt < 3);
}

int main(int argc, char *argv[])
{

    if(argc == 2) {
        int interval = atoi(argv[1]);

        int c_err, j_err;

        pthread_t camera_thread;
        pthread_t quantizer_threads[NUM_QUANTIZER];

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