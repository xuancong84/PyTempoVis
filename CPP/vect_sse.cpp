// Vector Arithmetic using SSE, define USESIMD to enable SSE
#include "parameters.h"
#include "TempoVis.h"
#include <math.h>
#include <memory.h>

FLOAT *MatrixMulVector4( FLOAT *mat, FLOAT *vin, FLOAT *vout ){
#ifdef	USESIMD
	asm(".intel_syntax noprefix\n"
		"mov			eax,	[mat]\n"
		"mov			ebx,	[vin]\n"
		"movups		xmm0,	[eax]\n"
		"movss		xmm4,	[ebx]\n"
		"movups		xmm1,	[eax+16]\n"
		"movss		xmm5,	[ebx+4]\n"
		"movups		xmm2,	[eax+32]\n"
		"movss		xmm6,	[ebx+8]\n"
		"movups		xmm3,	[eax+48]\n"
		"movss		xmm7,	[ebx+12]\n"
		"unpcklps	xmm4,	xmm4\n"
		"unpcklps	xmm5,	xmm5\n"
		"unpcklps	xmm6,	xmm6\n"
		"unpcklps	xmm7,	xmm7\n"
		"movlhps		xmm4,	xmm4\n"
		"movlhps		xmm5,	xmm5\n"
		"movlhps		xmm6,	xmm6\n"
		"movlhps		xmm7,	xmm7\n"
		"mulps		xmm0,	xmm4\n"
		"mulps		xmm1,	xmm5\n"
		"mulps		xmm2,	xmm6\n"
		"mulps		xmm3,	xmm7\n"
		"addps		xmm0,	xmm1\n"
		"addps		xmm0,	xmm2\n"
		"addps		xmm0,	xmm3\n"
		"mov			eax,	[vout]\n"
		"movups		[eax],	xmm0\n"
	);
#else
	for( int x=0; x<4; x++ ){
		vout[x] = 0;
		for( int y=0; y<4; y++ ) vout[x] += mat[(y<<2)+x]*vin[y];
	}
#endif
	return	vout;
}

/*
FLOAT *MatrixMulMatrix4( FLOAT *mat1, FLOAT *mat2, FLOAT *mout ){
	int	x, y;
	for(x=0; x<4; x++){
		vout[x] = 0;
		for(y=0; y<4; y++)	vout[x] += mat[(y<<2)+x]*vin[y];
	}
	return	vout;
}
*/

FLOAT VectorLength(float *v){
#ifdef USESIMD
	float	res;
	asm(".intel_syntax noprefix\n"
		"mov			eax,	[v]\n"
		"movups		xmm0,	[eax]\n"
		"mulps		xmm0,	xmm0\n"
		"movhlps		xmm4,	xmm0\n"
		"unpcklps	xmm0,	xmm0\n"
		"movhlps		xmm1,	xmm0\n"
		"addss		xmm0,	xmm1\n"
		"addss		xmm0,	xmm4\n"
		"sqrtss		xmm0,	xmm0\n"
		"movss		[res],	xmm0\n"
	);
	return	res;
#else
	return  (FLOAT)sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
#endif
}

FLOAT VectorDot( float *v1, float *v2 ){
#ifdef USESIMD
	float	res;
	asm(".intel_syntax noprefix\n"
		"mov			eax,	[v1]\n"
		"mov			ebx,	[v2]\n"
		"movups		xmm0,	[eax]\n"
		"movups		xmm1,	[ebx]\n"
		"mulps		xmm0,	xmm1\n"
		"movhlps		xmm4,	xmm0\n"
		"unpcklps	xmm0,	xmm0\n"
		"movhlps		xmm1,	xmm0\n"
		"addss		xmm0,	xmm1\n"
		"addss		xmm0,	xmm4\n"
		"movss		[res],	xmm0\n"
	);
	return	res;
#else
	return	v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2];
#endif
}

FLOAT Vector4Dot( float *v1, float *v2 ){
#ifdef USESIMD
	float	res;
	asm(".intel_syntax noprefix\n"
		"mov		eax,	[v1]\n"
		"mov		ebx,	[v2]\n"
		"movups	xmm0,	[eax]\n"
		"movups	xmm1,	[ebx]\n"
		"mulps	xmm0,	xmm1\n"
		"haddps	xmm0,	xmm0\n"
		"haddps	xmm0,	xmm0\n"
		"movss	[res],	xmm0\n"
	);
	return	res;
#else
	return	v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2]+v1[3]*v2[3];
#endif
}

