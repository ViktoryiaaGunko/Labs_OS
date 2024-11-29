#define _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING
#include <gtest/gtest.h>
#include <vector>
#include <windows.h>
#include <thread>
#include <mutex>
#include "Main.cpp" 
#ifndef TEST_MODE


TEST(MinMaxTest, BasicAssertions) {
    Data a;
    a.numbs = { 3, 1, 4, 1, 5, 9, 2, 6, 5 };

    HANDLE hMin_Max = CreateThread(nullptr, 0, Min_Max, &a, 0, nullptr);
    WaitForSingleObject(hMin_Max, INFINITE);

    EXPECT_EQ(a.min, 1);
    EXPECT_EQ(a.max, 9);

    CloseHandle(hMin_Max);
}

TEST(AverageTest, BasicAssertions) {
    Data b;
    b.numbs = { 3, 1, 4, 1, 5, 9 };

    HANDLE hAverage = CreateThread(nullptr, 0, Average, &b, 0, nullptr);
    WaitForSingleObject(hAverage, INFINITE);

    EXPECT_DOUBLE_EQ(b.average, 3.833333); 

    CloseHandle(hAverage);
}

//óñëîâíàÿ êîìïèëÿöèÿ äëÿ ïðåäîòâðàùåíèÿ êîíôëèêòà ñ main
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
#endif
