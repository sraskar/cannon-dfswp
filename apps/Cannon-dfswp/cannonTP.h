#ifndef __CANNON_TP_HEADER__
#define __CANNON_TP_HEADER__
#include <vector>
#include <algorithm>
#include "darts.h"

#include "thread_safe_deque.h" // DARTS internal 
#include "readerwriterqueue.h"  // moodycamel
#include "atomicops.h"  // moodycamel
#include "matrix.h"

using namespace darts;


// CLASSES DECLARATION
// This class allows us to have the C tile
// that contains the result of C, and the input 
// for the computation all together. 
// In Cannons this represents the tile, A and B will be 
// shifted
class tile_mem {
    private:
        int tileP;
        int sqrtP;
        double * mem;
        // TODO: This are scratchpad memory to avoid 
        // doing mallocs on the codelets. It should use the 
        // tail of the fifo queue instead
        matrix tileA_scratch;
        matrix tileB_scratch;

        // Local copy of C
        matrix tileC;

        // Queue of work for A and B
        //thread_safe::deque<matrix> AQueue;
        //thread_safe::deque<matrix> BQueue;

        moodycamel::BlockingReaderWriterQueue<matrix> AQueue;
        moodycamel::BlockingReaderWriterQueue<matrix> BQueue;
        
    public:
        tile_mem(int sqrt_tileP):
            tileP(sqrt_tileP * sqrt_tileP),
            sqrtP(sqrt_tileP),
            mem(new double[tileP*3]),
            tileA_scratch       (sqrt_tileP, sqrt_tileP, mem),
            tileB_scratch (sqrt_tileP, sqrt_tileP, mem + tileP),
            tileC       (sqrt_tileP, sqrt_tileP, mem + 2*tileP)
            { 
                std::fill_n(mem, tileP*3, 0);
            }

        ~tile_mem() {
            delete [] mem;
        }

        // TODO: These are to avoid doing malloc inside the codelet. 
        // Instead we have a scratch area that we use in the copy 
        // codelet and then do the initial push into the corresponding
        // queues
            matrix * getTileA_scratch() { return &this->tileA_scratch; }
            matrix * getTileB_scratch() { return &this->tileB_scratch; }

        void pushA(matrix * toPush) {
            // This should create a copy inside the container
            //this->AQueue.push_back(*toPush);
            //myPC->q.enqueue(i);
            
            this->AQueue.enqueue(*toPush);
        }

        void pushB(matrix * toPush) {
            // This should create a copy inside the container
            //this->BQueue.push_back(*toPush);
            this->BQueue.enqueue(*toPush);
        }

        // We want to read the memory from the head of the queueu, and not create a copy in the
        // codelet or somewhere else. So we want to read the head, use it, and then pop it in two separate
        // moments. Since it is single consumer this should be allowed 
            matrix * getCurrentHeadA() {
                // if (this->AQueue.size() != 0){
                //     return &this->AQueue.front();
                // }
                // else {
                //     return NULL;
                // }

                if(this->AQueue.size_approx() != 0){
                    return this->AQueue.peek();
                }
                else {
                    return NULL;
                }
                
            }

            void popCurrentHeadA() { 
                //this->AQueue.pop_front(); 
                this->AQueue.pop();
            }

            matrix * getCurrentHeadB() {
                // if (this->BQueue.size() != 0)
                //     return &this->BQueue.front();
                // else 
                //     return NULL;

                if(this->BQueue.size_approx() != 0){
                    return this->BQueue.peek();
                }
                else {
                    return NULL;
                }
            }

            void popCurrentHeadB() { 
                //this->BQueue.pop_front(); 
                this->BQueue.pop();
            }



        

        matrix& getTileC() { return tileC; }

        // void swap_write_read() {
        //     double * tmpPtr = tileA.getPtr();
        //     tileA.setPtr(tileA_write.getPtr());
        //     tileA_write.setPtr(tmpPtr);
        //     tmpPtr = tileB.getPtr();
        //     tileB.setPtr(tileB_write.getPtr());
        //     tileB_write.setPtr(tmpPtr);
        // }

        int getTileP() { return tileP; }
        int getsqrtP() { return sqrtP; }
};

class cannonSkewBarrier: public Codelet 
{
    public:
        cannonSkewBarrier(uint32_t dep, uint32_t reset, ThreadedProcedure * myTP,  uint32_t stat):
        Codelet(dep,reset,myTP, stat, "cannonSkewBarrier") {}

