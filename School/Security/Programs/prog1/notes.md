Prerequisites:
- gcc
```
gcc --version
```
- make
```
make --version
```

Install on Mac:
```bash
xcode-select --install
```

Compile program:
navigate to current folder of this file and run following command
```bash
make
```

Compile test files:
In same directory run following commands
```bash
gcc -o test1 test1.c
gcc -o test2 test2.c
gcc -o test3 test3.c
```

Run program:
```bash
./canrun test1 test2 test3
```
