#define _ISOC99_SOURCE
#define __STDC_WANT_DEC_FP__
#define	__DRAWFAST 1

#include	<vector>
#include	<assert.h>
#include	<math.h>
#include	<iostream>
#include	<string>
#include	<fstream>
#include	<float.h>
#include	<time.h>
#include	<queue>
#include	<map>
#include	<cmath>
#include	<unistd.h>
#include	<thread>

#include	"TempoVis.h"
#include	"Visualization.h"

using namespace std;

//#define	NOGRAPHICS	1

Visualization	*g_pVisual = NULL;
BYTE		gm_debug, gm_showFPS, gm_fullScreen, gm_pad;	// global modes
int			g_nSamplesPerSec=44100;
char		database_filename[]="TempoVis.txt";
char		waveform_filename[]="TempoVis.pcm";
char		*all_status[]={
	"",
	"Decompressing MP3...",
	"Computing Tempo...",
	"Computing Tempo...Done!",
};
char	*g_status = all_status[0];
char	*g_error = NULL;
void	*g_font = GLUT_BITMAP_9_BY_15;
FLOAT	CameraSpeed = DefaultCameraSpeed;
FLOAT	SceneRotSpeed = DefaultSceneRotSpeed;
FLOAT	RecordLatency = 0.0f;	// in seconds

const 	FLOAT BELT_OFF  = (FLOAT)M_PI/BELTCIRCUMSIZE;

int 	total_added = 0;

float	space_attn[3]	= { 0.0f, 0.2f, 0 };
float	zero_vector[4]	= { 0, 0, 0, 1 };
float	ones_vector[4]	= { 1, 1, 1, 1 };
float	zero_4vector[4]	= { 0, 0, 0, 0 };

FLOAT	l_ambient[4]	= {0.4f,0.4f,0.4f,1.0f};
FLOAT	l_diffuse[4]	= {0.2f,0.2f,0.2f,1.0f};
FLOAT	l_specular[4]	= {1.5f,1.5f,1.5f,1.0f};
FLOAT	m_ambient[4]	= {0.4f,0.4f,0.4f,1.0f};
FLOAT	m_diffuse[4]	= {0.2f,0.2f,0.2f,1.0f};
FLOAT	m_specular[4]	= {1.0f,1.0f,1.0f,1.0f};
FLOAT	m_emissive[4]	= {0.01f,0.01f,0.01f,1.0f};

bool	CheckFloat( float *f, int N ){
	return true;
	for( int x=0; x<N; x++ )
		if(!(f[x]>-1000000 && f[x]<1000000)){
			asm(".intel_syntax noprefix\n"
				"int 3");
			return false;
		}
	return	true;
}

const char *extr_fn( const char* str ){
	const char *p = strrchr(str, '\\');
	return	p?(p+1):str;
}

void	Record( int state, bool bClear );
DWORD	PostEstThreadFunc( void *param );
DWORD	PreEstThreadFunc( void *param );

int		n_est=0, b_est=0, n_threads=0;
enum	PlayerState last_state = stop_state;
vector <vector <FLOAT>> est_spec;
vector <FLOAT>	est_fact, est_fact2;

// Core export functions
extern "C" void CreateTempoVis(){
	if(!g_pVisual){
		srand( clock() );
		DWORD *bmpData = new DWORD [SPRITE_TEX_SIZE*SPRITE_TEX_SIZE];
		float mid = (SPRITE_TEX_SIZE-1)/2.0f;
		for(int x=0; x<SPRITE_TEX_SIZE; ++x) for(int y=0; y<SPRITE_TEX_SIZE; ++y){
			BYTE b = (BYTE)(exp(-((x-mid)*(x-mid)+(y-mid)*(y-mid)-0.25)/64.0)*255.0+0.5);
			DWORD d = (b<<24)|(b<<16)|(b<<8)|b;
			bmpData[y*SPRITE_TEX_SIZE+x] = d;
		}
		g_pVisual = new Visualization( (BYTE*)bmpData );
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
		glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
		glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	}
}