        virtual void fire(void);
};

class cannonCodCopyA : public Codelet 
{
    private:
        int tileIndex;

    public:
        cannonCodCopyA(uint32_t dep, uint32_t reset, ThreadedProcedure * myTP,  uint32_t stat, int indx):
        Codelet(dep,reset,myTP, stat, "cannonCodCopyA" + std::to_string(indx)), tileIndex(indx) { this->setSchedHint(indx); }

        virtual void fire(void);
};

class cannonCodCopyB : public Codelet 
{
    private:
        int tileIndex;

    public:
        cannonCodCopyB(uint32_t dep, uint32_t reset, ThreadedProcedure * myTP,  uint32_t stat, int indx):
        Codelet(dep,reset,myTP, stat, "cannonCodCopyB" + std::to_string(indx)), tileIndex(indx) {this->setSchedHint(indx);}

        virtual void fire(void);
};

class cannonCodCopyC : public Codelet 
{
    private:
        int tileIndex;

    public:
        cannonCodCopyC(uint32_t dep, uint32_t reset, ThreadedProcedure * myTP,  uint32_t stat, int indx):
        Codelet(dep,reset,myTP, stat, "cannonCodCopyC" + std::to_string(indx)), tileIndex(indx) {this->setSchedHint(indx);}

        virtual void fire(void);
};

class cannonCodCompute : public Codelet 
{
    private:
        int tileIndex;
        int nextRow;
        int nextCol;
        int numIterations;
        int curIterations;

    public:
        cannonCodCompute(uint32_t dep, uint32_t reset, ThreadedProcedure * myTP,  uint32_t stat, int indx, int nextR, int nextC, int numIts):
        Codelet(dep,reset,myTP, stat, "cannonCodCompute" + std::to_string(indx)), tileIndex(indx), nextRow(nextR), nextCol(nextC), numIterations(numIts), curIterations(0) {
        this->setSchedHint(indx);}

        virtual void fire(void);
};

class cannonSkewCodelet : public Codelet 
{
    private:
        int tileIndex;
        int destRow;
        int destCol;

    public:
        cannonSkewCodelet(uint32_t dep, uint32_t reset, ThreadedProcedure * myTP,  uint32_t stat, int tileIndex, int destR, int destC) :
        Codelet(dep,reset,myTP, stat, "cannonSkewCodelet" + std::to_string(tileIndex)), tileIndex(tileIndex), destRow(destR), destCol(destC) {this->setSchedHint(tileIndex);}

        virtual void fire(void);
};

class cannonFinalCodelet : public Codelet 
{
    public:
        cannonFinalCodelet(uint32_t dep, uint32_t reset, ThreadedProcedure * myTP,  uint32_t stat):
        Codelet(dep,reset,myTP, stat, "cannonFinalCodelet"){}

        virtual void fire(void);
};

class cannonTP : public ThreadedProcedure
{
    public:        

        // MATRIX INFORMATION
        int sqrtNumTiles; // square root of number of tiles matrix will be divided. 
        int numTiles;     // sqrt of number of elements each time will have. 
        matrix *A,*B,*C;
        std::vector<tile_mem*> allMatrices;

        // CODELETS
        std::vector<cannonSkewCodelet*> allSkewCodelets;
        std::vector<cannonCodCompute*> allComputeCodelets;
        std::vector<cannonCodCopyA*> allCopyACod;
        std::vector<cannonCodCopyB*> allCopyBCod;
        std::vector<cannonCodCopyC*> allCopyCCod;
	cannonSkewBarrier skewBarrier;
        cannonFinalCodelet endCodelet;
        Codelet * continuationCod;

