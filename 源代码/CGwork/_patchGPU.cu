#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <math.h>
#include<curand_kernel.h>
//计算距离
__device__ float calcDistance(int target_row ,int target_col,int source_row, int source_col , double * target_block ,double * source_block,int target_rows,int target_cols ,int source_rows,int source_cols)
{	
	int r = 3;
	double dif=0;
	double dif0=1,dif1=1,dif2=1;

	if( target_row-r<0 )  target_row = r;
	if( target_col-r<0 )  target_col =r;
	if( source_row-r<0 )  source_row = r;
	if( source_col-r<0 )  source_col = r;

	if( target_row+r>=target_rows )  target_row = target_cols-1-r;
	if( target_col+r>= target_cols )  target_col = target_rows-1-r;
	if( source_row+r>=source_rows  )  source_row = source_rows-1-r;
	if( source_col+r>= source_cols )  source_col = source_cols-1-r;	

	for(int i=-r ;i<=r;i++){
		for(int j=-r ;j<=r;j++){
			int temp = 3*((source_row+i)*source_cols+source_col+j) ;
			int temp2 = 3*((target_row+i)*target_cols+target_col+j) ;
			dif0 = source_block[ temp+ 0] - target_block[temp2+ 0] ;
			dif1 = source_block[ temp+ 1] - target_block[temp2+ 1] ;
			dif2 = source_block[ temp+ 2] - target_block[temp2+ 2] ;
			dif += sqrt(dif0*dif0 +dif1*dif1 +dif2*dif2);
		}	
	}
	return dif;
}
__device__ int calcDistance(int target_row ,int target_col,int source_row1, int source_col1,int source_row2, int source_col2 ,int source_row3, int source_col3,double * target_block , double *source_block,int target_rows,int target_cols ,int source_rows,int source_cols)
{
	float first2Second = calcDistance(target_row,  target_col , source_row1,  source_col1,target_block ,source_block,target_rows,target_cols,source_rows, source_cols);
	float first2Third = calcDistance(  target_row, target_col , source_row2,  source_col2,target_block ,source_block,target_rows,target_cols,source_rows, source_cols);
	float first2Fourth = calcDistance( target_row,target_col , source_row3,  source_col3,target_block ,source_block,target_rows,target_cols,source_rows, source_cols);
	if (first2Second<=first2Third)
	{
		if (first2Second<=first2Fourth)
			return 1;
		else
			return 3;
	}
	else if (first2Third<= first2Fourth)
		return 2;
	else
		return 3;
}
__device__ int calcDistance(int target_row,int target_col ,int source_row1, int source_col1,int source_row2, int source_col2 ,double * target_block ,double * source_block,int target_rows,int target_cols ,int source_rows,int source_cols)
{
	float first2Second = calcDistance(target_row,  target_col , source_row1,  source_col1,target_block ,source_block,target_rows,target_cols,source_rows, source_cols);
	float first2Third = calcDistance(  target_row, target_col , source_row2,  source_col2,target_block ,source_block,target_rows,target_cols,source_rows, source_cols);
	if (first2Second <= first2Third)
		return 1;
	return 2;
}

__global__ void extern PropagationGPU(double * target_block ,double * source_block ,  int * relation_block  ,  int target_rows  ,  int target_cols ,int source_rows  ,  int source_cols)
{
	//传递过程；
	int y = blockIdx.x;
	int x = threadIdx.x;

	//定义半径；
	int c_r0 = relation_block[ 2*(y*target_cols+x) + 0 ];
	int c_c0 = relation_block[ 2*(y*target_cols+x) + 1];

	int c_r1 = relation_block[ 2*((y+1)*target_cols+x) + 0 ]-1;
	int c_c1 = relation_block[ 2*((y+1)*target_cols+x) + 1 ];

	int c_r2 = relation_block[ 2*(y*target_cols+x+1) + 0];
	int c_c2 = relation_block[ 2*(y*target_cols+x+1) + 1]-1;

	int patchNumber = calcDistance(y  ,  x  ,  c_r0 ,  c_c0 , c_r1, c_c1 , c_r2 , c_c2 , target_block ,source_block,target_rows,target_cols,source_rows, source_cols);
	switch(patchNumber)
	{ 
		case 2:
			relation_block[ 2*(y*target_cols+x) + 0 ]= c_r1;
			relation_block[ 2*(y*target_cols+x) + 1 ]= c_c1;
			break;
		case 3:
			relation_block[ 2*(y*target_cols+x) + 0 ] = c_r2;
			relation_block[ 2*(y*target_cols+x) + 1 ] = c_c2;
			break;
	}
}
__global__ void extern RandomSearchGPU(double * target_block ,double * source_block ,int * relation_block,int target_rows,int target_cols ,int source_rows,int source_cols){
	//传递过程；
	int y = blockIdx.x;
	int x = threadIdx.x;

	//定义半径；
	int c_r0 = relation_block[ 2*(y*target_cols+x) + 0 ];
	int c_c0 = relation_block[ 2*(y*target_cols+x) + 1];

	int c_r1 = relation_block[ 2*((y-2)*target_cols+x) + 0 ]+2;
	int c_c1 = relation_block[ 2*((y-2)*target_cols+x) + 1 ];

	int c_r2 = relation_block[ 2*(y*target_cols+x-2) + 0];
	int c_c2 = relation_block[ 2*(y*target_cols+x-2) + 1]+2;

	int patchNumber = calcDistance(y  ,  x  ,  c_r0 ,  c_c0 , c_r1, c_c1 , c_r2 , c_c2 , target_block ,source_block,target_rows,target_cols,source_rows, source_cols);
	switch(patchNumber)
	{
		case 2:
			relation_block[ 2*(y*target_cols+x) + 0 ]= c_r1;
			relation_block[ 2*(y*target_cols+x) + 1 ]= c_c1;
			break;
		case 3:
			relation_block[ 2*(y*target_cols+x) + 0 ] = c_r2;
			relation_block[ 2*(y*target_cols+x) + 1 ] = c_c2;
			break;
	}
}

__global__ void extern baoli(double * target_block ,double * source_block ,int * relation_block,int target_rows,int target_cols ,int source_rows,int source_cols,double *distance){
	int y = threadIdx.y;
	int x = threadIdx.x;

	for(int i  = 0 ; i<12;i++){
		for(int j =  0 ;j< 12 ; j++){
			double c =  calcDistance(y  ,  x  ,  i ,  j ,  target_block ,source_block,target_rows,target_cols,source_rows, source_cols);
			if( c < distance[ y*target_cols+x ]){
				relation_block[ 2*(y*target_cols+x) + 0 ]= 1;
				relation_block[ 2*(y*target_cols+x) + 1 ]= 1;
				distance[ y*target_cols+x ] = c;
			}
		}
	}
}

void extern  bridge(double * target_block ,double * source_block  ,int *  relation_block,int target_rows,int target_cols ,int source_rows,int source_cols, double * distance){
	//对线程大小有限制；
/**/	for(int i = 0;i<130 ;i++){
		PropagationGPU<<<target_rows  ,target_cols>>>(target_block, source_block, relation_block , target_rows , target_cols , source_rows , source_cols);
		cudaThreadSynchronize();
		RandomSearchGPU<<<target_rows	,target_cols>>>(target_block, source_block, relation_block,target_rows,target_cols,source_rows, source_cols);
		cudaThreadSynchronize();
	}

	//baoli<<<target_rows  ,target_cols>>>(target_block, source_block, relation_block , target_rows , target_cols , source_rows , source_cols	,distance);
}