FLOAT *VectorNorm( float *v ){
#ifdef USESIMD
	asm(".intel_syntax noprefix\n"
		"mov			eax,	[v]\n"
		"movups		xmm0,	[eax]\n"
		"movaps		xmm2,	xmm0\n"
		"mulps		xmm0,	xmm0\n"
		"movhlps		xmm4,	xmm0\n"
		"unpcklps	xmm0,	xmm0\n"
		"movhlps		xmm1,	xmm0\n"
		"addss		xmm0,	xmm1\n"
		"addss		xmm0,	xmm4\n"
		"rsqrtss		xmm0,	xmm0\n"
		"unpcklps	xmm0,	xmm0\n"
		"movlhps		xmm0,	xmm0\n"
		"mulps		xmm0,	xmm2\n"
		"movlps		[eax],	xmm0\n"
		"movhlps		xmm0,	xmm0\n"
		"movss		[eax+8],xmm0\n"
	);
	return	v;
#else
	FLOAT	len=1.0f/VectorLength(v);
	v[0] *= len;
	v[1] *= len;
	v[2] *= len;
	return	v;
#endif
}

FLOAT *VectorMul( float *vin, float rhs, float *vout ){
#ifdef USESIMD
	asm(".intel_syntax noprefix\n"
		"movss		xmm1,	rhs\n"
		"mov			eax,	vin\n"
		"unpcklps	xmm1,	xmm1\n"
		"movups		xmm0,	[eax]\n"
		"movlhps		xmm1,	xmm1\n"
		"mulps		xmm0,	xmm1\n"
		"mov			eax,	vout\n"
		"movlps		[eax],	xmm0\n"
		"movhlps		xmm0,	xmm0\n"
		"movss		[eax+8],xmm0\n"
	);
#else
	vout[0] = vin[0]*rhs;
	vout[1] = vin[1]*rhs;
	vout[2] = vin[2]*rhs;
#endif
	return	vout;
}

FLOAT *Vector4Mul( float *vin, float rhs, float *vout ){
#ifdef USESIMD
	asm(".intel_syntax noprefix\n"
		"movss		xmm1,	rhs\n"
		"mov			eax,	vin\n"
		"unpcklps	xmm1,	xmm1\n"
		"movups		xmm0,	[eax]\n"
		"movlhps		xmm1,	xmm1\n"
		"mulps		xmm0,	xmm1\n"
		"mov			eax,	vout\n"
		"movups		[eax],	xmm0\n"
	);
#else
	vout[0] = vin[0]*rhs;
	vout[1] = vin[1]*rhs;
	vout[2] = vin[2]*rhs;
	vout[3] = vin[3]*rhs;
#endif
	return	vout;
}

float *VectorCross( float *v1, float *v2, float *vOut ){
#ifdef USESIMD
	asm(".intel_syntax noprefix\n"
		"mov		ebx,	[v2]\n"
		"mov		eax,	[v1]\n"
		"mov		edx,	[vOut]\n"
		"movups	xmm3,	[ebx]\n"
		"movups	xmm0,	[eax]\n"
		"movaps	xmm1,	xmm3\n"
		"movaps	xmm2,	xmm0\n"
		"psrldq	xmm1,	4\n"
		"psrldq	xmm2,	4\n"
		"movhps	xmm1,	[ebx]\n"
		"movhps	xmm2,	[eax]\n"
		"mulps	xmm0,	xmm1\n"
		"mulps	xmm2,	xmm3\n"
		"subps	xmm0,	xmm2\n"
		"movss	[edx+8],xmm0\n"
		"psrldq	xmm0,	4\n"
		"movlps	[edx],	xmm0\n"
	);
#else
	vOut[0] = v1[1]*v2[2]-v1[2]*v2[1];
	vOut[1] = v1[2]*v2[0]-v1[0]*v2[2];
	vOut[2] = v1[0]*v2[1]-v1[1]*v2[0];
#endif
	return	vOut;
}

