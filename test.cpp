#include <iostream>
#include <cstdlib>
#include <inttypes.h>

#pragma pack(push, 1)
struct c_struct{
   int32_t v1_int32;
   int64_t v2_int64;
   float v3_float32;
   double v4_float64;
   char v5_char;
   char *v6_char_p;
   int *v7_ptr_int32;
   float *v8_ptr_float32;
   float v9_arr_float32[2];
};
#pragma pack(pop)

extern "C" int cfunc_ptr(c_struct *obj) {
   printf("c_struct:\n");
   printf("v1_int32 = %" PRId32 "\n", obj->v1_int32);
   printf("v2_int64 = %" PRId64 "\n", obj->v2_int64);
   printf("v3_float32 = %.15g\n", obj->v3_float32);
   printf("v4_float64 = %.15g\n", obj->v4_float64);
   printf("v5_char = %c\n", obj->v5_char);
   printf("v6_char_p = %s\n", obj->v6_char_p);

   printf("v7_ptr_int32 =");
   for(int x=0; x<10; ++x) printf(" %d", obj->v7_ptr_int32[x]);
   printf("\n");

   printf("v8_ptr_float32 =");
   for(int x=0; x<10; ++x) printf(" %f", obj->v8_ptr_float32[x]);
   printf("\n");

   printf("v9_arr_float32 = [%.6f, %.6f]\n", obj->v9_arr_float32[0], obj->v9_arr_float32[1]);
   return 1234;
}

extern "C" float cfunc_cst(c_struct obj) {
   cfunc_ptr(&obj);
   return 1.234f;
}

int main(int argc, const char *argv[]){
   char str[] = "Hello1\0Hello2";
   printf("Hello world\n");
   int vi[10];
   float v[10];
   for(int x=0; x<10; ++x){ v[x] = x+0.5f; vi[x] = x+1;}
   struct c_struct ca1 = {123456789, (int64_t)123456789123456789, 3.14159265358979, 3.14159265358979,
   	'Q', &str[0], &vi[0], &v[0], {1.2f, 3.4f}};
   printf("cfunc_ca1() returns %f\n\n", cfunc_cst(ca1));
   printf("cfunc_ptr() returns %d\n", cfunc_ptr(&ca1));
   return 0;
}
