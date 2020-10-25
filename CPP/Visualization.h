
#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include <stdint.h>

#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <GL/glut.h>
#endif


typedef struct tagTimedLevel{
	float *frequency[2];
	float *waveform[2];
	int64_t timeStamp;
	int32_t state;
} TimedLevel;

class	Camera{
public:
	FLOAT	eyev[3], centerv[3], upv[3], rightv[3];
	FLOAT	viewAngle;
	FLOAT	viewMat[16], projMat[16];

	Camera();
	~Camera();

	void reset();
	void setCamera();
	void updateCamera( FLOAT *eyev, FLOAT *objv, FLOAT *topv );
	void shiftUp( FLOAT dist );
};

class	LoopBuffer{
	// All variables set to public for performance
public:
	int		N_total_frames;
	int		N_draw_frames;
	int		N_valid_frames;
	int		frame_size;
	int		current_frame;		// inclusive
	int		draw_size;
	int		data_size;
	int		status;				// 0:playing or stopped, 1:paused, 2:unpaused but buffering
	FLOAT	time_factor;
	FLOAT	time_stamp;
	char	*data;				// main data buffer
	char	*last_frame_data;

	LoopBuffer(int _N_total_frames, int _N_draw_frames, int _frame_size, FLOAT _fps);
	~LoopBuffer();

	// Functions
	int	AddFrame(FLOAT timeStamp, FLOAT *currFrame);
	int AddFrame(FLOAT timeStamp, FLOAT val);
	void	ResizeBuffer(int _N_total_frames, int _N_draw_frames, int _frame_size, FLOAT _fps);
	void	Reset(FLOAT _time_stamp);
	FLOAT*	getCurrentPtrFront();
	FLOAT*	getCurrentPtrBack();
};

