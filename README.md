<h1>Multi-threaded implementation of FSS in using OpenMP</h1>

<h2> How to run sequential program </h2>

```
cd ./src
make all
./seq_fss [-n NUM FISHES] [-r NUM ROUNDS] [-v DISPLAY GUI] [-g GUI GRID SIZE] [-f FITNESS FUNCTION]
```

<h2> How to run parallel program </h2>

```
cd ./src/
make all
./parallel_fss [-t NUM THREADS] [-n NUM FISHES] [-r NUM ROUNDS] [-s SCHEDULE] [-c CHUNK SIZE] [-v DISPLAY GUI] [-g GUI GRID SIZE] [-f FITNESS FUNCTION]
```