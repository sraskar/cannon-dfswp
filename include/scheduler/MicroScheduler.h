/* 
 * Copyright (c) 2011-2014, University of Delaware
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */


#pragma once
//Containers
#include <vector>

#include "getClock.h"
//Codelets and TPs
#include "codeletDefines.h"
#include "Codelet.h"
#include "ThreadedProcedure.h"
//Schedulers
#include "TPScheduler.h"
#include "Scheduler.h"

namespace darts
{
    enum MICROSCHED {MCSTANDARD = 0, 
                     MCSTATIC   = 1,                 
                     MCDYNAMIC  = 2,
                     MCSTEAL    = 3,
                     MCHINT     = 4};
    
    class MScheduler : public Scheduler
    {
    private:
        TPScheduler * parent_;
    protected:
        bool local_;
    public:
        MScheduler(void):
        parent_(NULL),
        local_(false){ }
        
	~MScheduler(void) {}

        TPScheduler * 
        getParentScheduler(void)
        {
            return parent_;
        }
        
        bool getLocal(void) { return local_; }
         
        void 
        setParentScheduler(TPScheduler * Parent)
        {
            parent_ = Parent;
        }
        
        virtual void policy(void) = 0;
        
        virtual bool pushCodelet(Codelet * codeletToPush) = 0;
        
        virtual bool pushLocal(Codelet *) = 0;

        virtual Codelet * popCodelet() { return NULL; }
        
        virtual bool empty(void)
        {
            return false;
        }
        
        static MScheduler * create(unsigned int type);
    };
}


