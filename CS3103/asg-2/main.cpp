#include <iostream>
using namespace std;

template <class T>
struct Queue
{
    T arr[5];
    unsigned head;
    unsigned tail;
    unsigned size;
    unsigned capacity;

    Queue() : head(-1), tail(-1), size(0), capacity(5) {}
    Queue(unsigned cap): head(-1), tail(-1), size(0), capacity(cap) {}

    bool push(T data)
    {
        if (isFull())
            return false;

        if (isEmpty())
            head = 0;

        tail = (tail + 1) % capacity;
        arr[tail] = data;
        size++;

        return true;
    }

    bool pop()
    {
        if (isEmpty())
            return false;

        if (head == tail)
        {
            head = -1;
            tail = -1;
            return true;
        }

        head = (head + 1) % capacity;
        size--;

        return true;
    }

    T front()
    {
        if (size == 0)
            return nullptr;

        return arr[head];
    }

    bool isEmpty()
    {
        return size == 0;
    }

    bool isFull()
    {
        return size == capacity;
    }

    void print_queue()
    {
        cout << "Current Item in Queue: " << "size = " << size << endl;
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

void test()
{
    Queue<double *> q;

    double v1[] = {1.2, 1.3, 1.4, 1.5};
    double v2[] = {2.2, 2.3, 2.4, 2.5};
    double v3[] = {2.2, 3.3, 3.4, 3.5};
    double v4[] = {2.2, 3.3, 3.4, 3.5};
    double v5[] = {2.2, 3.3, 3.4, 3.5};
    double v6[] = {2.2, 3.3, 3.4, 3.5};

    cout << "v1: " << v1 << endl;
    cout << "v2: " << v2 << endl;
    cout << "v3: " << v3 << endl;
    cout << "v4: " << v4 << endl;

    cout << q.push(v1) << endl; // 1
    cout << q.push(v2) << endl; // 1
    cout << q.push(v3) << endl; // 1
    cout << q.push(v4) << endl; // 1
    cout << q.push(v5) << endl; // 1
    cout << q.push(v6) << endl; // 0
    cout << q.front() << endl; // v1
    cout << q.push(v1) << endl; // 0
    q.print_queue(); // 1 2 3 4 5 
    cout << q.pop() << endl; // 1
    q.print_queue(); // 2 3 4 5 
    cout << q.pop() << endl; // 1
    cout << q.pop() << endl; // 1
    cout << q.pop() << endl; // 1
    cout << q.pop() << endl; // 1
    cout << q.pop() << endl; // 0

}

int main(int argc, char *argv[])
{
    test();
    return 0;
}