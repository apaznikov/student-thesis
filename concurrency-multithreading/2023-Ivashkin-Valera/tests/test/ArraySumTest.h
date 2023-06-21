#ifndef ARRAYSUMTEST_H
#define ARRAYSUMTEST_H

#include "Test.h"

class ArraySumTest: public NumbersTest {
public:
    virtual void setup() {
        m_sharedSum = 0;
    }

    /*
    *  Check in case STM algorithm make wrong result
    */
    virtual bool check() {
        size_t refSum = 0;
        for(size_t i = 0; i < m_inputSize; i++) {
            refSum += m_input[i];
        }
        return (refSum == m_sharedSum);
    }

protected:
    /*
    * Function that threads execute in parallel
    * start&&end - points at part of sequience data for exact thread
    */
    virtual void worker(size_t start, size_t end) {
        size_t localSum = 0;
        for(size_t i = start; i < end; i++) {
            localSum += m_input[i];
        }
        //First way to express transaction
        /*TX_BEGIN{
            m_sharedSum += localSum;
        } TX_END;*/

        //Second way to express transaction
        {
            TX_RAII;
            m_sharedSum += localSum;
        }
    }

    size_t m_sharedSum;
};


#endif // ARRAYSUMTEST_H