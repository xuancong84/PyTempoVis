/////////////////////////////////////////////////////////////////////////////
//
// TempoVis.h : Declaration of the CTempoVis
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __TEMPOVIS_H_
#define __TEMPOVIS_H_

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <inttypes.h>
#include "parameters.h"
#include "Visualization.h"


extern "C" BYTE	gm_debug, gm_showFPS, gm_fullScreen, gm_pad;	// global modes
extern char		database_filename[];
extern char		waveform_filename[];
extern char		*all_status[];
extern char		*g_status;
extern char		*g_error;

extern int		n_est, b_est, n_threads;
extern float	*est_spec[], est_fact[], est_fact2[];
extern void Realft (float*);
extern FLOAT getMax( FLOAT *data, int size );
extern FLOAT getMin( FLOAT *data, int size );

extern const FLOAT BELT_OFF;

extern int total_added;

extern float	space_attn[];
extern float	zero_vector[];
extern float	ones_vector[];
extern float	zero_4vector[];

extern FLOAT	l_ambient[];
extern FLOAT	l_diffuse[];
extern FLOAT	l_specular[];
extern FLOAT	m_ambient[];
extern FLOAT	m_diffuse[];
extern FLOAT	m_specular[];
extern FLOAT	m_emissive[];

extern const int	nTotalParams;
extern const int	nTotalBufs;

enum PlayerState {
	stop_state	= 0,
	pause_state	= 1,
	play_state	= 2
};

// Global functions
extern bool	CheckFloat( float *f, int N=1 );
extern void get_viewport_size(int *Width, int *Height);
extern float* autoCorr( float *data_out, float *data_in, int max_shift, int Corr_window_size );
extern float* (*ComputeNormal)( float* pVertex, float* pNormal, int width, int height );
extern FLOAT *MatrixMulVector4( FLOAT *mat, FLOAT *vin, FLOAT *vout );
extern FLOAT VectorLength(float *v);
extern FLOAT VectorDot( float *v1, float *v2 );
extern FLOAT Vector4Dot( float *v1, float *v2 );
extern FLOAT *VectorNorm( float *v );
extern FLOAT *VectorMul( float *vin, float rhs, float *vout );
extern FLOAT *Vector4Mul( float *vin, float rhs, float *vout );
extern float *VectorCross( float *v1, float *v2, float *vOut );
extern float *VectorCrossAdd( float *v1, float *v2, float *vOut );
extern float *VectorSub( float *v1, float *v2, float *vOut );
extern float *Vector4Inter( float *v1, float *v2, float s_factor, float *vout );
extern float *VectorAdd( float *v1, float *v2, float *vOut );
extern float *Vector4Add( float *v1, float v, float *vOut );
extern bool InvertMatrix( FLOAT *m, FLOAT *invOut);
extern float* ComputeNormalA( float* pVertex, float* pNormal, int width, int height );
extern float* ComputeNormalB( float* pVertex, float* pNormal, int width, int height );
extern int glXYPrintf(int X, int Y, WORD align, void *font, char* fmt, ...);
extern int glXYPrintf(int X, int Y, WORD align, char* fmt, ...);
extern void normCorr( FLOAT *data, int size );
extern FLOAT calcSpecWeightByMaxPeakHeight( FLOAT *data, int size );
extern void getPeakSpectrum( FLOAT *dst, FLOAT *src, int size );
extern FLOAT calcPeakHeight( FLOAT *data, int size, FLOAT posi );
extern int findMax( FLOAT *data, int size );
extern int findPeakPosi( FLOAT *data, int size, int posi );
extern void addCorr( FLOAT *dst, FLOAT *src, int size, FLOAT factor );
extern FLOAT interPeakPosi( FLOAT *middle );
extern int	getMeter( FLOAT pri_tempo, FLOAT *spec, int size, int *bAmbiguous=NULL );
extern float ComputeTempo( float *data, int size, int sample_rate );
extern int	adjustTempo( float pri_tempo, int min_tempo, int max_tempo,
						FLOAT *TempoSpec, FLOAT *PeakSpec,
						FLOAT *Window2, FLOAT *Window3 );

#endif //__TEMPOVIS_H_
