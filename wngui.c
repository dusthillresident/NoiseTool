#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TimeConflictBullshit

#define gui_forreal
#include "gui.c"

#include <alsa/asoundlib.h>
#define WNMAIN
#include "wnt.c"

int wngui_running = 1;

void WNGUI_Close(){
 //tb();
 wngui_running=0;
}


double wngui_ls(FILE *f){
 int s = 0;
 s = fgetc(f);
 s = s | fgetc(f)<<8;
 return (double)s / 65535;
}
void wngui_ss(double val, FILE *f){ 
 int s = 65535 * val;
 fputc(s&255,f);
 fputc((s>>8)&255,f);
}
void wngui_savesettings(double vv[3]){
 FILE *fout=fopen("wngui_settings_file.bin","wb");
 if(fout==NULL)return;
 wngui_ss(vv[0],fout);
 wngui_ss(vv[1],fout);
 wngui_ss(vv[2],fout);
 fputc(n_interpolate,fout);
 fputc(n_stereo,fout);
 fclose(fout);
}
void wngui_loadsettings(double vv[3]){
 FILE *fin=fopen("wngui_settings_file.bin","rb");
 if(fin == NULL)return;
 vv[0]=wngui_ls(fin);
 vv[1]=wngui_ls(fin);
 vv[2]=wngui_ls(fin);
 n_interpolate = fgetc(fin);
 n_stereo = fgetc(fin);
 fclose(fin);
}

#define wn_pad 5
#define wn_outerpad 12

int main(void){

 gui_init(510+wn_outerpad*2,162+wn_outerpad*2);
 SetWindowTitle("NoiseTool");
 gui_CloseHandler = WNGUI_Close;

 pthread_t wnthread;
 pthread_create(&wnthread, NULL, &wnthreadfunction2, NULL);

 int    ss[3] = { 0,0,0 };
 double vv[3] = { 0.125, 0.100, 0.075}; // vol, speed, lowpass
 
 wngui_loadsettings(vv);

 int X,Y;
 while(wngui_running){
  gui_StartLoop();
 
  X=wn_outerpad;
  Y=wn_outerpad;

  gui__label( X, Y, 0,  "Volume");  Y+=textheight(0)+wn_pad;
  gui__slider( X, Y,WinW-wn_outerpad*2,0, ss, vv);  Y+=slidersize+wn_pad;

  gui__label( X, Y, 0,  "Speed");  Y+=textheight(0)+wn_pad;
  gui__slider( X, Y,WinW-wn_outerpad*2,0, ss+1, vv+1);  Y+=slidersize+wn_pad;

  gui__label( X, Y, 0,  "Lowpass");  Y+=textheight(0)+wn_pad;
  gui__slider( X, Y,WinW-wn_outerpad*2,0, ss+2, vv+2);  Y+=slidersize+wn_pad;

  Y+=buttonheight(0); gui__togglebutton(X, Y,0,"Interpolation",&n_interpolate);  Y+=wn_pad;
  Y+=buttonheight(0); gui__togglebutton(X, Y,0,"Stereo",&n_stereo);  Y+=wn_pad;
  Y+=buttonheight(0); gui__togglebutton(X, Y,0,"Enable",&n_enable);  Y+=slidersize+wn_pad;

  n_volume		= vv[0];
  #define minspeed 0.01
  #define maxspeed 1.0
  n_speed		= minspeed + vv[1]*(maxspeed-minspeed);
  n_lowpass_strength	= 1-vv[2];

  if( whitenoisetool_soundfailure ){
   gui_message("crap");
   exit(0);
  }

  gui_EndLoop(0);
 }

 wngui_savesettings(vv);
 
 return 0;
}