float *VectorCrossAdd( float *v1, float *v2, float *vOut ){
#ifdef USESIMD
	asm(".intel_syntax noprefix\n"
		"mov		ebx,	[v2]\n"
		"mov		eax,	[v1]\n"
		"mov		edx,	[vOut]\n"
		"movups	xmm3,	[ebx]\n"
		"movups	xmm0,	[eax]\n"
		"movups	xmm4,	[edx]\n"
		"movaps	xmm1,	xmm3\n"
		"movaps	xmm2,	xmm0\n"
		"psrldq	xmm1,	4\n"
		"psrldq	xmm2,	4\n"
		"movhps	xmm1,	[ebx]\n"
		"movhps	xmm2,	[eax]\n"
		"mulps	xmm0,	xmm1\n"
		"mulps	xmm2,	xmm3\n"
		"subps	xmm0,	xmm2\n"
		"movss	xmm1,	xmm0\n"
		"psrldq	xmm0,	4\n"
		"movlhps	xmm0,	xmm1\n"
		"addps	xmm0,	xmm4\n"
		"movss	[edx],	xmm0\n"
		"psrldq	xmm0,	4\n"
		"movlps	[edx+4],xmm0\n"
	);
#else
	vOut[0] = v1[1]*v2[2]-v1[2]*v2[1];
	vOut[1] = v1[2]*v2[0]-v1[0]*v2[2];
	vOut[2] = v1[0]*v2[1]-v1[1]*v2[0];
#endif
	return	vOut;
}

float *VectorSub( float *v1, float *v2, float *vOut ){
#ifdef USESIMD
	asm(".intel_syntax noprefix\n"
		"mov		eax,	v1\n"
		"mov		ebx,	v2\n"
		"mov		edx,	vOut\n"
		"movups	xmm0,	[eax]\n"
		"movups	xmm1,	[ebx]\n"
		"subps	xmm0,	xmm1\n"
		"movlps	[edx],	xmm0\n"
		"movhlps	xmm0,	xmm0\n"
		"movss	[edx+8],xmm0\n"
	);
#else
	vOut[0] = v1[0]-v2[0];
	vOut[1] = v1[1]-v2[1];
	vOut[2] = v1[2]-v2[2];
#endif
	return	vOut;
}

float *Vector4Inter( float *v1, float *v2, float s_factor, float *vout ){
#ifdef USESIMD
	float	one = 1;
	asm(".intel_syntax noprefix\n"
		"movss		xmm2,	s_factor\n"
		"movss		xmm3,	one\n"
		"unpcklps	xmm2,	xmm2\n"
		"unpcklps	xmm3,	xmm3\n"
		"movlhps		xmm2,	xmm2\n"
		"movlhps		xmm3,	xmm3\n"
		"mov			eax,	v1\n"
		"mov			ebx,	v2\n"
		"movups		xmm0,	[eax]\n"
		"movups		xmm1,	[ebx]\n"
		"subps		xmm3,	xmm2\n"
		"mulps		xmm0,	xmm2\n"
		"mulps		xmm1,	xmm3\n"
		"addps		xmm0,	xmm1\n"
		"mov			eax,	vout\n"
		"movups		[eax],	xmm0\n"
	);
#else
	vout[0] = v1[0]*s_factor+v2[0]*(1-s_factor);
	vout[1] = v1[1]*s_factor+v2[1]*(1-s_factor);
	vout[2] = v1[2]*s_factor+v2[2]*(1-s_factor);
	vout[3] = v1[3]*s_factor+v2[3]*(1-s_factor);
#endif
	return	vout;
}

float *VectorAdd( float *v1, float *v2, float *vOut ){
#ifdef USESIMD
	asm(".intel_syntax noprefix\n"
		"mov		eax,	v1\n"
		"mov		ebx,	v2\n"
		"mov		edx,	vOut\n"
		"movups	xmm0,	[eax]\n"
		"movups	xmm1,	[ebx]\n"
		"addps	xmm0,	xmm1\n"
		"movlps	[edx],	xmm0\n"
		"movhlps	xmm0,	xmm0\n"
		"movss	[edx+8],xmm0\n"
	);
#else
	vOut[0] = v1[0]+v2[0];
	vOut[1] = v1[1]+v2[1];
	vOut[2] = v1[2]+v2[2];
#endif
	return	vOut;
}

float *Vector4Add( float *v1, float v, float *vOut ){
#ifdef USESIMD
	asm(".intel_syntax noprefix\n"
		"mov			eax,	v1\n"
		"movss		xmm1,	v\n"
		"movups		xmm0,	[eax]\n"
		"unpcklps	xmm1,	xmm1\n"
		"movlhps		xmm1,	xmm1\n"
		"mov			eax,	vOut\n"
		"addps		xmm0,	xmm1\n"
		"movups		[eax],	xmm0\n"
	);
#else
	vOut[0] = v1[0]+v;
	vOut[1] = v1[1]+v;
	vOut[2] = v1[2]+v;
	vOut[3] = v1[3]+v;
#endif
	return	vOut;
}

