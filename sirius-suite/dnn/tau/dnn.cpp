/*******************************************************************************
*   Copyright(C) 2012 Intel Corporation. All Rights Reserved.
*   
*   The source code, information  and  material ("Material") contained herein is
*   owned  by Intel Corporation or its suppliers or licensors, and title to such
*   Material remains  with Intel Corporation  or its suppliers or licensors. The
*   Material  contains proprietary information  of  Intel or  its  suppliers and
*   licensors. The  Material is protected by worldwide copyright laws and treaty
*   provisions. No  part  of  the  Material  may  be  used,  copied, reproduced,
*   modified, published, uploaded, posted, transmitted, distributed or disclosed
*   in any way  without Intel's  prior  express written  permission. No  license
*   under  any patent, copyright  or  other intellectual property rights  in the
*   Material  is  granted  to  or  conferred  upon  you,  either  expressly,  by
*   implication, inducement,  estoppel or  otherwise.  Any  license  under  such
*   intellectual  property  rights must  be express  and  approved  by  Intel in
*   writing.
*   
*   *Third Party trademarks are the property of their respective owners.
*   
*   Unless otherwise  agreed  by Intel  in writing, you may not remove  or alter
*   this  notice or  any other notice embedded  in Materials by Intel or Intel's
*   suppliers or licensors in any way.
*
********************************************************************************/

/*******************************************************************************
*   This example computes real matrix C=alpha*A*B+beta*C using Intel(R) MKL 
*   function dgemm, where A, B, and C are matrices and alpha and beta are 
*   scalars in double precision. 
*
*   In this simple example, practices such as memory management, data alignment, 
*   and I/O that are necessary for good programming style and high MKL 
*   performance are omitted to improve readability.
********************************************************************************/


/*
Form  C := alpha*A*B + beta*C.
 DO 90, J = 1, N
    DO 60, I = 1, M
        C( I, J ) = BETA*C( I, J )
    DO 80, L = 1, K
        TEMP = ALPHA*B( L, J )
        DO 70, I = 1, M
            C( I, J ) = C( I, J ) + TEMP*A( I, L )
       END IF
    CONTINUE
 CONTINUE
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "simt.h"

#define LOOP 1
#define M 1000
#define P 1000
#define N 1000

int main()
{
    /* Timing */
	struct timeval tv1, tv2;
    unsigned int totalruntimeseq = 0;
    unsigned int totalruntimetau = 0;

    double *A, *B, *C, *C_TAU;
    double alpha, beta;

    printf ("\n This example computes real matrix C=alpha*A*B+beta*C\n"
            " where A, B, and  C are matrices and \n"
            " alpha and beta are double precision scalars\n\n");

    printf (" Looping: %i\n"
            " Initializing data for matrix multiplication C=A*B for matrix \n"
            " A(%ix%i) and matrix B(%ix%i)\n\n", LOOP, M, P, P, N);

    alpha = 1.0; beta = 1.0;

    A = (double *)malloc( M*P*sizeof( double ) );
    B = (double *)malloc( P*N*sizeof( double ) );
    C = (double *)malloc( M*N*sizeof( double ) );
    C_TAU = (double *)malloc( M*N*sizeof( double ) );

    if (A == NULL || B == NULL || C == NULL) {
      free(A);
      free(B);
      free(C);
      return 1;
    }

    /* Fill loops */
    for (int i = 0; i < (M*P); i++) {
        A[i] = (double)(i+1);
    }

    for (int i = 0; i < (P*N); i++) {
        B[i] = (double)(-i-1);
    }

    for (int i = 0; i < (M*N); i++) {
        C[i] = 0.0;
        C_TAU[i] = 0.0;
    }

    /* Hot loop */
    gettimeofday(&tv1,NULL);
    // for (int l = 0; l<N*M; ++l)
    // {
    //    int i = l/N;
    //    int j = l%N;
    //    float temp = beta*C_TAU[i*N+j];
    //    for (int r = 0; r < P; ++r) {
    //        C_TAU[i*N+j] += alpha*B[r*N+j]*A[i*P+r];
    //    }
    //     C_TAU[i*N+j] += temp;
    // }

    // for (int j = 0; j<N; ++j)
    // {
    //     for (k = 0; k < M; ++k) {
    //         C[k*N+j] = beta*C[k*N+j];
    //     }
    //     for (l = 0; l < P; ++l) {
    //         temp = alpha*B[l*N+j];
    //         for (i = 0; i < M; ++i) {
    //             C[i*N+j] = C[i*N+j] + temp*A[i*P+l];
    //         }
    //     }
    // }
    gettimeofday(&tv2,NULL);
    totalruntimeseq = (tv2.tv_sec-tv1.tv_sec)*1000000 + (tv2.tv_usec-tv1.tv_usec);

    gettimeofday(&tv1,NULL);
    // version 1
    // ar::simt_tau::par_for(N, [&](size_t j)
    // {
    //     for (int k = 0; k < M; ++k) {
    //         C_TAU[k*N+j] = beta*C_TAU[k*N+j];
    //     }
    //     for (int l = 0; l < P; ++l) {
    //         float temp = alpha*B[l*N+j];
    //         for (int i = 0; i < M; ++i) {
    //             C_TAU[i*N+j] = C_TAU[i*N+j] + temp*A[i*P+l];
    //         }
    //     }
    // });
    // version 2
    ar::simt_tau::par_for(N*M, [&](size_t l)
    {
       int i = l/N;
       int j = l%N;
       float temp = beta*C_TAU[i*N+j];
       for (int r = 0; r < P; ++r) {
           C_TAU[i*N+j] += alpha*B[r*N+j]*A[i*P+r];
       }
        C_TAU[i*N+j] += temp;
    });

    // version 3
    // ar::simt_tau::par_for(N*M, [&](size_t j)
    // {
    //     int k = j / M;
    //     int l = j / P;
    //     int i = j % M;
    //     float temp = beta*C_TAU[k*N+j];
    //     float temp1= alpha*B[l*N+j];
    //     C_TAU[j] = temp + temp1*A[i*P+l];
    // });

    gettimeofday(&tv2,NULL);
    totalruntimetau = (tv2.tv_sec-tv1.tv_sec)*1000000 + (tv2.tv_usec-tv1.tv_usec);

    // check correctness
    // for(int i = 0; i < N; ++i){
    //     for(int k = 0; k < M; ++k){
    //         if(C[k*N+i] != C_TAU[k*N+i]){
    //             printf("Error: C[%d]: %f CTAU[%d]: %f\n",
    //                     k*N+i,C[k*N+i],k*N+i,C_TAU[k*N+i]);
    //             break;
    //         }
    //     }
    // }

    // Timing
    printf("Seq time: %.2f ms\n", (double)totalruntimeseq/1000);
    printf("TAU time: %.2f ms\n", (double)totalruntimetau/1000);
    printf("Speedup: %.2f \n", (double)totalruntimeseq/(double)totalruntimetau);

    free(A);
    free(B);
    free(C);
    free(C_TAU);

    return 0;
}