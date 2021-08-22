#include <stdio.h>
#include <stdint.h>


typedef uint32_t u32;
typedef int32_t i32;
typedef float f32;
typedef double f64;
typedef uint64_t u64;
typedef int64_t i64;

void __fib__(i32 depth,i32 current_depth) {
if(depth==current_depth){
return ;
}
}
void fib(i32 depth) {
__fib__(depth,0);
}
void begin() {
fib(10);
}

int main(int argc, char *argv[]) {
begin();
	return 0;
}