// Adapted from MESA implementation of the GLU library
bool InvertMatrix( FLOAT *m, FLOAT *invOut)
{
	FLOAT inv[16], det;
	int i;

	inv[0] =   m[5]*m[10]*m[15] - m[5]*m[11]*m[14] - m[9]*m[6]*m[15]
	+ m[9]*m[7]*m[14] + m[13]*m[6]*m[11] - m[13]*m[7]*m[10];
	inv[4] =  -m[4]*m[10]*m[15] + m[4]*m[11]*m[14] + m[8]*m[6]*m[15]
	- m[8]*m[7]*m[14] - m[12]*m[6]*m[11] + m[12]*m[7]*m[10];
	inv[8] =   m[4]*m[9]*m[15] - m[4]*m[11]*m[13] - m[8]*m[5]*m[15]
	+ m[8]*m[7]*m[13] + m[12]*m[5]*m[11] - m[12]*m[7]*m[9];
	inv[12] = -m[4]*m[9]*m[14] + m[4]*m[10]*m[13] + m[8]*m[5]*m[14]
	- m[8]*m[6]*m[13] - m[12]*m[5]*m[10] + m[12]*m[6]*m[9];
	inv[1] =  -m[1]*m[10]*m[15] + m[1]*m[11]*m[14] + m[9]*m[2]*m[15]
	- m[9]*m[3]*m[14] - m[13]*m[2]*m[11] + m[13]*m[3]*m[10];
	inv[5] =   m[0]*m[10]*m[15] - m[0]*m[11]*m[14] - m[8]*m[2]*m[15]
	+ m[8]*m[3]*m[14] + m[12]*m[2]*m[11] - m[12]*m[3]*m[10];
	inv[9] =  -m[0]*m[9]*m[15] + m[0]*m[11]*m[13] + m[8]*m[1]*m[15]
	- m[8]*m[3]*m[13] - m[12]*m[1]*m[11] + m[12]*m[3]*m[9];
	inv[13] =  m[0]*m[9]*m[14] - m[0]*m[10]*m[13] - m[8]*m[1]*m[14]
	+ m[8]*m[2]*m[13] + m[12]*m[1]*m[10] - m[12]*m[2]*m[9];
	inv[2] =   m[1]*m[6]*m[15] - m[1]*m[7]*m[14] - m[5]*m[2]*m[15]
	+ m[5]*m[3]*m[14] + m[13]*m[2]*m[7] - m[13]*m[3]*m[6];
	inv[6] =  -m[0]*m[6]*m[15] + m[0]*m[7]*m[14] + m[4]*m[2]*m[15]
	- m[4]*m[3]*m[14] - m[12]*m[2]*m[7] + m[12]*m[3]*m[6];
	inv[10] =  m[0]*m[5]*m[15] - m[0]*m[7]*m[13] - m[4]*m[1]*m[15]
	+ m[4]*m[3]*m[13] + m[12]*m[1]*m[7] - m[12]*m[3]*m[5];
	inv[14] = -m[0]*m[5]*m[14] + m[0]*m[6]*m[13] + m[4]*m[1]*m[14]
	- m[4]*m[2]*m[13] - m[12]*m[1]*m[6] + m[12]*m[2]*m[5];
	inv[3] =  -m[1]*m[6]*m[11] + m[1]*m[7]*m[10] + m[5]*m[2]*m[11]
	- m[5]*m[3]*m[10] - m[9]*m[2]*m[7] + m[9]*m[3]*m[6];
	inv[7] =   m[0]*m[6]*m[11] - m[0]*m[7]*m[10] - m[4]*m[2]*m[11]
	+ m[4]*m[3]*m[10] + m[8]*m[2]*m[7] - m[8]*m[3]*m[6];
	inv[11] = -m[0]*m[5]*m[11] + m[0]*m[7]*m[9] + m[4]*m[1]*m[11]
	- m[4]*m[3]*m[9] - m[8]*m[1]*m[7] + m[8]*m[3]*m[5];
	inv[15] =  m[0]*m[5]*m[10] - m[0]*m[6]*m[9] - m[4]*m[1]*m[10]
	+ m[4]*m[2]*m[9] + m[8]*m[1]*m[6] - m[8]*m[2]*m[5];

	det = m[0]*inv[0] + m[1]*inv[4] + m[2]*inv[8] + m[3]*inv[12];
	if (det == 0) return false;
	det = (FLOAT)1.0 / det;
	for (i = 0; i < 16; i++) invOut[i] = inv[i] * det;

	return true;
}

