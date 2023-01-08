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


#include "TPSchedPolicy.h"
#include "TPScheduler.h"
#include "Codelet.h"
#include "MicroScheduler.h"
#include <cstdlib>
#include "tpClosure.h"
#ifdef TRACE
#include "getClock.h"
#endif

namespace darts {

    void
    TPRoundRobin::policy() {
#ifdef TRACE
        addRecord(getTime(), (void*) &TPRoundRobin::policy, TPS_START, "");
#endif       
        while (alive()) {
            //Check if we have any work in our deque
            tpClosure * tempClosure;
            if (!(tempClosure = popTP()))
                tempClosure = steal();

            if (tempClosure) {
#ifdef TRACE
                addRecord(getTime(), (void*) tempClosure->factory, TP_START, "");
#endif
                tempClosure->factory(tempClosure);
#ifdef TRACE
                //addRecord(getTime(), (void*) tempClosure->factory, TP_END, "");
#endif
                delete tempClosure;
                //Get the work ready!
            }
            //Lets do the work!
            Codelet * tempCodelet = popCodelet();
            while (tempCodelet) {
                MScheduler * myCDS = static_cast<MScheduler*> (getSubScheduler(getSubIndexInc()));
                while (!myCDS->pushCodelet(tempCodelet)) {
                    myCDS = static_cast<MScheduler*> (getSubScheduler(getSubIndexInc()));
                }
                tempCodelet = popCodelet();
            }
        }
#ifdef TRACE
        addRecord(getTime(), (void*) &TPRoundRobin::policy, TPS_STOP, "");
#endif
    }

    void
    TPPushFull::policy() {
#ifdef TRACE
        addRecord(getTime(), (void*) &TPPushFull::policy, TPS_START, "");
#endif
        while (alive()) {
            //Check if we have any work in our deque
            tpClosure * tempClosure = popTP();
            if (!tempClosure)
                tempClosure = steal();

            if (tempClosure) {
#ifdef TRACE
                addRecord(getTime(), (void*) tempClosure->factory, TP_START, "");
#endif
                tempClosure->factory(tempClosure);
#ifdef TRACE
                //addRecord(getTime(), (void*) &TPPushFull::policy);
#endif
                delete tempClosure;
                //Get the work ready!
            }
            //Lets do the work!
            Codelet * tempCodelet = popCodelet();
            while (tempCodelet) {
                //Here we are going to try to push
                bool fail = true;
                for (size_t i = 0; i < getNumSub(); i++) {
                    MScheduler * myCDS = static_cast<MScheduler*> (getSubScheduler(getSubIndexInc()));
                    if (myCDS->pushCodelet(tempCodelet)) {
                        fail = false;
                        break;
                    }
                }
                //If we fail do it ourself
                if (fail) {
                    ThreadedProcedure * checkTP = tempCodelet->getTP();
                    bool deleteTP = (checkTP) ? checkTP->checkParent() : false;
#ifdef TRACE
                    addRecord(getTime(), tempCodelet->returnFunct(), CODELET_START, tempCodelet->getName());
#endif
#ifdef COUNT
		    if(getAffinity()) getAffinity()->startCounters(getID());
#endif
		    tempCodelet->fire();
#ifdef COUNT
		    if(getAffinity()) getAffinity()->incrementCounters(getID());
#endif
#ifdef TRACE
                    addRecord(getTime(), tempCodelet->returnFunct(), CODELET_END, tempCodelet->getName());
#endif
                    if (deleteTP) {
                        if (checkTP->decRef())
                            delete checkTP;
                    }
                }
                tempCodelet = popCodelet();
            }
        }
#ifdef TRACE
        addRecord(getTime(), (void*) &TPRoundRobin::policy, TPS_STOP, "");
#endif
    }

    void
    TPStatic::policy() {
#ifdef TRACE
        addRecord(getTime(), (void*) &TPStatic::policy, TPS_START, "");
#endif
        while (alive()) {
            //Check if we have any work in our deque
            tpClosure * tempClosure = popTP();
            if (!tempClosure)
                tempClosure = steal();

            if (tempClosure) {
#ifdef TRACE
                addRecord(getTime(), (void*) tempClosure->factory, TP_START, "");
#endif
                tempClosure->factory(tempClosure);
#ifdef TRACE
                //addRecord(getTime(), (void*) &TPStatic::policy);
#endif
                delete tempClosure;
                //Get the work ready!
            }

            Codelet * tempCodelet = popCodelet();
            while (tempCodelet) {
                ThreadedProcedure * checkTP = tempCodelet->getTP();
                bool deleteTP = (checkTP) ? checkTP->checkParent() : false;
#ifdef TRACE
                addRecord(getTime(), tempCodelet->returnFunct(), CODELET_START, tempCodelet->getName());
#endif
#ifdef COUNT
		    if(getAffinity()) getAffinity()->startCounters(getID());
#endif
		    tempCodelet->fire();
#ifdef COUNT
		    if(getAffinity()) getAffinity()->incrementCounters(getID());
#endif
#ifdef TRACE
                addRecord(getTime(), tempCodelet->returnFunct(), CODELET_END, tempCodelet->getName());
#endif
                if (deleteTP) {
                    if (checkTP->decRef())
                        delete checkTP;
                }
                tempCodelet = popCodelet();
            }
        }
#ifdef TRACE
        addRecord(getTime(), (void*) &TPRoundRobin::policy, TPS_STOP, "");
#endif
    }

