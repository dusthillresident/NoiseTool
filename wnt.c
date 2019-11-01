#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef WNMAIN
#include "mylib.c"
#endif
#include <pthread.h>


double n_speed=1,n_volume=0.5;
int n_interpolate=1;
double n_lowpass_strength=0, n_lowpass_state=0, n_lowpass_state2=0;
int n_enable=0;
int n_stereo=1;

int whitenoisetool_soundfailure=0;

struct nnoise_state {
 double oldsample,newsample,pos;
} ;
typedef struct nnoise_state nnoise_state;


void doubleswap(double *a, double *b){
 double c;
 c = *a; *a = *b; *b = c;
}

double rnoise(nnoise_state *ns, double speed,double volume, int interpolate){
 ns->pos += speed;
 while(ns->pos>=1.0){
  doubleswap(&ns->newsample,&ns->oldsample);
  ns->newsample = Rnd(65536)-32768;
  ns->pos-=1.0;
 }
 //printf("fffuck %f, %f, %f\n",ns->newsample,ns->oldsample,ns->pos);
 //printf("fuck %f, %f\n",speed,volume);
 if( interpolate )
  return (ns->oldsample * (1.0-ns->pos) + ns->newsample * ns->pos) * volume ;
 else
  return ns->oldsample * volume ;
}


double lowpass(double *state, double strength, double in){
 double o, n;
 o = strength;
 n = 1.0-strength;
 *state = *state * o  +  in * n;
 //printf("lowpass state: %f\nstrength: %f\nin: %f\n",*state,strength,in);
 return *state;
}


#define FFFUCK ((1024*4)*2)//(48000/8)
void *wnthreadfunction2(void* arg){
 nnoise_state nst  = { 0, 0, 0 };
 nnoise_state nst2 = { 0, 0, 0 };
 short buffer[FFFUCK]; // sound buffer
 static char *device = "default"; //"default"; /* playback device */
 snd_output_t *output = NULL;
 int err;
 unsigned int i;
 snd_pcm_t *handle;  // i dunno lol
 snd_pcm_sframes_t frames;

 //printf("ssss\n");

 if ((err = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) { // open sound device (I assume) 
  printf("Playback open error: %s\n", snd_strerror(err));
  //exit(EXIT_FAILURE);
  goto wnt_failloop;
 }

 // set sound format (I assume)
 if ((err = snd_pcm_set_params(handle,			// pcm
                               SND_PCM_FORMAT_S16,	// format
                               SND_PCM_ACCESS_RW_INTERLEAVED, // 'access'
                               2,			// num channels
                               48000,			// rate
                               1,			// soft_resample
                               100000 * 0.65)) < 0){   // latency. apparently, 100000 is 1 second. multiplier below 0.6 had underrun trouble
  printf("Playback open error: %s\n", snd_strerror(err));
  //exit(EXIT_FAILURE);
  goto wnt_failloop;
 }
 while(1){
  if(!n_enable){
   for(i=0; i < FFFUCK; i+=1) buffer[i] = 0;
  }else if(n_stereo){
   for(i=0; i < FFFUCK; i+=2){
    buffer[i  ] = lowpass( &n_lowpass_state,  n_lowpass_strength, rnoise(&nst,  n_speed,n_volume,n_interpolate) );
    buffer[i+1] = lowpass( &n_lowpass_state2, n_lowpass_strength, rnoise(&nst2, n_speed,n_volume,n_interpolate) );
   }
  }else{
   for(i=0; i < FFFUCK; i+=2){ 
    double samp = lowpass( &n_lowpass_state, n_lowpass_strength, rnoise(&nst, n_speed,n_volume,n_interpolate) );
    buffer[i]=samp; buffer[i+1]=samp; 
   }
  }
  frames = snd_pcm_writei(handle, buffer, sizeof(buffer)/sizeof(short)/2);
  if( frames < 0 ) frames = snd_pcm_recover(handle, frames, 0);
  if( frames < 0 ){
   printf("snd_pcm_writei failed: %s\n", snd_strerror(frames));
   break;
  }
  if (frames > 0 && frames < (long)sizeof(buffer)/sizeof(short)/2 ) printf("ALSA: Short write (expected %li, wrote %li)\n", (long)sizeof(buffer), frames);
 }
 wnt_failloop:
 whitenoisetool_soundfailure = 1;
 while(1){
  sleep(1);
  tb();
 }
 return NULL;
}

