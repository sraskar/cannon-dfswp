#ifndef __CANNON_TP_HEADER__
#define __CANNON_TP_HEADER__
#include <vector>
#include <algorithm>
#include "darts.h"
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
        matrix tileA;
        matrix tileA_write;
        matrix tileB;
        matrix tileB_write;
        matrix tileC;
        
    public:
        tile_mem(int sqrt_tileP):
            tileP(sqrt_tileP * sqrt_tileP),
            sqrtP(sqrt_tileP),
            mem(new double[tileP*5]),
            tileA       (sqrt_tileP, sqrt_tileP, mem),
            tileA_write (sqrt_tileP, sqrt_tileP, mem + tileP),
            tileB       (sqrt_tileP, sqrt_tileP, mem + 2*tileP),
            tileB_write (sqrt_tileP, sqrt_tileP, mem + 3*tileP),
            tileC       (sqrt_tileP, sqrt_tileP, mem + 4*tileP)
            { 
                std::fill_n(mem, tileP*5, 0);
            }

        ~tile_mem() {
            delete [] mem;
        }

        // Getter for tile matrices 
        matrix& getTileA() { return tileA; }
        matrix& getTileA_write() { return tileA_write; }
        matrix& getTileB() { return tileB; }
        matrix& getTileB_write() { return tileB_write; }
        matrix& getTileC() { return tileC; }

        void swap_write_read() {
            double * tmpPtr = tileA.getPtr();
            tileA.setPtr(tileA_write.getPtr());
            tileA_write.setPtr(tmpPtr);
            tmpPtr = tileB.getPtr();
            tileB.setPtr(tileB_write.getPtr());
            tileB_write.setPtr(tmpPtr);
        }

        int getTileP() { return tileP; }
        int getsqrtP() { return sqrtP; }

        void setTileA(double val)
        {
            for (size_t i = 0; i < tileP; i++)
            {
                tileA.getPtr()[i] = val;
            }
            
        }

        void setTileB(double val)
        {
            
            for (size_t i = 0; i < tileP; i++)
            {
                tileB.getPtr()[i] = val;
            }
            
        }
};

class cannonCodCopyA : public Codelet 
{
    private:
        int tileIndex;

    public:
        cannonCodCopyA(uint32_t dep, uint32_t reset, ThreadedProcedure * myTP,  uint32_t stat, int indx):
        Codelet(dep,reset,myTP, stat, "cannonCopyA" + std::to_string(indx)), tileIndex(indx) {}

        virtual void fire(void);
};

class cannonCodCopyB : public Codelet 
{
    private:
        int tileIndex;

    public:
        cannonCodCopyB(uint32_t dep, uint32_t reset, ThreadedProcedure * myTP,  uint32_t stat, int indx):
        Codelet(dep,reset,myTP, stat, "cannonCodCopyB" + std::to_string(indx) ), tileIndex(indx) {}

        virtual void fire(void);
};

class cannonCodCopyC : public Codelet 
{
    private:
        int tileIndex;

    public:
        cannonCodCopyC(uint32_t dep, uint32_t reset, ThreadedProcedure * myTP,  uint32_t stat, int indx):
        Codelet(dep,reset,myTP, stat, "cannonCodCopyC" + std::to_string(indx)), tileIndex(indx) {}

        virtual void fire(void);
};

class cannonCodCompute : public Codelet 
{
    private:
        int tileIndex;
        int nextRow;
        int nextCol;

    public:
        cannonCodCompute(uint32_t dep, uint32_t reset, ThreadedProcedure * myTP,  uint32_t stat, int indx, int nextR, int nextC):
        Codelet(dep,reset,myTP, stat, "cannonCodCompute" + std::to_string(indx)), tileIndex(indx), nextRow(nextR), nextCol(nextC) {
        }

        virtual void fire(void);
};

class cannonLoopCodelet : public Codelet 
{
    private:
        int numIterations;
        int curIterations;

    public:
        cannonLoopCodelet(uint32_t dep, uint32_t reset, ThreadedProcedure * myTP,  uint32_t stat, int numIt) :
        Codelet(dep,reset,myTP, stat, "cannonLoopCodeletB" + std::to_string(numIt)), numIterations(numIt), curIterations(0) {}

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
        Codelet(dep,reset,myTP, stat, "cannonSkewCodelet" + std::to_string(tileIndex)), tileIndex(tileIndex), destRow(destR), destCol(destC) {}

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
        cannonLoopCodelet myLoopCntr;
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
            myLoopCntr(numTiles, numTiles, this, SHORTWAIT, sqrtNumTiles),
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
                    allComputeCodelets.push_back(new cannonCodCompute(1, 1, this, SHORTWAIT, i, nextRow, nextCol));
                    
                    // 1 dependancy from the loop codelet when all iterations are done. 
                    allCopyCCod.push_back(new cannonCodCopyC(1, 1, this, SHORTWAIT, i));
                }
                // Creation of the tiles
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
cannonCodCopyA::fire() {
    //std::cout<<"CopyA of " << this->tileIndex<<std::endl;
    cannonTP * myCannonTP = static_cast<cannonTP*> (this->myTP_);
    tile_mem * myTileMemory = (*myCannonTP->getAllMatrices())[this->tileIndex];
    matrix * A = myCannonTP->A;
    int sqrtNT = myCannonTP->sqrtNumTiles;

    // Get the offset of the element according to the tile ID
    int initial_index = (this->tileIndex/sqrtNT)*A->getM()*myTileMemory->getsqrtP() + (this->tileIndex % sqrtNT)*myTileMemory->getsqrtP();

    // Helper pointers
    double * origin;
    double * dest = myTileMemory->getTileA().getPtr();

    // Making the actual copy
    for (int i = initial_index; i < (initial_index + myTileMemory->getsqrtP()*A->getM()); i+=A->getM()) {
        origin = A->getPtr()+i;
        std::copy_n( origin, myTileMemory->getsqrtP(), dest);
        dest += myTileMemory->getsqrtP();
    }

    // Signal the compute codelet
    myCannonTP->allSkewCodelets[this->tileIndex]->decDep();
}