class	Visualization{
public:
	// Normal parameters
	int		N_total_frames;
	int		N_draw_frames;
	int		bins_per_bin;
	int		verts_per_line;
	int		**multidraw_first;
	int		*multidraw_count;
	int		belt_lastParity;
	int		belt_prim_count;
	int		fft_prim_count;
	int		sprite_count;
	int		tempo_state;									// number of spin stars to be drawn, 0: don't draw any (tempo not reliable)
	int		last_tempo_index;
	int		last_pri_tempo_index;
	int		last_phase_index;
	int		last_phase_posi;
	GLuint	texture_names[1];
	FLOAT	global_H_rot, global_V_rot;
	FLOAT	global_H_shift, global_V_shift;
	FLOAT	last_tempo_change_time, last_tempo_change_time2;
	FLOAT	last_tempo_est_time;
	FLOAT	freq_bin_cutoff;								// for extending freq. bins
	FLOAT	preset_tempo;									// >0: valid, =0: variable tempo, <0: unknown
	int		preset_meter;									// to distinguish 4/4 vs 3/3 meters
	int		tempoMeter;										// to distinguish 4/4 vs 3/3 meters
	FLOAT	Data_rate;
	DWORD	Data_cnt;
	DWORD	FPS_lastIntv, FPS_lastTick, FPS_currIntv;
	FLOAT	target_eye[3];									// target eye position coordinate
	FLOAT	target_up[3];									// target up position coordinate
	FLOAT	current_eye[3];									// current eye position coordinate
	FLOAT	current_up[3];									// current up position coordinate
	FLOAT	center_posi[3];									// current center position coordinate
	FLOAT	belt_radial_posi;								// head of the belt, [0,2*pi]
	FLOAT	FPS;											// average no. of frames per second
	FLOAT	tempoPeriod, tempoPhase, pri_tempoIndex;		// in seconds
	FLOAT	tempo_enhance_factor;
	FLOAT	phase_enhance_factor;
	FLOAT	tempo_period_frac, tempo_period_step;			// fractional part of tempo period
	FLOAT	tempo_period_acc;								// fractional accumulator
	FLOAT	last1FFT[FFTSIZE],last2FFT[FFTSIZE];
	FLOAT	star_scale;
	FLOAT	StarVB[6*4*3*3];								// vertex buffer of spin stars
	FLOAT	BeltVB[BELTCIRCUMSIZE*BELTDEPTHSIZE*3];			// vertex buffer of the belt
	FLOAT	BeltNB[BELTCIRCUMSIZE*BELTDEPTHSIZE*3];			// vertex buffer of the belt
	DWORD	BeltCB[BELTCIRCUMSIZE*BELTDEPTHSIZE];			// color buffer of the belt
	WORD	BeltIB[BELTCIRCUMSIZE*BELTDEPTHSIZE*3];			// index buffer of the belt
	BYTE	BeltAlpha[BELTDEPTHSIZE];						// belt alpha for fast update
	FLOAT	belt_x_off, belt_y_off;
	FLOAT	belt_last_radius;
	DWORD	belt_last_color;
	DWORD	wave_last_color;
	FLOAT	SpriteVB[MAXSPRITES*3];							// geometric coordinates of spin sprite
	DWORD	SpriteCB[MAXSPRITES];							// color buffer of spin sprite
	FLOAT	SpaceVB[SPACEPOINTS*3];
	FLOAT	WaveBuffer[FFTSIZE*3];
	FLOAT	last_time_second;
	FLOAT	last_time_second2;								// 2: for update random target eye
	FLOAT	last_time_second3;								// 3: for update camera path
	FLOAT	power_level;									// Average power level for fft adaptation
	FLOAT	PhaseAccBuf[TempoMaxShift];						// Phase accumulation buffer for phase est.
	FLOAT	PhaseAccBuf2[TempoMaxShift];					// Phase accumulation buffer for phase est.
	FLOAT	CombFilter[TempoMaxShift];						// for phase computation
	FLOAT	TempoWindowFunc[TempoMaxShift];
	FLOAT	ExpWindowFunc[TempoMaxShift];
	FLOAT	TempoSpec[TempoMaxShift];
	FLOAT	TempoSpecP[TempoMaxShift];
	FLOAT	TempoAcorr[TempoMaxShift],
			TempoEcorr[TempoMaxShift],
			TempoEDcorr[TempoMaxShift],
			TempoEDDcorr[TempoMaxShift];

	// Buffers
	FLOAT		*FFTVB, *FFTNB;
	DWORD		*FFTCB;
	WORD		*FFTEB;
	LoopBuffer	*fftBuffer;
	LoopBuffer	*maxBuffer;
	LoopBuffer	*TempoABuffer;		// Amplitude buffer
	LoopBuffer	*TempoEBuffer;		// Energy buffer
	LoopBuffer	*TempoEDBuffer;		// Energy derivative buffer
	LoopBuffer	*TempoEDDBuffer;	// Energy derivative's derivative buffer
	LoopBuffer	*BeltEBuffer;		// Belt Energy buffer
	LoopBuffer	*BeltEDBuffer;		// Belt Energy derivative buffer
	Camera		*camera,*fft_camera;
	BYTE		*PointSpriteData;

	// Functions
	Visualization( BYTE *psData );
	~Visualization();

	void	free();

	void	SetCosineArray( int tempoInd );

	void	init();
	void 	reset( TimedLevel *pLevels, FLOAT _time_stamp );

	int		addData( TimedLevel *pLevels, bool compute_tempo=false );

	void	phase_slide( int phaseDiff );
	void	phase_change( int phaseInd, int n_added );

	void	DrawAll( TimedLevel *pLevels, int n_added );
	void	setupLight( int n_light, float brightness );
};

extern void get_viewport_size(int *Width, int *Height);
extern void rotateBy(FLOAT *center2eye, FLOAT *center2right, FLOAT h);
extern void HV_shift(FLOAT h, FLOAT v);
extern void HV_rotation(FLOAT h, FLOAT v);

#endif