        /// CONSTRUCTOR 
        cannonTP(matrix* A, matrix* B, matrix* C, int sqrtNumTiles, int sqrtP, Codelet* toSig):
            ThreadedProcedure(),
            sqrtNumTiles(sqrtNumTiles),
            numTiles(sqrtNumTiles*sqrtNumTiles),
            A(A),
            B(B),
            C(C),
            skewBarrier(numTiles, numTiles, this, SHORTWAIT),
            endCodelet(numTiles, numTiles, this, SHORTWAIT),
            continuationCod(toSig)
            {
                // Creation of the tiles
                for (int i = 0; i < numTiles; ++i) {
                    allMatrices.push_back(new tile_mem(sqrtP));
                    // no dependancies, all copy codelets can start asap
                    allCopyACod.push_back(new cannonCodCopyA(0, 0, this, SHORTWAIT, i));
                    allCopyBCod.push_back(new cannonCodCopyB(0, 0, this, SHORTWAIT, i));
                    
                    // Precompute indices for Skew phase. 
                    int theI = i % sqrtNumTiles;
                    int theJ = i / sqrtNumTiles;

                    int destI = (theI - theJ + sqrtNumTiles)%sqrtNumTiles;
                    int destJ = (theJ - theI + sqrtNumTiles)%sqrtNumTiles;

                    // For computational shifting
                    // Shifting Right
                    int nextI = (theI+1)%sqrtNumTiles;
                    // Shifting Left
                    int nextJ = (theJ+1)%sqrtNumTiles;

                    // Getting the indexes of the next tiles
                    int nextRow = theJ*sqrtNumTiles + nextI;
                    int nextCol = nextJ*sqrtNumTiles + theI;

                    int destRow = theJ*sqrtNumTiles + destI;
                    int destCol = destJ*sqrtNumTiles + theI;
                    
                    // 2 depnendacies from Copy A & Copy B.
                    allSkewCodelets.push_back(new cannonSkewCodelet(2, 2, this, SHORTWAIT, i, destRow, destCol));
                    // 1 dependancy from loop codelet. 
                    allComputeCodelets.push_back(new cannonCodCompute(1, 1, this, SHORTWAIT, i, nextRow, nextCol, sqrtNumTiles));
                    
                    // 1 dependancy from the loop codelet when all iterations are done. 
                    allCopyCCod.push_back(new cannonCodCopyC(1, 1, this, SHORTWAIT, i));
				}
                for (int i = 0; i < numTiles; ++i) {
                    add(allCopyACod[i]);
                    add(allCopyBCod[i]);
                }
            }

        inline std::vector<tile_mem*> * getAllMatrices() { return &allMatrices; };

        ~cannonTP() {
            for (int i = 0; i < numTiles; ++i) {  
                delete allMatrices[i];
                delete allCopyACod[i];
                delete allCopyBCod[i];
                delete allComputeCodelets[i];
            }
        }


};

void
cannonSkewBarrier::fire() {
    //std::cout<<"SKEW BARRIER \n";
    cannonTP * myCannonTP = static_cast<cannonTP*> (this->myTP_);
    // Creation of the tiles
    for (auto it =  myCannonTP->allComputeCodelets.begin(); it < myCannonTP->allComputeCodelets.end(); it++)
	(*it)->decDep();
}
void
cannonCodCopyA::fire() {
    //std::cout<<"CopyA of " << this->tileIndex<<"\n";
    cannonTP * myCannonTP = static_cast<cannonTP*> (this->myTP_);
    tile_mem * myTileMemory = (*myCannonTP->getAllMatrices())[this->tileIndex];
    matrix * A = myCannonTP->A;
    int sqrtNT = myCannonTP->sqrtNumTiles;

    // Get the offset of the element according to the tile ID
    int initial_index = (this->tileIndex/sqrtNT)*A->getM()*myTileMemory->getsqrtP() + (this->tileIndex % sqrtNT)*myTileMemory->getsqrtP();

    // Helper pointers
    double * origin;
    double * dest = myTileMemory->getTileA_scratch()->getPtr();

    // Making the actual copy
    for (int i = initial_index; i < (initial_index + myTileMemory->getsqrtP()*A->getM()); i+=A->getM()) {
        origin = A->getPtr()+i;
        std::copy_n(origin, myTileMemory->getsqrtP(), dest);
        dest += myTileMemory->getsqrtP();
    }

    // Signal the compute codelet
    myCannonTP->allSkewCodelets[this->tileIndex]->decDep();
}

void
cannonCodCopyB::fire() {
    //std::cout<<"CopyB of " << this->tileIndex<<"\n";
    cannonTP * myCannonTP = static_cast<cannonTP*> (this->myTP_);

    tile_mem * myTileMemory = (*myCannonTP->getAllMatrices())[this->tileIndex];
    matrix * B = myCannonTP->B;
    int sqrtNT = myCannonTP->sqrtNumTiles;

    int initial_index = (this->tileIndex/sqrtNT)*B->getM()*myTileMemory->getsqrtP() + (this->tileIndex % sqrtNT)*myTileMemory->getsqrtP();

    double * origin;
    double * dest = myTileMemory->getTileB_scratch()->getPtr();

    for (int i = initial_index; i < (initial_index + myTileMemory->getsqrtP()*B->getM()); i+=B->getM()) {
        origin = B->getPtr()+i;
        std::copy_n(origin, myTileMemory->getsqrtP(), dest);
        dest += myTileMemory->getsqrtP();
    }

    // Signal the compute codelet
    myCannonTP->allSkewCodelets[this->tileIndex]->decDep();
}

