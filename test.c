#include <stdio.h>
#include <stdint.h>

typedef uint32_t u32;
typedef int32_t i32;
typedef float f32;
typedef double f64;
typedef uint64_t u64;
typedef int64_t i64;

void begin() {
i32 a=2;
i32 b=3;
i32 c=4;
c=b=a;
}

int main(int argc, char *argv[]) {
begin();
	return 0;
}