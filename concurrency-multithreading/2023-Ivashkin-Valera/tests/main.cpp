#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <set>
#include <map>
#include <chrono>


#include "test/ArraySumTest.h"
#include "test/ArrayInsertTest.h"
#include "test/HashInsertTest.h"
#include "test/ListInsertTest.h"
#include "test/TreeInsertTest.h"
#include "test/TreeRemoveTest.h"


/*#if defined(TLRW_EAGER)
#define ALGORITHM "tlrw_eager"
#elif defined(COHORTS)
#define ALGORITHM "cohorts"
#elif defined(TML_LAZY)
#define ALGORITHM "tml_lazy"
#else
#define ALGORITHM "NONE"
#endif*/
#define ALGORITHM "OREC_MIXED_UNSAFE"


using namespace std;

typedef map<string, function<ITest *(void)>> TestsMap;

static const TestsMap TESTS = {
    {"ArraySumTest", [] { return new ArraySumTest(); }},
    {"ArrayInsertTest", [] { return new ArrayInsertTest(); }},
    { "ListInsertTest", [] { return new ListInsertTest(); } },
    { "TreeInsertTest", [] { return new TreeInsertTest(); } },
    { "TreeRemoveTest", [] { return new TreeRemoveTest(); } },
    { "HashInsertTest", [] { return new HashInsertTest(); } },
};

typedef std::chrono::high_resolution_clock Clock;

int main()
{
    cout << ALGORITHM+ "starts testing" << endl;

    size_t testsCount = 0;

    ifstream params("Data/params.txt"); //get testing params
    ofstream results(string("Data/")+ALGORITHM+"Result.txt"); //output file
    if (params.is_open() && results.is_open())
    {

        while (true)
        {
            string testName;
            size_t threadsCount;
            size_t inputSize;
            size_t repeatCount;

            const int sym = params.get();
            if (sym == -1) {
                // end of file
                break;
            } else if (!std::isalpha(sym)) {
                // skip comment and white space
                params.unget();
                string line;
                getline(params, line);
                continue;
            } else {
                params.unget();
                params >> testName >> threadsCount >> inputSize >> repeatCount;
                if (!params.good()) {
                    continue;
                }
            }

            TestsMap::const_iterator it = TESTS.find(testName);
            if (it == TESTS.end())
            {
                cerr << "Test not found: " << testName << endl;
                continue;
            }

            results << "Test: " << testName << endl;
            results << "Threads count: " << threadsCount << endl;
            results << "Input size: " << inputSize << endl;
            results << "Repeat count: " << repeatCount << endl;

            ITest *test = it->second();
            double msThreadedAv = 0.0;

            bool isOk = true;

            for (size_t i = 0; i < repeatCount; i++)
            {
                // generate data
                test->generate(inputSize, threadsCount);
                test->setup();

                results << setw(20) << left << "\tRun...";
                Clock::time_point t0 = Clock::now();
                test->run();
                Clock::time_point t1 = Clock::now();

                if (test->check))
                {
                    const double ms = std::chrono::nanoseconds(t1 - t0).count() * 1e-6;
                    const size_t opsPerSec = static_cast<size_t>(ceil((double)inputSize / ms));

                    msThreadedAv += ms;
                    results << "OK " << fixed << setprecision(3) << ms << " ms, " << opsPerSec << " ops/s" << endl;
                }
                else
                {
                    isOk = false;
                    results << "FAIL " << endl;
                }

                test->teardown();

                results << flush;
            }

            if (isOk)
            {
                msThreadedAv /= repeatCount;

                size_t opsPerSecAv = static_cast<size_t>(ceil((double)inputSize / msThreadedAv));

                results << endl
                     << "> " << testName << " " << ALGORITHM << " OK "
                     << inputSize << " " << threadsCount << " " << repeatCount << " " << msThreadedAv << " " << opsPerSecAv << endl;

                    cout << endl
                     << "> " << testName << " " << ALGORITHM << " OK "
                     << inputSize << " " << threadsCount << " " << repeatCount << " " << msThreadedAv << " " << opsPerSecAv << endl;
            }
            else
            {
                cout << endl
                     << "> " << testName << " fail" << endl;
                results << endl
                     << "> " << testName << " fail" << endl;
            }

            delete test;

            results << endl
                 << endl;
            testsCount++;
        }
    }
    params.close();
    results.close();

    if (testsCount > 0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}