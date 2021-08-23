#include <stdio.h>
#include <stdint.h>


typedef uint32_t u32;
typedef int32_t i32;
typedef float f32;
typedef double f64;
typedef uint64_t u64;
typedef int64_t i64;

i32 fib(i32 depth) {
if(depth<=1){
return depth;
}
return fib(depth-1)+fib(depth-2);
}
void begin() {
fib(10);
char c='a';
char d='a';
if(c==d){
c=d='b';
}
}

int main(int argc, char *argv[]) {
begin();
	return 0;
}