    bool
    TPStatic::pushCodelet(Codelet * CodeletToPush) 
    {
        uint64_t status = CodeletToPush->getStatus();
        size_t numSub = getNumSub();
        if (!status || !numSub)
        {
            return codelets_.push(CodeletToPush);
        }
        MScheduler * myCDS = static_cast<MScheduler*> (getSubScheduler((status - 1) % numSub));
        return myCDS->pushCodelet(CodeletToPush);
    }

    void
    TPDynamic::policy() {
#ifdef TRACE
        addRecord(getTime(), (void*) &TPDynamic::policy, TPS_START, "");
#endif
        while (alive()) {
            //Check if we have any work in our deque
            tpClosure * tempClosure;
            if (!(tempClosure = popTP()))
                tempClosure = steal();

            if (tempClosure) {

#ifdef TRACE
                addRecord(getTime(), (void*) tempClosure->factory, TP_START, "");
#endif
                tempClosure->factory(tempClosure);
#ifdef TRACE
                //addRecord(getTime(), (void*) &TPDynamic::policy, );
#endif
                delete tempClosure;
                //Get the work ready!
            }

            //Lets do the work!
            Codelet * tempCodelet = popCodelet();
            while (tempCodelet) {
                ThreadedProcedure * checkTP = tempCodelet->getTP();
                bool deleteTP = (checkTP) ? checkTP->checkParent() : false;
#ifdef TRACE
                addRecord(getTime(), tempCodelet->returnFunct(), CODELET_START, tempCodelet->getName());
#endif
#ifdef COUNT
		    if(getAffinity()) getAffinity()->startCounters(getID());
#endif
		    tempCodelet->fire();
#ifdef COUNT
		    if(getAffinity()) getAffinity()->incrementCounters(getID());
#endif
#ifdef TRACE
                addRecord(getTime(), (void*) &TPDynamic::policy, CODELET_END, tempCodelet->getName());
#endif
                if (deleteTP) {
                    if (checkTP->decRef())
                        delete checkTP;
                }

                tempCodelet = popCodelet();
            }
        }
#ifdef TRACE
        addRecord(getTime(), (void*) &TPRoundRobin::policy, TPS_STOP, "");
#endif
    }

    void
    TPWorkPush::policy() {
#ifdef TRACE
        addRecord(getTime(), (void*) &TPWorkPush::policy, TPS_START, "");
#endif
        while (alive()) {
            //Check if we have any work in our deque
            tpClosure * tempClosure = popTP();

            if (tempClosure) {
#ifdef TRACE
                addRecord(getTime(), (void*) tempClosure->factory, TP_START, "");
#endif
                tempClosure->factory(tempClosure);
#ifdef TRACE
                addRecord(getTime(), (void*) &TPWorkPush::policy, TP_END,"");
#endif
                delete tempClosure;
                //Get the work ready!
            }
            //Lets do the work!
            Codelet * tempCodelet = popCodelet();
            while (tempCodelet) {
                //Here we are going to try to push
                bool fail = true;
                while (fail) {
                    MScheduler * myCDS = static_cast<MScheduler*> (getSubScheduler(getSubIndexInc()));
                    if (myCDS->pushCodelet(tempCodelet)) {
                        fail = false;
                        break;
                    }
                }
                tempCodelet = popCodelet();
            }
        }
#ifdef TRACE
        addRecord(getTime(), (void*) &TPRoundRobin::policy, TPS_STOP, "");
#endif
    }

    void
    TPHintSched::policy() {
#ifdef TRACE
        addRecord(getTime(), (void*) &TPHintSched::policy, TPS_START, "");
#endif       
        while (alive()) {
            //Check if we have any work in our deque
            tpClosure * tempClosure;
            if (!(tempClosure = popTP()))
                tempClosure = steal();

            if (tempClosure) {
#ifdef TRACE
                addRecord(getTime(), (void*) tempClosure->factory, TP_START, "");
#endif
                tempClosure->factory(tempClosure);
                delete tempClosure;
                //Get the work ready!
            }
            //Schedule codelets, if no hint, do work
            Codelet * tempCodelet = popCodelet(); 
            while (tempCodelet) {
                uint32_t hint = tempCodelet->getSchedHint();
                MScheduler * myCDS;
                if (hint < this->getNumSub()) {
                    // If there is a hint, we push it until it is there. 
                    myCDS = static_cast<MScheduler*> (getSubScheduler(hint));
                    while (!myCDS->pushCodelet(tempCodelet));
                } else {
                    // Otherwise we use regular round robin
                    uint32_t sendTo = getSubIndexInc();
                    myCDS = static_cast<MScheduler*> (getSubScheduler(sendTo));
                    while (!myCDS->pushCodelet(tempCodelet)) {
                        sendTo = getSubIndexInc();
                        myCDS = static_cast<MScheduler*> (getSubScheduler(sendTo));
                    }
                }
                tempCodelet = popCodelet();
            }
        }
#ifdef TRACE
        addRecord(getTime(), (void*) &TPHintSched::policy, TPS_STOP, "");
#endif
    }

    TPScheduler *
    TPScheduler::create(unsigned int type) 
    {
        if (type == TPROUNDROBIN) return new TPRoundRobin;
        if (type == TPPUSHFULL) return new TPPushFull;
        if (type == TPSTATIC) return new TPStatic;
        if (type == TPDYNAMIC) return new TPDynamic;        
        if (type == TPHINT) return new TPHintSched;        
        else return NULL;
    }

} // Namespace darts
