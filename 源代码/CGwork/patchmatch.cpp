#include"patchmatch.h"

//°áÔËº¯Êý;
void patchmatch::fromImage2Array(cv::Mat img,double **data,int cols,int rows){
	(*data) = new  double [rows*cols*3];
	for (int y=0;y<rows;y++)
	{
		for (int x=0;x<cols;x++)
		{
			(*data)[3*(y*cols+x)+0] = img.at<cv::Vec3f>(y,x)[0] ;
			(*data)[3*(y*cols+x)+1] = img.at<cv::Vec3f>(y,x)[1] ;
			(*data)[3*(y*cols+x)+2] = img.at<cv::Vec3f>(y,x)[2] ;
		
		}
	}
}
void patchmatch::fromArray2Image(cv::Mat img,double *data,int cols,int rows){
	for (int x=0;x<cols;x++)
	{
		for (int y=0;y<rows;y++)
		{
				img.at<cv::Vec3f>(y,x)[0] = data[3*(y*cols+x)+0] ;
				img.at<cv::Vec3f>(y,x)[1] = data[3*(y*cols+x)+1] ;
				img.at<cv::Vec3f>(y,x)[2] = data[3*(y*cols+x)+2] ;
		}
	}
}
void patchmatch::fromImage2Array(cv::Mat img,int **data,int cols,int rows){
	(*data) = new  int [rows*cols*3];
	for (int y=0;y<rows;y++)
	{
		for (int x=0;x<cols;x++)
		{
			(*data)[3*(y*cols+x)+0]= img.at<cv::Vec3b>(y,x)[0] ;
			(*data)[3*(y*cols+x)+1] = img.at<cv::Vec3b>(y,x)[1] ;
			(*data)[3*(y*cols+x)+2] = img.at<cv::Vec3b>(y,x)[2] ;
		}
	}
}
void patchmatch::fromArray2Image(cv::Mat img,int *data,int cols,int rows){
	for (int x=0;x<cols;x++)
	{
		for (int y=0;y<rows;y++)
		{
				img.at<cv::Vec3b>(y,x)[0] = data[3*(y*cols+x)+0] ;
				img.at<cv::Vec3b>(y,x)[1] = data[3*(y*cols+x)+1] ;
				img.at<cv::Vec3b>(y,x)[2] = data[3*(y*cols+x)+2] ;
		}
	}
}
//patchmatchºËÐÄº¯Êý;
void patchmatch::InitRandom(){
	generate = new  double [target_rows*target_cols];
	ralation = new  int [target_rows*target_cols*2];
	for (int y=0;y<target_rows;y++)
	{	
		for (int x=0;x<target_cols;x++)
		{
			ralation[2*(y*target_cols+x)+0] = rand()%source_rows;
			ralation[2*(y*target_cols+x)+1] = rand()%source_cols;
			generate[y*target_cols+x]=1000;
		}
	}
}
void patchmatch::Generate(){
	for (int y=0;y<target_rows;y++)
	{	
		for (int x=0;x<target_cols;x++)
		{
			generate[y*target_cols+x]=0;
		}
	}

	int row ,col;
	int x_t,y_t;
	for (int x=0;x<target_cols;x++)
	{
		for (int y=0;y<target_rows;y++)
		{
			x_t = x;
			y_t = y;
			row = ralation[2*(y*target_cols+x)+0];
			col = ralation[2*(y*target_cols+x)+1] ;

			if( x_t-radius<0 )  x_t = radius;
			if( y_t-radius<0 )  y_t =radius;
			if( col-radius<0 )  col = radius;
			if( row-radius<0 )  row = radius;

			if( x_t+radius>= target_cols)  x_t = target_cols-1-radius;
			if( y_t+radius>= target_rows )  y_t = target_rows-1-radius;
			if( col+radius>= source_cols )  col = source_cols-1-radius;
			if( row+radius>= source_rows )  row = source_rows-1-radius;
			
			for(int m=-radius ;m<=radius;m++){
				for(int n=-radius ;n<=radius;n++){
					double w = pow(2.7,-abs(m*n));
					
					int a = 3*((y_t+n)*target_cols+(x_t+m));
					int b = 3*((row+n)*source_cols+(col+m));

					reslut_data[a+0] +=w* source_data[b+0];
					reslut_data[a+1] +=w* source_data[b+1];
					reslut_data[a+2] +=w* source_data[b+2];
					generate[(y_t+n)*target_cols+x_t+m] +=w;
				}	
			}
		}
	}
	/**/
	for (int y=0;y<target_rows;y++)
	{
		for (int x=0;x<target_cols;x++)
		{
			reslut_data[3*(y*target_cols+x)+0] /= generate[y*target_cols+x] ;
			reslut_data[3*(y*target_cols+x)+1] /= generate[y*target_cols+x] ;
			reslut_data[3*(y*target_cols+x)+2] /= generate[y*target_cols+x] ;
			generate[y*target_cols+x] =0;
		}
	}
}
void patchmatch::patchmatchGPU(){
	double  * target_block;//ÉêÇëGPU¿Õ¼ä£»
	int  * relation_block;//ÉêÇëGPU¿Õ¼ä£»
	double  * source_block;//ÉêÇëGPU¿Õ¼ä£»
	double  * distance;//ÉêÇëGPU¿Õ¼ä£»

	int source_size = source_cols *source_rows;
	int target_size = target_cols *target_rows;

	cudaMalloc((void**)&target_block			,	target_size		*3*sizeof(double));
	cudaMalloc((void**)&source_block		,	source_size	*3*sizeof(double));
	cudaMalloc((void**)&relation_block		,	target_size		*2*sizeof(int));
	cudaMalloc((void**)&distance		,	target_size		*sizeof(int));

	cudaMemcpy(target_block	,	target_data	,	target_size		*3*sizeof(double),cudaMemcpyHostToDevice);
	cudaMemcpy(source_block	,	source_data	,	source_size	*3*sizeof(double),cudaMemcpyHostToDevice);
	cudaMemcpy(relation_block,	ralation			,	target_size		*2*sizeof(int),cudaMemcpyHostToDevice);
	cudaMemcpy(distance,	generate	,	target_size		*sizeof(int),cudaMemcpyHostToDevice);

	bridge(target_block, source_block, relation_block, target_rows,target_cols,source_rows,source_cols,distance);

	cudaMemcpy(target_data	,	target_block	, target_size*3*sizeof(double)	,cudaMemcpyDeviceToHost);
	cudaMemcpy(source_data	,	source_block	, source_size*3*sizeof(double)	,cudaMemcpyDeviceToHost);
	cudaMemcpy(ralation			,   relation_block,target_size*2*sizeof(int)		,cudaMemcpyDeviceToHost);

	cudaFree(target_block);
	cudaFree(source_block);
	cudaFree(relation_block);
	cudaFree(distance);
}