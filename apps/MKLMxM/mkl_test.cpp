#include <iostream>
#include<omp.h>
#include "mkl.h"

#define innerloop 10
#define outerloop 1


int main(int argc, char * argv[]) {

    double innerTime = 0;
    double outerTime = 0;

    double startTime=0, endTime=0;

    if (argc != 3)
    {
        std::cout << "Enter MatrixSize " << std::endl;
        return 0;
    }

    //int tps = atoi(argv[1]);                // numSU
    int cds = atoi(argv[1]);                // numCU
    int msize = atoi(argv[2]);              // MatSize : number of elements per row or Col in Matrix.
    //int sqrtNumTiles = atoi(argv[4]);       // sqrtNumTiles
    //int sqrtP = atoi(argv[5]);              // SqrtNumElementPerTile

    //std::cout<<"MAX NumThreads: "<<mkl_get_max_threads()<<std::endl;
    mkl_set_num_threads_local(cds);

    double *A,*B,*C;
    int m,n,k;
    double alpha, beta;
    alpha = 1.0; beta = 0.0;
    m=n=k=msize;
    A = (double *)mkl_malloc( m*k*sizeof( double ), 64 );
    B = (double *)mkl_malloc( k*n*sizeof( double ), 64 );
    C = (double *)mkl_malloc( m*n*sizeof( double ), 64 );
   
    for(int i=0;i<outerloop;i++){
        //C.resetMatrix();
        // call test mkl
        //rt->run(launch<cannonTP>(&A,&B,&C,sqrtNumTiles,sqrtP,&Runtime::finalSignal));

        for(int j=0;j<innerloop;j++){
            //C.resetMatrix();
            double startTime = omp_get_wtime();

            // #define DGEMM(i, j, k, alpha, aPtr, aSize, bPtr, bSize, beta, cPtr, cSize) 
            // cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, i, j, k, alpha, aPtr, aSize, bPtr, bSize, beta, cPtr, cSize)
            //DGEMM(msize,msize,msize,1,A,msize,B,msize,1,C,msize);
            cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,m, n, k, alpha, A, k, B, n, beta, C, n);
            
            double endTime = omp_get_wtime();

            innerTime+=endTime-startTime;
        }
    outerTime+=innerTime/innerloop;
    innerTime = 0;

    }
    std::cout<< outerTime/outerloop << std::endl;
    return 0;
}
