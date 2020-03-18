# compile the test file
g++ -std=c++11 test/demo1.cpp -o demo1
g++ -std=c++11 test/demo2.cpp -o demo2
g++ -std=c++11 test/demo3.cpp -o demo3
g++ -std=c++11 test/demo4.cpp -o demo4

# compile the main file
g++ -std=c++11 main.cpp -o main

# execute the main file
./main