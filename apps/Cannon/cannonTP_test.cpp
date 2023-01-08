#include <iostream>
#include "cannonTP.h"
#include<omp.h>
#include <random>

#define innerloop 10
#define outerloop 1


// Command line arguments 
// ./CannonTP_test numSU numCU Matsize sqrtNumTiles SqrtNumElementPerTile

int main(int argc, char * argv[]) {

    double innerTime = 0;
    double outerTime = 0;

    //double startTime=0, endTime=0;

    if (argc != 5)
    {
        std::cout << "Enter number of numTP numCD MatsSize sqrtNumTiles" << std::endl;
        return 0;
    }
    
    int tps = atoi(argv[1]);                // numSU
    int cds = atoi(argv[2]);                // numCU
    int msize = atoi(argv[3]);              // Size of rows/columsn sqaure matrix 
    int sqrtNumTiles = atoi(argv[4]);       // sqrtNumTiles
    int sqrtP; //= atoi(argv[5]);              // SqrtNumElementPerTile

    if(msize % sqrtNumTiles != 0)
    {
        std::cout<<"Error wrong numner of tiles!";
            return 0;
    }
    
    sqrtP = msize / sqrtNumTiles;

    matrix A(msize,msize); 
    matrix B(msize,msize); 
    matrix C(msize,msize);
	//matrix CC(msize,msize);

	
    double *AA,*BB,*CC;
	int m,n,k;
	m=n=k=msize;;
	AA = (double *)malloc( m*k*sizeof( double ));
    BB = (double *)malloc( k*n*sizeof( double ));
    CC = (double *)malloc( m*n*sizeof( double ));
	
	std::random_device rd;
	std::default_random_engine generator(rd()); // rd() provides a random seed
	std::uniform_real_distribution<double> distribution(0.1,10);	
    
	double n1,n2;
	for (int i = 0; i < msize*msize ; i++) {
        n1 = distribution(generator);
		n2 = distribution(generator);
		//std::cout<< n1 << " " << n2 <<"\n";
		A.getPtr()[i] = n1;
        B.getPtr()[i] = n2;
		AA[i] = n1;
		BB[i] = n2;
        CC[i] = 0;
    }

	
	double alpha, beta;
	alpha = 1.0 ;
	beta = 0.0;
	//cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
	DGEMM( m, n, k, 1, AA, k, BB, n, 1, CC, n); 

    //std::cout<<"A \n";
    // A.printMatrix();
    //std::cout<<"\n";
    //std::cout<<"B \n";
    //A.printMatrix();
    //std::cout<<"C \n";
    //C.printMatrix();
	
    //    for (int i = 0; i < msize*msize ; i++) {
	//		std::cout<<AA[i]<<" ";
	//		std::cout<<*(A.getPtr()+i)<<" ";
	//		std::cout<<"\t";
	//		std::cout<<BB[i]<<" ";
	//		std::cout<<*(B.getPtr()+i)<<" ";
	//		std::cout<<"\n";
	//	}

    // Add all the crap for DARTS
    ThreadAffinity affin(cds,tps, COMPACT_NO_SMT, 3, 3);
    if (affin.generateMask())
    {
        Runtime * rt = new Runtime(&affin);
        //std::cout<<"No TPs : "<<affin.getNumTPS()<<"\n";
        //std::cout<<"No CDS : "<<affin.getNumMCS()<<"\n";
        //affin.printMask();

        for(int i=0;i<outerloop;i++){
            C.resetMatrix();
            rt->run(launch<cannonTP>(&A,&B,&C,sqrtNumTiles,sqrtP,&Runtime::finalSignal));
            
            for(int j=0;j<innerloop;j++)
            {                
                C.resetMatrix();
                double startTime = omp_get_wtime();
                //cannonTP(Matrix A, Matrix B, Matrix C, sqrt NumTiles, ElementsPerTile, final signal)
                rt->run(launch<cannonTP>(&A,&B,&C,sqrtNumTiles,sqrtP,&Runtime::finalSignal));
                double endTime = omp_get_wtime();

                innerTime+=endTime-startTime;
           }
        outerTime+=innerTime/innerloop;
        innerTime = 0;
        //std::cout<<"Result \n";
        //C.printMatrix();

        }


        bool res = true;
        for (int i = 0; i < msize*msize ; i++)
            if (std::abs(CC[i] - C.getPtr()[i]) > 0.0001) {
                //std::cout << "Error in : " << i << "\t" << CC[i] << "\t" << C.getPtr()[i] << std::endl;
                res = false;
                break;
            }

        std::cout << "Check is: " << (res? "SUCCESS": "FAIL") ;
		std::cout<<"\t";

        std::cout<< outerTime/outerloop << std::endl;
   
       delete rt; 
    }
    else
    {
        std::cout<<"Failed to generate mask! \n\n";
    }
    
    return 0;
}