float* autoCorr1( float *data_out, float *data_in, int max_shift, int Corr_window_size ){
	float	*dataEnd = &data_in[Corr_window_size];
	if( max_shift>Corr_window_size ){
		memset( data_out, 0, sizeof(float)*max_shift );
		max_shift = Corr_window_size;
	}
	for(int x=0; x<max_shift; x++ ){
		float	*data1	=	data_in;
		float	*data2	=	data1+x;
		float	sum		=	0;
		if( data2+8<=dataEnd ){
			asm volatile(".intel_syntax noprefix\n"
			"vxorps		ymm4,	ymm4,	ymm4\n"
			"sub		rcx,	rbx\n"
			"shr		rcx,	5\n"
"re01:\n"
			"vmovups	ymm0,	[rax]\n"
			"vmovups	ymm1,	[rbx]\n"
			"add		rax,	32\n"
			"vmulps		ymm0,	ymm0,	ymm1\n"
			"add		rbx,	32\n"
			"vaddps		ymm4,	ymm4,	ymm0\n"
			"dec		rcx\n"
			"jnz		re01\n"

			"vhaddps	ymm4,	ymm4,	ymm4\n"
			"vhaddps	ymm4,	ymm4,	ymm4\n"
			"vhaddps	ymm4,	ymm4,	ymm4\n"
			"vmovss  [rdx], xmm4\n"
			:"=a"(data1), "=b"(data2)
			:"a"(data1), "b"(data2), "c"(dataEnd), "d"(&sum)
			);
		}
		for( ; data2<dataEnd; data1++,data2++ )
			sum	+=	(*data1)*(*data2);

		CheckFloat(&sum);
		data_out[x] = (float)(sum/(Corr_window_size-x));
		CheckFloat(&data_out[x]);
	}
	return	data_out;
	//CheckFloat( data_out, max_shift );
}// Compute autocorrelation


FLOAT* autoCorr(float *data_out, float *data_in, int max_shift, int data_size){
	if (max_shift>data_size){
		memset(data_out, 0, sizeof(float)*max_shift);
		max_shift = data_size;
	}
	for (int x = 0; x<max_shift; x++){
		float	*data_in2 = &data_in[x];
		float	sum = 0;
		// Do not use the entire residue buffer of size (data_size - x) to avoid aliasing
		//register int	I = (x == 0 ? data_size : (data_size - x) / x * x);
		int	I = data_size - x;
		int	i = 0;
/*
		{// this block of AVX optimization code can be commented out if CPU/compiler does not support
			__m256 ymm0 = _mm256_setzero_ps();
			for (; i + 7 < I; i += 8){
				__m256 ymm1 = _mm256_loadu_ps(&data_in[i]);
				__m256 ymm2 = _mm256_loadu_ps(&data_in2[i]);
				__m256 ymm3 = _mm256_mul_ps(ymm1, ymm2);
				ymm0 = _mm256_add_ps(ymm0, ymm3);
			}
			float float8[8];
			__m256 ymm1 = _mm256_setzero_ps();
			ymm0 = _mm256_hadd_ps(ymm0, ymm1);
			ymm0 = _mm256_hadd_ps(ymm0, ymm1);
			_mm256_storeu_ps(float8, ymm0);
			sum = float8[0] + float8[4];
		}
*/
		for (; i<I; ++i)
			sum += data_in[i]*data_in2[i];

		data_out[x] = sum / I;
	}

	// normalize all values into 0 and 1 range
	normCorr(data_out, max_shift);

	return	data_out;
	//CheckFloat( data_out, max_shift );
}// Compute autocorrelation