void
cannonCodCopyC::fire() {
    //std::cout<<"CopyC of " << this->tileIndex<<"\n";
    cannonTP * myCannonTP = static_cast<cannonTP*> (this->myTP_);

    tile_mem * myTileMemory = (*myCannonTP->getAllMatrices())[this->tileIndex];
    matrix * C = myCannonTP->C;
    int sqrtNT = myCannonTP->sqrtNumTiles;

    int initial_index = (this->tileIndex/sqrtNT)*C->getM()*myTileMemory->getsqrtP() + (this->tileIndex % sqrtNT)*myTileMemory->getsqrtP();

    double * origin = myTileMemory->getTileC().getPtr();
    double * dest;

    for (int i = initial_index; i < (initial_index + myTileMemory->getsqrtP()*C->getM()); i+=C->getM()) {
        dest = C->getPtr()+i;
        std::copy_n( origin, myTileMemory->getsqrtP(), dest);
        origin += myTileMemory->getsqrtP();
    }

    // Signal the final codelet
    // TODO when cannon is implemented this is the condition of the number of shifsts that have to happen
    bool done = true; 

    if(done)
        myCannonTP->endCodelet.decDep();
}

void
cannonCodCompute::fire() {
    //std::cout<<"Compute of " << this->tileIndex <<"\n";
    // Get ready for the next iteration
    this->resetCodelet();

    cannonTP * myCannonTP = static_cast<cannonTP*> (this->myTP_);
    std::vector<tile_mem*> * matrices = myCannonTP->getAllMatrices();

    while (this->curIterations < this->numIterations) {
        matrix * A = NULL;
        matrix * B = NULL;

        // Waiting for work to do 
        while (A == NULL || B == NULL) {
            A = (*matrices)[this->tileIndex]->getCurrentHeadA();
            B = (*matrices)[this->tileIndex]->getCurrentHeadB();
        }
        double * C = (*matrices)[this->tileIndex]->getTileC().getPtr(); 

        int sideTile = (*matrices)[this->tileIndex]->getsqrtP();
        int wholeTile = (*matrices)[this->tileIndex]->getTileP();

        // call DGEMM
        DGEMM(
            sideTile,
            sideTile,
            sideTile,
            1,
            A->getPtr(),
            sideTile,
            B->getPtr(),
            sideTile,
            1,
            C,
            sideTile);


        // Shift A right and B down
        (*matrices)[this->nextRow]->pushA(A);
        (*matrices)[this->nextCol]->pushB(B);

        (*matrices)[this->tileIndex]->popCurrentHeadA();
        (*matrices)[this->tileIndex]->popCurrentHeadB();
        this->curIterations++;
    }
    
    myCannonTP->allCopyCCod[this->tileIndex]->decDep();
}

void
cannonSkewCodelet::fire() {
    //std::cout<<"Swap of " << this->tileIndex <<"\n";

    cannonTP * myCannonTP = static_cast<cannonTP*> (this->myTP_);

    std::vector<tile_mem*> * matrices = myCannonTP->getAllMatrices();
    matrix * A = (*matrices)[this->tileIndex]->getTileA_scratch();
    matrix * B = (*matrices)[this->tileIndex]->getTileB_scratch();  

    int sideTile = (*matrices)[this->tileIndex]->getsqrtP();
    int wholeTile = (*matrices)[this->tileIndex]->getTileP();

    // Shift A right and B down
    //std::cout << "Hi I am " << this->tileIndex << " Writting A to " << this->destRow << " and B to " << this->destCol << "\n";
    (*matrices)[this->destRow]->pushA(A);
    (*matrices)[this->destCol]->pushB(B);
        
    //myCannonTP->allComputeCodelets[tileIndex]->decDep();
    myCannonTP->skewBarrier.decDep();
}

void 
cannonFinalCodelet::fire() {

    cannonTP * myCannonTP = static_cast<cannonTP*> (this->myTP_);

    myCannonTP->continuationCod->decDep();
}
#endif