extern "C" void DrawFrame( TimedLevel *pLevels ){

	if( !g_pVisual || pLevels->state != play_state ) return;

	// Start drawing
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Draw quad strip FFT
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
	g_pVisual->DrawAll( pLevels, g_pVisual->addData( pLevels, true ) );
	glPopClientAttrib();
	glPopAttrib();

	glColor4ub(0xff,0xff,0xff,0xff);

	if( gm_debug>=1 ){
		// Show FPS and debug info
		glXYPrintf(0,16,(WORD)0,"%c FPS=%#.4g DPS=%.2f binSize=%d latency=%.2f FFTcutOff=%.1f/%d %s", (char)(gm_fullScreen?'#':'^'),
			g_pVisual->FPS, g_pVisual->Data_rate, g_pVisual->bins_per_bin, RecordLatency, g_pVisual->freq_bin_cutoff, FFTSIZE, g_status);

		glXYPrintf(0,32,(WORD)0,"0 1 2 3 4 5 6 7 8 :SET_BIN_SIZE");

		char ind[20]="";
		if( g_pVisual->freq_bin_cutoff < 0 ){
			int	y = g_pVisual->bins_per_bin;
			for( int x=0; x<y; x++ ) sprintf( &ind[strlen(ind)], "%d ", x );
		}
		glXYPrintf(0,32,(WORD)0,strcat(ind,"_"));
	}
	if( gm_debug>=2 ){
		glXYPrintf(0,48,(WORD)0,"Timestamp=%" PRId64 ", State=%d, Total_added=%d, Tempo=%d/%f [%f]",
			pLevels->timeStamp, last_state, total_added, g_pVisual->last_phase_index,
			g_pVisual->last_tempo_index+g_pVisual->tempo_period_frac, g_pVisual->preset_tempo);
		glXYPrintf(0,64,(WORD)0,"phase_enhance_tempo=%f, tempo_enhance_phase=%f",
			g_pVisual->tempo_enhance_factor, g_pVisual->phase_enhance_factor);
		glXYPrintf(0,80,(WORD)0,"freq_bin_cutoff=%f, power_level=%f",
			g_pVisual->freq_bin_cutoff, g_pVisual->power_level);
		if( g_error ) glXYPrintf(0, 80, (WORD)0, "%s", g_error);
	}
}

bool MK_LBUTTON=false, MK_MBUTTON=false;
int last_move_x, last_move_y, last_mid_move_x, last_mid_move_y;
extern "C" void onMouseMove( int32_t x, int32_t y ){
	int Width, Height;
	get_viewport_size(&Width, &Height);
	if (MK_LBUTTON){
		FLOAT x_delta = x - last_move_x;
		FLOAT y_delta = y - last_move_y;
		g_pVisual->global_H_rot -= x_delta / Width;
		g_pVisual->global_V_rot += y_delta / Height;
		last_move_x = x;
		last_move_y = y;
		if (x_delta!=0||y_delta!=0)
			SceneRotSpeed = CameraSpeed = 0;
	}else if (MK_MBUTTON){
		FLOAT x_delta = x - last_mid_move_x;
		FLOAT y_delta = y - last_mid_move_y;
		g_pVisual->global_H_shift += x_delta / Width;
		g_pVisual->global_V_shift -= y_delta / Height;
		last_mid_move_x = x;
		last_mid_move_y = y;
		if (x_delta != 0 || y_delta != 0)
			SceneRotSpeed = CameraSpeed = 0;
	}
}

void glGetWindowRect(int &x, int &y, int &width, int &height){
	x = glutGet(GLUT_WINDOW_X);
	y = glutGet(GLUT_WINDOW_Y);
	width = glutGet(GLUT_WINDOW_WIDTH);
	height = glutGet(GLUT_WINDOW_HEIGHT);
}

void glGetScreenSize(int &width, int &height){
	width = glutGet(GLUT_SCREEN_WIDTH);
	height = glutGet(GLUT_SCREEN_HEIGHT);
}

