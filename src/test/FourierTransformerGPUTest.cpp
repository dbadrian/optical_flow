//
// Created by Adam Kosiorek on 23.05.15.
//
#include "gtest/gtest.h"
#include "common.h"
#include "IFourierTransformer.h"
#include "FourierTransformerCUFFTW.cpp"
#include "FourierPadder.h"

class FourierTransformerGPUTest : public testing::Test {

    void SetUp() {
        ftfftw = std::make_unique<FourierTransformerCUFFTW>(8,8);
    }

public:
    std::unique_ptr<FourierTransformerCUFFTW> ftfftw;
};

TEST_F(FourierTransformerGPUTest, ConstructorTest)
{
	ASSERT_EQ(8,ftfftw->rows_);
	ASSERT_EQ(8,ftfftw->cols_);
	ASSERT_EQ(8/2+1,ftfftw->colsHS_);
}

TEST_F(FourierTransformerGPUTest, ForwardTestMatrixSizes)
{
	RealMatrix 	inT(ftfftw->rows_,ftfftw->rows_),
				inF(7,9);
	ComplexMatrix 	outT(ftfftw->rows_,ftfftw->colsHS_),
				  	outF(3,3);

	// ASSERT_THROW(ftfftw->forward(inT,outF), std::invalid_argument);
	ASSERT_THROW(ftfftw->forward(inF,outT), std::invalid_argument);
	// ASSERT_THROW(ftfftw->forward(inF,outF), std::invalid_argument);
	ASSERT_NO_THROW(ftfftw->forward(inT,outT));
}

TEST_F(FourierTransformerGPUTest, BackwardTestMatrixSizes)
{
	ComplexMatrix 	inT(ftfftw->rows_,ftfftw->colsHS_),
				  	inF(3,3);
	RealMatrix 	outT(ftfftw->rows_,ftfftw->rows_),
				outF(7,9);

	// ASSERT_THROW(ftfftw->backward(inT,outF), std::invalid_argument);
	ASSERT_THROW(ftfftw->backward(inF,outT), std::invalid_argument);
	// ASSERT_THROW(ftfftw->backward(inF,outF), std::invalid_argument);
	ASSERT_NO_THROW(ftfftw->backward(inT,outT));
}

TEST_F(FourierTransformerGPUTest, FowardInputConsistency)
{
	RealMatrix 		rm(ftfftw->rows_,ftfftw->rows_);

	rm << 	17	,24	,1	,8	,15	,0,	0,	0,
			23	,5	,7	,14	,16	,0,	0,	0,
			4	,6	,13	,20	,22	,0,	0,	0,
			10	,12	,19	,21	,3	,0,	0,	0,
			11	,18	,25	,2	,9	,0,	0,	0,
			0	,0	,0	,0	,0	,0,	0,	0,
			0	,0	,0	,0	,0	,0,	0,	0,
			0	,0	,0	,0	,0	,0,	0,	0;

	RealMatrix		rm_copy = rm;

	ComplexMatrix	cm(ftfftw->rows_,ftfftw->colsHS_);

	ftfftw->forward(rm,cm);
	ASSERT_EQ(true,rm.isApprox(rm_copy,0.000000001));
}

TEST_F(FourierTransformerGPUTest, ForwardBackwardScaling)
{
	RealMatrix 		rm(ftfftw->rows_,ftfftw->rows_);

	rm << 	17	,24	,1	,8	,15	,0,	0,	0,
			23	,5	,7	,14	,16	,0,	0,	0,
			4	,6	,13	,20	,22	,0,	0,	0,
			10	,12	,19	,21	,3	,0,	0,	0,
			11	,18	,25	,2	,9	,0,	0,	0,
			0	,0	,0	,0	,0	,0,	0,	0,
			0	,0	,0	,0	,0	,0,	0,	0,
			0	,0	,0	,0	,0	,0,	0,	0;
	RealMatrix		rm_copy = rm;

	ComplexMatrix	cm(ftfftw->rows_,ftfftw->cols_/2+1);
	cm.setZero();

	ftfftw->forward(rm,cm);
	ftfftw->backward(cm,rm);
	rm = rm/(ftfftw->rows_*ftfftw->cols_);

	ASSERT_EQ(true,rm.isApprox(rm_copy,0.000001));
}

TEST_F(FourierTransformerGPUTest, FwdBwdFilterTest)
{
	RealMatrix	rmData(5,5), rmFilter(4,4); // MAGIC NUMBERS WHICH END UP AT 8x8

	rmData << 	17, 24, 1, 8, 15,
			23, 5, 7, 14, 16,
			4, 6, 13, 20, 22,
			10, 12, 19, 21, 3,
			11, 18, 25, 2, 9;

	rmFilter << 16,	2,	3, 13,
				5,	11,	10, 8,
				9,	7,	6, 12,
				4,  14, 15, 1;

	RealMatrix		rmDataPadded, rmFilterPadded;
	FourierPadder padder(5, 4);
	padder.padData(rmData,rmDataPadded);
	padder.padFilter(rmFilter,rmFilterPadded);

	ComplexMatrix	cmData(ftfftw->rows_,ftfftw->cols_/2+1);
	ComplexMatrix	cmFilter(ftfftw->rows_,ftfftw->cols_/2+1);
	cmData.setZero(); 	cmFilter.setZero();

	ftfftw->forward(rmDataPadded,cmData);
	ftfftw->forward(rmFilterPadded,cmFilter);

	ComplexMatrix cmResult = cmData.cwiseProduct(cmFilter);

	ftfftw->backward(cmResult,rmDataPadded);
	rmDataPadded = rmDataPadded/(ftfftw->rows_*ftfftw->cols_);
	padder.extractDenseOutput(rmDataPadded,rmData);

	RealMatrix expectedResult(5,5);
	expectedResult << 	831, 1224, 1338,  810,  784,
						1360, 1779, 1366, 1473, 1159,
						1422, 1400, 1847, 1615,  877,
						944, 1423, 1462, 1315,  726,
						811,  973, 1064,  751,  144;

	std::cout << rmData << std::endl;
	ASSERT_EQ(true,rmData.isApprox(expectedResult,0.000001));
}