float* ComputeNormalA( float* pVertex, float* pNormal, int width, int height ){
	// Negative width or height indicate wrap around
	if( height < 2 ) return pNormal;

	bool wrapX = false, wrapY = false;
	if( width<0 ){
		width = -width;
		wrapX = true;
	}
	if( height<0 ){
		height = -height;
		wrapY = true;
	}

	const int	line_size = width*3;
	const int	full_size = line_size*height;
	float	*pNorm = pNormal, *pVert = pVertex;
	float	v1[3], v2[3], v3[3], v4[3], v[3];

	// First line: begin
	VectorSub(pVert+line_size,pVert,v4);
	VectorSub(pVert+3,pVert,v1);
	VectorCross(v4,v1,pNorm);
	VectorNorm(pNorm);
	if( wrapX ){
		VectorSub(pVert+line_size-3,pVert,v3);
		VectorCross(v3,v4,v);
		VectorNorm(v);
		VectorAdd(pNorm,v,pNorm);
	}
	if( wrapY ){
		VectorSub(pVert-line_size+full_size,pVert,v2);
		VectorCross(v1,v2,v);
		VectorNorm(v);
		VectorAdd(pNorm,v,pNorm);
	}
	if( wrapX && wrapY ){
		VectorCross(v2,v3,v);
		VectorNorm(v);
		VectorAdd(pNorm,v,pNorm);
	}
	VectorNorm(pNorm);
	pNorm += 3;
	pVert += 3;

	// First line: middle part, 2 cross products
	for( int x=2; x<width; x++,pNorm+=3,pVert+=3 ){
		VectorSub(pVert+3,pVert,v1);
		VectorSub(pVert-3,pVert,v3);
		VectorSub(pVert+line_size,pVert,v4);
		VectorCross(v3,v4,pNorm);
		VectorNorm(pNorm);
		VectorCross(v4,v1,v);
		VectorNorm(v);
		VectorAdd(pNorm,v,pNorm);
		if( wrapY ){
			VectorSub(pVert-line_size+full_size,pVert,v2);
			VectorCross(v1,v2,v);
			VectorNorm(v);
			VectorAdd(pNorm,v,pNorm);
			VectorCross(v2,v3,v);
			VectorNorm(v);
			VectorAdd(pNorm,v,pNorm);
		}
		VectorNorm(pNorm);
	}

	// First line: end
	VectorSub(pVert-3,pVert,v3);
	VectorSub(pVert+line_size,pVert,v4);
	VectorCross(v3,v4,pNorm);
	VectorNorm(pNorm);
	if( wrapX ){
		VectorSub(pVert+3-line_size,pVert,v1);
		VectorCross(v4,v1,v);
		VectorNorm(v);
		VectorAdd(pNorm,v,pNorm);
	}
	if( wrapY ){
		VectorSub(pVert-line_size+full_size,pVert,v2);
		VectorCross(v2,v3,v);
		VectorNorm(v);
		VectorAdd(pNorm,v,pNorm);
	}
	if( wrapX && wrapY ){
		VectorCross(v1,v2,v);
		VectorNorm(v);
		VectorAdd(pNorm,v,pNorm);
	}
	VectorNorm(pNorm);
	pNorm += 3;
	pVert += 3;


	// Middle lines: 4 cross products
	for( int y=2; y<height; y++ ){
		VectorSub(pVert+3,pVert,v1);
		VectorSub(pVert-line_size,pVert,v2);
		VectorSub(pVert+line_size,pVert,v4);
		VectorCross(v1,v2,pNorm);
		VectorNorm(pNorm);
		VectorCross(v4,v1,v);
		VectorNorm(v);
		VectorAdd(pNorm,v,pNorm);
		if( wrapX ){
			VectorSub(pVert-3+line_size,pVert,v3);
			VectorCross(v2,v3,v);
			VectorNorm(v);
			VectorAdd(pNorm,v,pNorm);
			VectorCross(v3,v4,v);
			VectorNorm(v);
			VectorAdd(pNorm,v,pNorm);
		}
		VectorNorm(pNorm);
		pNorm += 3;
		pVert += 3;

		for( int x=2; x<width; x++,pNorm+=3,pVert+=3 ){
			VectorSub(pVert+3,pVert,v1);
			VectorSub(pVert-line_size,pVert,v2);
			VectorSub(pVert-3,pVert,v3);
			VectorSub(pVert+line_size,pVert,v4);
			VectorCross(v1,v2,pNorm);
			VectorNorm(pNorm);

			VectorCross(v2,v3,v);
			VectorNorm(v);
			VectorAdd(pNorm,v,pNorm);

			VectorCross(v3,v4,v);
			VectorNorm(v);
			VectorAdd(pNorm,v,pNorm);

			VectorCross(v4,v1,v);
			VectorNorm(v);
			VectorAdd(pNorm,v,pNorm);

			VectorNorm(pNorm);
		}

		VectorSub(pVert-line_size,pVert,v2);
		VectorSub(pVert-3,pVert,v3);
		VectorSub(pVert+line_size,pVert,v4);
		VectorCross(v2,v3,pNorm);
		VectorNorm(pNorm);
		VectorCross(v3,v4,v);
		VectorNorm(v);
		VectorAdd(pNorm,v,pNorm);
		if( wrapX ){
			VectorSub(pVert+3-line_size,pVert,v1);
			VectorCross(v1,v2,v);
			VectorNorm(v);
			VectorAdd(pNorm,v,pNorm);
			VectorCross(v4,v1,v);
			VectorNorm(v);
			VectorAdd(pNorm,v,pNorm);
		}
		VectorNorm(pNorm);
		pNorm += 3;
		pVert += 3;
	}


	// Last line: begin
	VectorSub(pVert+3,pVert,v1);
	VectorSub(pVert-line_size,pVert,v2);
	VectorCross(v1,v2,pNorm);
	VectorNorm(pNorm);
	if( wrapX ){
		VectorSub(pVert-3+line_size,pVert,v3);
		VectorCross(v2,v3,v);
		VectorNorm(v);
		VectorAdd(pNorm,v,pNorm);
	}
	if( wrapY ){
		VectorSub(pVert+line_size-full_size,pVert,v4);
		VectorCross(v4,v1,v);
		VectorNorm(v);
		VectorAdd(pNorm,v,pNorm);
	}
	if( wrapX && wrapY ){
		VectorCross(v3,v4,v);
		VectorNorm(v);
		VectorAdd(pNorm,v,pNorm);
	}
	VectorNorm(pNorm);
	pNorm += 3;
	pVert += 3;

	// Last line: middle part, 2 cross products
	for( int x=2; x<width; x++,pNorm+=3,pVert+=3 ){
		VectorSub(pVert+3,pVert,v1);
		VectorSub(pVert-line_size,pVert,v2);
		VectorSub(pVert-3,pVert,v3);
		VectorCross(v1,v2,pNorm);
		VectorNorm(pNorm);
		VectorCross(v2,v3,v);
		VectorNorm(v);
		VectorAdd(pNorm,v,pNorm);
		if( wrapY ){
			VectorSub(pVert+line_size-full_size,pVert,v4);
			VectorCross(v3,v4,v);
			VectorNorm(v);
			VectorAdd(pNorm,v,pNorm);
			VectorCross(v4,v1,v);
			VectorNorm(v);
			VectorAdd(pNorm,v,pNorm);
		}
		VectorNorm(pNorm);
	}

	// Last line: end
	VectorSub(pVert-line_size,pVert,v2);
	VectorSub(pVert-3,pVert,v3);
	VectorCross(v2,v3,pNorm);
	VectorNorm(pNorm);
	if( wrapX ){
		VectorSub(pVert+3-line_size,pVert,v1);
		VectorCross(v1,v2,v);
		VectorNorm(v);
		VectorAdd(pNorm,v,pNorm);
	}
	if( wrapY ){
		VectorSub(pVert+line_size-full_size,pVert,v4);
		VectorCross(v3,v4,v);
		VectorNorm(v);
		VectorAdd(pNorm,v,pNorm);
	}
	if( wrapX && wrapY ){
		VectorCross(v4,v1,v);
		VectorNorm(v);
		VectorAdd(pNorm,v,pNorm);
	}
	VectorNorm(pNorm);

	return	pNormal;
}