extern "C" void toggleFullScreen(){
	static int last_win_x=20, last_win_y=20, last_win_width=640, last_win_height=480;
	int win_x, win_y, win_width, win_height;
	int scr_width, scr_height;
	glGetWindowRect(win_x, win_y, win_width, win_height);
	glGetScreenSize(scr_width, scr_height);
	if (!win_x && !win_y && win_width==scr_width && win_height==scr_height){	// if already fullscreen
		glutReshapeWindow(last_win_width, last_win_height);
		glutPositionWindow(last_win_x, last_win_y);
		glutPostRedisplay();
		gm_fullScreen = false;
	} else {				// if not fullscreen go fullscreen
		glutFullScreen();
		last_win_x = win_x;
		last_win_y = win_y;
		last_win_width = win_width;
		last_win_height = win_height;
		gm_fullScreen = true;
	}
}

extern "C" void onMouseButton(int32_t event, int32_t x, int32_t y){
	if (!g_pVisual)
		return;
	static int last_x, last_y, last_mid_x, last_mid_y;
	switch (event){
	case 0: // WM_LBUTTONUP
		if (last_x == x && last_y == y){
			bool bToggle = true;
			if (gm_showFPS || gm_debug ){
				WORD	Xpos = x, Ypos = y;
				if (Xpos < 9 && Ypos < 15)
					toggleFullScreen();
				else if (Ypos <= 30 && Ypos >= 15){
					int cx = glutBitmapLength(g_font, (const BYTE*)"0 1 2 3 4 5 6 7 8 ");
					int	bin_size = (int)(Xpos*9.0 / cx + 0.5);
					if (!bin_size){
						g_pVisual->freq_bin_cutoff = (float)g_pVisual->bins_per_bin * 128;
						bToggle = false;
					}
					else if (bin_size <= 8){
						g_pVisual->freq_bin_cutoff = -1;
						g_pVisual->bins_per_bin = bin_size;
						bToggle = false;
					}
				}
			}
			if (bToggle){
				gm_showFPS = !gm_showFPS;
				ComputeNormal = gm_showFPS ? ComputeNormalA : ComputeNormalB;
			}
		}
		MK_LBUTTON = false;
		break;
	case 1:	// WM_LBUTTONDOWN
		last_move_x = last_x = x;
		last_move_y = last_y = y;
		MK_LBUTTON = true;
		break;
	case 2:	// WM_MBUTTONUP
		if (last_mid_x == x && last_mid_y == y){
			g_pVisual->global_H_shift = 0;
			g_pVisual->global_V_shift = 0;
			g_pVisual->global_H_rot = 0;
			g_pVisual->global_V_rot = 0;
			CameraSpeed = DefaultCameraSpeed;
			SceneRotSpeed = DefaultSceneRotSpeed;
		}
		MK_MBUTTON = false;
		break;
	case 3:	// WM_MBUTTONDOWN
		last_mid_move_x = last_mid_x = x;
		last_mid_move_y = last_mid_y = y;
		MK_MBUTTON = true;
		break;
	}
}

void TempoThreadFunc( FLOAT *fdata, int fsize, int sr ){
	g_status = all_status[2];
	float	tempo = ComputeTempo( fdata, fsize, sr, g_pVisual->pri_tempoIndex );
	g_status = all_status[3];
	if(g_pVisual){
		g_pVisual->preset_tempo = abs(tempo);
		g_pVisual->preset_meter = tempo>=0?2:3;
	}
}

thread	*pTempoThread = NULL;
FLOAT	*pTempoData = NULL;
int		szTempoData = 0;
extern "C" bool	CreateTempoThread( FLOAT *fdata, int32_t fsize, int32_t sr ){
	if(pTempoThread){	// close the thread object if possible
		if(!pTempoThread->joinable())
			return false;
		pTempoThread->join();
		delete pTempoThread;
		pTempoThread = NULL;
	}
	if(pTempoData && szTempoData!=fsize){	// release the buffer if size has changed
		delete [] pTempoData;
		pTempoData = NULL;
	}
	if(!pTempoData){	// allocate audio buffer for tempo estimation
		pTempoData = new FLOAT [fsize];
		szTempoData = fsize;
	}
	// create tempo estimation thread
	memcpy(pTempoData, fdata, fsize*sizeof(FLOAT));
	pTempoThread = new thread(TempoThreadFunc, pTempoData, fsize, sr);
	return true;
}// Thread for computing and saving tempo values