void
cannonCodCopyB::fire() {
    //std::cout<<"CopyB of " << this->tileIndex<<std::endl;
    cannonTP * myCannonTP = static_cast<cannonTP*> (this->myTP_);

    tile_mem * myTileMemory = (*myCannonTP->getAllMatrices())[this->tileIndex];
    matrix * B = myCannonTP->B;
    int sqrtNT = myCannonTP->sqrtNumTiles;

    int initial_index = (this->tileIndex/sqrtNT)*B->getM()*myTileMemory->getsqrtP() + (this->tileIndex % sqrtNT)*myTileMemory->getsqrtP();

    double * origin;
    double * dest = myTileMemory->getTileB().getPtr();

    for (int i = initial_index; i < (initial_index + myTileMemory->getsqrtP()*B->getM()); i+=B->getM()) {
        origin = B->getPtr()+i;
        std::copy_n( origin, myTileMemory->getsqrtP(), dest);
        dest += myTileMemory->getsqrtP();
    }

    // Signal the barrier codelet
    myCannonTP->allSkewCodelets[this->tileIndex]->decDep();
}

void
cannonCodCopyC::fire() {
    //std::cout<<"CopyC of " << this->tileIndex<<std::endl;
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
    //std::cout<<"Compute of " << this->tileIndex <<std::endl;
    // Get ready for the next iteration
    this->resetCodelet();

    cannonTP * myCannonTP = static_cast<cannonTP*> (this->myTP_);

    std::vector<tile_mem*> * matrices = myCannonTP->getAllMatrices();
    double * A = (*matrices)[this->tileIndex]->getTileA().getPtr(); 
    double * B = (*matrices)[this->tileIndex]->getTileB().getPtr();  
    double * C = (*matrices)[this->tileIndex]->getTileC().getPtr(); 

    int sideTile = (*matrices)[this->tileIndex]->getsqrtP();
    int wholeTile = (*matrices)[this->tileIndex]->getTileP();

    // call DGEMM
    DGEMM(
          sideTile,
          sideTile,
          sideTile,
          1,
          A,
          sideTile,
          B,
          sideTile,
          1,
          C,
          sideTile);

    // Shift A right and B down
    double * A_write = (*matrices)[this->nextRow]->getTileA_write().getPtr();
    double * B_write = (*matrices)[this->nextCol]->getTileB_write().getPtr();

    std::copy_n(A, wholeTile, A_write);
    std::copy_n(B, wholeTile, B_write);
    
    myCannonTP->myLoopCntr.decDep();
}

void 
cannonLoopCodelet::fire() {
    cannonTP * myCannonTP = static_cast<cannonTP*> (this->myTP_);
    std::vector<tile_mem*> * matrices = &myCannonTP->allMatrices;
    std::vector<cannonCodCompute*> * allComputeCodelets = &myCannonTP->allComputeCodelets;
    std::vector<cannonCodCopyC*> * allCopyCCodelets = &myCannonTP->allCopyCCod;

    this->resetCodelet();

    if (this->curIterations < this->numIterations) {
        // Spawn all the codelets
        for (auto it = matrices->begin(); it < matrices->end(); it++)
            (*it)->swap_write_read();

        this->curIterations++;
        for (auto it = allComputeCodelets->begin(); it < allComputeCodelets->end(); it++)
            (*it)->decDep(); 
        //std::cout<< "Iteration " << curIterations << " out of " << this->numIterations << std::endl; 
    } 
    // for last iteration , copy C back. 
    else {
        for (auto it = allCopyCCodelets->begin(); it < allCopyCCodelets->end(); it++)
            (*it)->decDep();
    }
}

void
cannonSkewCodelet::fire() {
    //std::cout<<"Skew of " << this->tileIndex <<std::endl;

    cannonTP * myCannonTP = static_cast<cannonTP*> (this->myTP_);

    std::vector<tile_mem*> * matrices = myCannonTP->getAllMatrices();
    double * A = (*matrices)[this->tileIndex]->getTileA().getPtr(); 
    double * B = (*matrices)[this->tileIndex]->getTileB().getPtr();  

    int sideTile = (*matrices)[this->tileIndex]->getsqrtP();
    int wholeTile = (*matrices)[this->tileIndex]->getTileP();

    // Shift A right and B down
    double * A_write = (*matrices)[this->destRow]->getTileA_write().getPtr();
    double * B_write = (*matrices)[this->destCol]->getTileB_write().getPtr();

    std::copy_n(A, wholeTile, A_write);
    std::copy_n(B, wholeTile, B_write);
        
    myCannonTP->myLoopCntr.decDep();
}

void 
cannonFinalCodelet::fire() {

    cannonTP * myCannonTP = static_cast<cannonTP*> (this->myTP_);

    myCannonTP->continuationCod->decDep();
}
#endif