float* ComputeNormalB( float* pVertex, float* pNormal, int width, int height ){
	// Negative width or height indicate wrap around
	if( height < 2 ) return pNormal;

	bool wrapX = false, wrapY = false;
	if( width<0 ){
		width = -width;
		wrapX = true;
	}
	if( height<0 ){
		height = -height;
		wrapY = true;
	}

	const int	line_size = width*3;
	const int	full_size = line_size*height;
	float	*pNorm = pNormal, *pVert = pVertex;
	float	v1[3], v2[3], v3[3], v4[3];

	// First line: begin
	VectorSub(pVert+line_size,pVert,v4);
	VectorSub(pVert+3,pVert,v1);
	VectorCross(v4,v1,pNorm);
	if( wrapX ){
		VectorSub(pVert+line_size-3,pVert,v3);
		VectorCrossAdd(v3,v4,pNorm);
	}
	if( wrapY ){
		VectorSub(pVert-line_size+full_size,pVert,v2);
		VectorCrossAdd(v1,v2,pNorm);
	}
	if( wrapX && wrapY ){
		VectorCrossAdd(v2,v3,pNorm);
	}
	VectorNorm(pNorm);
	pNorm += 3;
	pVert += 3;

	// First line: middle part, 2 cross products
	for( int x=2; x<width; x++,pNorm+=3,pVert+=3 ){
		VectorSub(pVert+3,pVert,v1);
		VectorSub(pVert-3,pVert,v3);
		VectorSub(pVert+line_size,pVert,v4);
		VectorCross(v3,v4,pNorm);
		VectorCrossAdd(v4,v1,pNorm);
		if( wrapY ){
			VectorSub(pVert-line_size+full_size,pVert,v2);
			VectorCrossAdd(v1,v2,pNorm);
			VectorCrossAdd(v2,v3,pNorm);
		}
		VectorNorm(pNorm);
	}

	// First line: end
	VectorSub(pVert-3,pVert,v3);
	VectorSub(pVert+line_size,pVert,v4);
	VectorCross(v3,v4,pNorm);
	if( wrapX ){
		VectorSub(pVert+3-line_size,pVert,v1);
		VectorCrossAdd(v4,v1,pNorm);
	}
	if( wrapY ){
		VectorSub(pVert-line_size+full_size,pVert,v2);
		VectorCrossAdd(v2,v3,pNorm);
	}
	if( wrapX && wrapY ){
		VectorCrossAdd(v1,v2,pNorm);
	}
	VectorNorm(pNorm);
	pNorm += 3;
	pVert += 3;


	// Middle lines: 4 cross products
	for( int y=2; y<height; y++ ){
		VectorSub(pVert+3,pVert,v1);
		VectorSub(pVert-line_size,pVert,v2);
		VectorSub(pVert+line_size,pVert,v4);
		VectorCross(v1,v2,pNorm);
		VectorCrossAdd(v4,v1,pNorm);
		if( wrapX ){
			VectorSub(pVert-3+line_size,pVert,v3);
			VectorCrossAdd(v2,v3,pNorm);
			VectorCrossAdd(v3,v4,pNorm);
		}
		VectorNorm(pNorm);
		pNorm += 3;
		pVert += 3;

		for( int x=2; x<width; x++,pNorm+=3,pVert+=3 ){
			VectorSub(pVert+3,pVert,v1);
			VectorSub(pVert-line_size,pVert,v2);
			VectorSub(pVert-3,pVert,v3);
			VectorSub(pVert+line_size,pVert,v4);
			VectorCross(v1,v2,pNorm);
			VectorCrossAdd(v2,v3,pNorm);
			VectorCrossAdd(v3,v4,pNorm);
			VectorCrossAdd(v4,v1,pNorm);
			VectorNorm(pNorm);
		}

		VectorSub(pVert-line_size,pVert,v2);
		VectorSub(pVert-3,pVert,v3);
		VectorSub(pVert+line_size,pVert,v4);
		VectorCross(v2,v3,pNorm);
		VectorCrossAdd(v3,v4,pNorm);
		if( wrapX ){
			VectorSub(pVert+3-line_size,pVert,v1);
			VectorCrossAdd(v1,v2,pNorm);
			VectorCrossAdd(v4,v1,pNorm);
		}
		VectorNorm(pNorm);
		pNorm += 3;
		pVert += 3;
	}


	// Last line: begin
	VectorSub(pVert+3,pVert,v1);
	VectorSub(pVert-line_size,pVert,v2);
	VectorCross(v1,v2,pNorm);
	if( wrapX ){
		VectorSub(pVert-3+line_size,pVert,v3);
		VectorCrossAdd(v2,v3,pNorm);
	}
	if( wrapY ){
		VectorSub(pVert+line_size-full_size,pVert,v4);
		VectorCrossAdd(v4,v1,pNorm);
	}
	if( wrapX && wrapY ){
		VectorCrossAdd(v3,v4,pNorm);
	}
	VectorNorm(pNorm);
	pNorm += 3;
	pVert += 3;

	// Last line: middle part, 2 cross products
	for( int x=2; x<width; x++,pNorm+=3,pVert+=3 ){
		VectorSub(pVert+3,pVert,v1);
		VectorSub(pVert-line_size,pVert,v2);
		VectorSub(pVert-3,pVert,v3);
		VectorCross(v1,v2,pNorm);
		VectorCrossAdd(v2,v3,pNorm);
		if( wrapY ){
			VectorSub(pVert+line_size-full_size,pVert,v4);
			VectorCrossAdd(v3,v4,pNorm);
			VectorCrossAdd(v4,v1,pNorm);
		}
		VectorNorm(pNorm);
	}

	// Last line: end
	VectorSub(pVert-line_size,pVert,v2);
	VectorSub(pVert-3,pVert,v3);
	VectorCross(v2,v3,pNorm);
	if( wrapX ){
		VectorSub(pVert+3-line_size,pVert,v1);
		VectorCrossAdd(v1,v2,pNorm);
	}
	if( wrapY ){
		VectorSub(pVert+line_size-full_size,pVert,v4);
		VectorCrossAdd(v3,v4,pNorm);
	}
	if( wrapX && wrapY ){
		VectorCrossAdd(v4,v1,pNorm);
	}
	VectorNorm(pNorm);

	return	pNormal;
}

float* (*ComputeNormal)( float* pVertex, float* pNormal, int width, int height )=ComputeNormalB;
