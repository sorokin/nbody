.PHONY: all stat nbody-gcc-stat nbody-clang-stat nbody-rust-stat nbody-gcc-perf nbody-clang-perf nbody-rust-perf

all: stat

nbody-gcc: main.cpp
	g++ -g -O3 -flto -ffast-math main.cpp -o nbody-gcc

nbody-clang: main.cpp
	clang++ -g -O3 -flto -ffast-math main.cpp -o nbody-clang

nbody-rust: main.rs
	rustc -C opt-level=3 -o nbody-rust main.rs

stat: nbody-gcc-stat nbody-clang-stat nbody-rust-stat

nbody-gcc-stat: nbody-gcc
	sudo chrt -f 99 perf stat ./nbody-gcc 50000000

nbody-clang-stat: nbody-clang
	sudo chrt -f 99 perf stat ./nbody-clang 50000000

nbody-rust-stat: nbody-rust
	sudo chrt -f 99 perf stat ./nbody-rust 50000000

nbody-gcc-perf:
	sudo chrt -f 99 perf record -o nbody-gcc.perf ./nbody-gcc 50000000

nbody-clang-perf:
	sudo chrt -f 99 perf record -o nbody-clang.perf ./nbody-clang 50000000

nbody-rust-perf:
	sudo chrt -f 99 perf record -o nbody-clang.perf ./nbody-clang 50000000
