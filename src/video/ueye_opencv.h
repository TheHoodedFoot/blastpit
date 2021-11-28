/* This header can be read by both C and C++ compilers */
#ifndef UEYE_OPENCV_H
#define UEYE_OPENCV_H
#ifdef __cplusplus
// class Fred {
//       public:
// 	Fred();
// 	void wilma(int);

//       private:
// 	int a_;
// };
#else
// typedef struct Fred Fred;
#endif
#ifdef __cplusplus
extern "C"
{
#endif
	int  UeyeToOpenCV( unsigned char* bitmap, int width, int height );
	void UeyeCloseWindow();
#ifdef __cplusplus
}
#endif
#endif /*UEYE_OPENCV_H*/
