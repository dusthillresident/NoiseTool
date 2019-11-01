#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloca.h>
#define myxlib_notstandalonetest 1
#include "NewBase.c"

// debug options:	print start & endloops, print events
#ifndef GUI_DEBUG
 #define GUI_DEBUG 0b00
#endif

#if GUI_DEBUG & 0b10
int gui_loopcounter=0;
#endif

// done:
//  button
//  menu
//  menubutton
//  text label
//  toggle button
//  string editor
//  sliders
//  intbox

// to do:
//  colour scheme support

// eventually:
//  tab book

/*

 -------------
 --- NOTES ---
 -------------

 Sat 10 Aug 2019
  I discovered that setting a background pixel makes it so the window is automatically cleared when it's resized, which results in a lot of flicker.
  I changed newbase so that Cls() simply draws a filled rectangle of whatever colour NewBase_last_bg_col is.
  A background colour is never actually set.
  There is much less flicker now. 

  I got 'refresh off' working too which may also potentially reduce flicker.

  The 'dontclear' thing is unused now because now that no background pixel is ever set, it is always necessary to Cls() when there is an expose event.
  I'm leaving that there anyway because it might be useful later on.

*/

/*
void tb(){
 system("beep -f 100 -l 2 &");
}
*/


#define saveomousepos int SaVeD_omx=gui.omx,SaVeD_omy=gui.my;
#define restoreomousepos gui.mx=SaVeD_omx;gui.my=SaVeD_omy;//gui.dontwaitnextturn=1;
#define savemouse    int SaVeD_omx=gui.omx, SaVeD_omy=gui.omy, SaVeD_omb=gui.omb, SaVeD_mx=gui.mx, SaVeD_my=gui.my, SaVeD_mb=gui.mb;
#define restoremouse gui.omx=SaVeD_omx; gui.omy=SaVeD_omy; gui.omb=SaVeD_omb; gui.mx=SaVeD_mx; gui.my=SaVeD_my; gui.mb=SaVeD_mb;

struct guidata {
 int requestredraw,
     redraw,
     dontwaitnextturn,
     resized,
     exposed,
     dontclear,
     
     csx,csy, // click start x,y position
     omx, omy, omb,
      mx,  my,  mb,
     nmx, nmy, nmb,
     
     // ==== COLOURS ====
     // foreground
     fg_normal,
     fg_dark,
     // background
     bg_normal,
     bg_dark,
     bg_bright,
     bg_field,
     // border
     bd_normal,
     bd_dark,
     bd_bright,

     blarghrubbishpadunused;
};

struct guidata gui;

#define gui_init(w,h) (gui__init(w,h,0))
#define gui_init_threading(w,h) (gui__init(w,h,1))

void gui__init(int w, int h,int threading){
 int i;
 for(i=0; i<sizeof(struct guidata); i++){
  ((unsigned char*)(&gui))[i]=0;
 }
 gui.requestredraw=1;
 NewBase_MyInit(w,h,threading);
 // ==== COLOURS ====
 // foreground
 gui.fg_normal = MyColour2(0xFFFFFF);
 gui.fg_dark   = MyColour2(0xC0C0C0);
 // background
 gui.bg_normal = MyColour2(0x404040);
 gui.bg_dark   = MyColour2(0x303030);
 gui.bg_bright = MyColour2(0x606060);
 gui.bg_field  = MyColour2(0x000000);
 // border
 gui.bd_normal = MyColour2(0x24A7FF);
 gui.bd_dark   = MyColour2(0x808080);
 gui.bd_bright = MyColour2(0x7FFFFF);
 GcolDirect(gui.fg_normal);
 NewBase_last_bg_col = gui.bg_normal;
 // custom character graphics
 CustomChar(224,1,3,6,12,24,176,224,64);          // Tick
 CustomChar(225,60,66,129,129,129,129,66,60);     // Radio unchecked
 CustomChar(226,60,66,153,189,189,153,66,60);     // Radio checked
 CustomChar(227,0,126,66,66,66,66,126,0);         // Unticked box
 CustomChar(228,1,127,70,110,122,82,126,0);       // Ticked box
 CustomChar(229,0,0,255,126,60,24,0,0);           // Down arrow for menubuttons
 Cls();
}

void (*gui_CloseHandler)()=NULL;

int gui_HandleEvents(int EnableBlocking){
 int i;
 unsigned char ch;
 int DidSomethingHappen=0;
 int WindowWasResized=0;
 //Refresh();
 while(XEventsQueued(Mydisplay,QueuedAlready) || EnableBlocking){
  EnableBlocking=0;
  DidSomethingHappen=1;
  //printf("XNextEvent\n");
  XNextEvent(Mydisplay, &Myevent);
  //printf("XNextEvent returned\n");

  //printf("event.type: %d\n",Myevent.type);

  switch(Myevent.type){
  case Expose:
   #if (GUI_DEBUG&0b1)
   printf("Expose\n");
   #endif
   gui.requestredraw=1;
   //gui.dontclear = !gui.resized;
   gui.exposed=1;
   break;
  case ConfigureNotify:
   #if (GUI_DEBUG&0b1)
   printf("ConfigureNotify\n");
   #endif
   if( WinW != Myevent.xconfigure.width || WinH != Myevent.xconfigure.height ){
    WinW = Myevent.xconfigure.width;
    WinH = Myevent.xconfigure.height;
    gui.resized=1;
    gui.requestredraw=1;
    gui.dontclear=0;
    WindowWasResized=1;
   }
   break;
  case KeyRelease:
  case KeyPress:
   #if (GUI_DEBUG&0b1)
   if( Myevent.type == KeyRelease ) printf("KeyRelease\n");
   if( Myevent.type == KeyPress   ) printf("KeyPress\n");
   #endif
   #if (GUI_DEBUG&0b1)
   printf("st: %d\n",Myevent.xkey.state); printf("kc: %d\n",Myevent.xkey.keycode);
   #endif
   ch=GetCharFromEvent(&Myevent);
   PutOntoKeyboardBuffer( (0x80000000 * (Myevent.type == KeyRelease)) | ((Myevent.xkey.state&0xff)<<16) | ((Myevent.xkey.keycode&0xff)<<8) | ch );
   #if (GUI_DEBUG&0b1)
   printf("ch: %d\n",ch);
   #endif
   break;
/*
  case KeyRelease:
   #if (GUI_DEBUG&0b1)
   printf("KeyRelease\n");
   //printf("KeyRelease\n");
   #endif
   break;
*/
  case ClientMessage: {
    #if (GUI_DEBUG&0b1)
    printf("ClientMessage\n");
    //system("Shia");
    #endif
    /*
    char *thisatomnamestring =XGetAtomName(Mydisplay, Myevent.xclient.message_type);
    printf("%d: %s\n",Myevent.xclient.message_type,thisatomnamestring);
    XFree(thisatomnamestring);
    printf("%x %x %x %x %x\n",Myevent.xclient.data.l[0],Myevent.xclient.data.l[1],Myevent.xclient.data.l[2],Myevent.xclient.data.l[3],Myevent.xclient.data.l[4]);
    tb();   printf("%d, %d\n",XInternAtom( Mydisplay, "WM_DELETE_WINDOW", 1 ),  Myevent.xclient.message_type );
    */
    if( (Myevent.type == ClientMessage && (ulong) Myevent.xclient.data.l[0] == WmDeleteWindowAtom) ){
     if( gui_CloseHandler ){
      gui_CloseHandler();
     }else{
      MyCleanup();
      exit(0);
     }
    }/*else{
     tb();
    }*/
   }
   break;
  case MotionNotify:
   #if (GUI_DEBUG&0b1)
   printf("MotionNotify\n");
   #endif
   gui.nmx=Myevent.xmotion.x;
   gui.nmy=Myevent.xmotion.y;
   break;
  case ButtonPress:
   #if (GUI_DEBUG&0b1)
   printf("ButtonPress\n");
   #endif
   if(!gui.mb){
    gui.csx = gui.mx; gui.csy = gui.my;
   }
   gui.mb |= 1<<(Myevent.xbutton.button-1);
   break;
  case ButtonRelease:
   #if (GUI_DEBUG&0b1)
   printf("ButtonRelease\n");
   #endif
   gui.nmb |= 1<<(Myevent.xbutton.button-1);
   break;
  case SelectionNotify:
   #if (GUI_DEBUG&0b1)
   printf("SelectionNotify\n");
   #endif
   break;
  default:
   #if (GUI_DEBUG&0b1)
   printf("unhandled: %d\n",Myevent.type);
   #endif
   break;
  }//endcase

 }//endwhile

 if( WindowWasResized && newbase_manual_refresh_mode ){
  UpdateMypixmap();
  Wait(1);
 }
 
 //Refresh();
 return DidSomethingHappen;
}//endproc

void gui_StartLoop(){
 #if (GUI_DEBUG&0b10)
 printf("startloop (counter: %d)\n",gui_loopcounter++);
 #endif
 gui.omx=gui.mx; gui.omy=gui.my; gui.omb=gui.mb; // update old mouse pos
 gui.mx=gui.nmx; gui.my=gui.nmy; // get current mouse pos
 gui.mb = gui.mb & ~gui.nmb; gui.nmb=0; // update mouse button state
 gui.resized=0; //clear resized flag
 gui.exposed=0; //clear exposed flag
 if( !XEventsQueued(Mydisplay,QueuedAlready) && !gui.dontwaitnextturn && !gui.requestredraw && gui.mx==gui.omx && gui.my==gui.omy && gui.mb==gui.omb){ //wait for events
  //Wait(1);
  #if (GUI_DEBUG&0b10)
  printf("waiting\n");
  #endif
  gui_HandleEvents(1);
  //Refresh();

 }
 gui_HandleEvents(0); // process events
 
 if(gui.requestredraw){ // handle redraw request
  if( newbase_manual_refresh_mode ) Mydrawable = NewBase_Mypixmap;
  if(gui.dontclear)
   gui.dontclear=0;
  else{
   //printf("BLAHHGHGHGHG %d\n",Mydrawable == NewBase_Mypixmap );
   Cls();
  }
  gui.redraw=1;
  gui.requestredraw=0;
 }else if(gui.dontwaitnextturn){ 
  gui.dontwaitnextturn=0;
 }
}

void gui_EndLoop(int w){
 #if (GUI_DEBUG&0b10)
 printf("endloop\n");
 #endif
 
 if( newbase_manual_refresh_mode && Mydrawable == NewBase_Mypixmap ){
  Refresh();
  Mydrawable = Mywindow;
 }

 if(gui.requestredraw) return;
 //if(gui.redraw) Refresh();
 //if(!gui.redraw) Wait(w);
 gui.redraw=0;
}

void showbin(int in){
 int i=1<<31;
 for(i=31; i>=0; i--){
  putchar(48+!!(in&(1<<i)) );
  //printf("\n i %d \n",1<<i); Wait(50);
 }
 printf(" (%d)",in);
}

int gui_boxcheck(int x, int y, int w, int h, int px,int py){
 #if 0
  Gcol(255,128,0);
  Rectangle(x,y,w,h);
  //CircleFill(px,py,4);
  //Refresh();
  //Wait(50);
 #endif
 int x1,y1,x2,y2, swp;
 x1=x; x2=x+w; if(x1>x2){ swp=x1; x1=x2; x2=swp; }
 y1=y; y2=y+h; if(y1>y2){ swp=y1; y1=y2; y2=swp; }
 return px>=x1 && px<=x2 && py>=y1 && py<=y2;
}

// =================================================================================

Pixmap gui_saverect(int x,int y,int w,int h){
 if(w<0){
  x+=w; w=-w;
 }
 if(h<0){
  y+=h; h=-h;
 }
 w+=1; h+=1;
 Pixmap px = XCreatePixmap(Mydisplay,Mywindow,w,h,SDepth);
 XCopyArea(Mydisplay, Mywindow,px, MyGC, x,y,w,h, 0,0);
 //Gcol(0,255,0); Rectangle(x,y,w,h); Circle(x,y,10); //Refresh(); Wait(50);
 return px;
}

void gui_restorerect(Pixmap px, int x,int y,int w,int h){
 //printf("restorerect: x:%5d, y:%5d, w:%5d, h:%5d\n");
 if(w<0){
  x+=w; w=-w;
 }
 if(h<0){
  y+=h; h=-h;
 }
 w+=1; h+=1;
 XCopyArea(Mydisplay, px,Mywindow, MyGC, 0,0,w,h, x,y);
 XFreePixmap(Mydisplay,px);
 //Gcol(255,128,0); Rectangle(x,y,w,h); Circle(x,y,10); //Refresh(); Wait(50);
}

// =================================================================================

#define buttonpadding 10
#define buttonwidth(scale,text)   (8*(!!((scale)&0b10)+1)*strlen(text)+buttonpadding)
#define buttonwidth_(scale,textl) (8*(!!((scale)&0b10)+1)*(textl)+buttonpadding)
#define buttonheight(scale)       (8*(!!((scale)&0b01)+1)+buttonpadding)

int gui__button(int x, int y, int scale, char *text){
 if(gui.requestredraw) return 0;

 int xs,ys;
 xs = 1+!!(scale&2);
 ys = 1+!!(scale&1);

 int xe,ye, rd, bgcol, bdcol; ye=8*ys+buttonpadding; xe=8*xs*strlen(text)+buttonpadding; y-=ye;
 void (*PrintFunc)(int,int,unsigned char*);

 switch(scale&3){
 case 0b00: PrintFunc=Print;  break;
 case 0b01: PrintFunc=Print2; break;
 case 0b10: PrintFunc=Print4; break;
 case 0b11: PrintFunc=Print3; break;
 }

 rd = gui_boxcheck(x,y,xe,ye, gui.omx,gui.omy) ^ gui_boxcheck(x,y,xe,ye, gui.mx,gui.my);
 //draw
 if(gui.redraw || rd){ //tb();

  if( gui_boxcheck(x,y,xe,ye, gui.mx,gui.my) ){
   bgcol=gui.bg_bright;
   bdcol=gui.bd_bright;
  }else{
   bgcol=gui.bg_normal;
   bdcol=gui.bd_normal;
  }

  GcolDirect(bgcol);
  RectangleFill(x,y,xe,ye);
  GcolDirect(bdcol);
  Rectangle(x,y,xe,ye);
  PrintFunc(x+buttonpadding/2,y+buttonpadding/2+1,text);
  
 }
 
 return (gui_boxcheck(x,y,xe,ye, gui.mx,gui.my) && !(gui.mb&1) && (gui.omb&1)) && gui_boxcheck(x,y,xe,ye, gui.csx,gui.csy);
}

#define gui_button(x,y,s)  gui__button(x,y,0b00,s)
#define gui_button2(x,y,s) gui__button(x,y,0b01,s)
#define gui_button3(x,y,s) gui__button(x,y,0b10,s)
#define gui_button4(x,y,s) gui__button(x,y,0b11,s)

void gui_seriousproblem(char *s){
 gui.requestredraw=1;
 while(1){
  gui_StartLoop();
  if(gui.redraw){
   GcolDirect(gui.fg_dark);
   Print(WinW/2-7*8,WinH/2,"Serious problem:");
   Print(WinW/2-strlen(s)/2*8,WinH/2+8,s);
  }
  if( gui_button(WinW/2,WinH/2+40,"Quit") ) Quit();
  if( gui_button(WinW/2,WinH/2+40+buttonheight(0)+4,"Continue") ){ gui.requestredraw=1;  return; }
  gui_EndLoop(1);
 }
}

int howmanyofthischaracterinthisstring(char *s, char c){
 int r=0;
 while(*s){
  r += (*s++)==c;
 }
 return r;
}

char* strcopyuntil(char *dest, char *src, char *until){
 int i,l; l=strlen(until);
 while(*src){
  for(i=0; i<=l; i++) if( *src == until[i] ){ src++; goto scu_out; }
  *dest = *src; dest++; src++;
 }
 scu_out:
 *dest=0;
 return src;
}

void strextendtolength(char *str, int length, int limit, char extension){
 int i = strlen(str);
 while(i<length && i<limit){
  str[i++]=extension;
 }
 str[i]=0;
}

#define menupadding 4
#define menuspacing 2
#define menu_maxchoicelen 128	// maximum string length allowed for menu items (buttons inside menus)
#define menu_maxsublen 128	// maximum string length allowed for submenu strings (strings that define submenus)

int gui__menu(int x, int y, int scale, char *menu){
 if( gui.requestredraw ) return 0;
 saveomousepos;

 int choice = 0, num_choices=0, i, xe,ye, maxlength=0, makeextraspaceforsubmenuarrow=0;
 
 char choices[32][menu_maxchoicelen];
 char submenustrings[32][menu_maxsublen];
 for(i=0; i<32; i++){ 
  choices[i][0]=0;
  submenustrings[i][0]=0;
 }
 
 // parse menu string
 int bc=0;		// bracket count
 char *p = menu;	// pointer for looking through the menu string
 i=0;			// position in current menu option that's being processed
 int ii=0;		// position in current submenu string that's being processed
 int cmo=0;	// the current menu option
 while( *p ){
  switch( *p ){
  case ';':
   choices[cmo][i]=0; i=0; 
   if( strlen(choices[cmo] ) > maxlength ) maxlength=strlen(choices[cmo]);
   cmo++; num_choices++; if(num_choices > 31){ gui_seriousproblem("can't have more than 31 items in a menu"); return 0; }
   break;
  case '{':
   if(!i){ gui_seriousproblem("tried to define submenu for empty menu option"); return 0; }
   if( submenustrings[cmo][0] ){ gui_seriousproblem("submenu already defined"); return 0; }
   makeextraspaceforsubmenuarrow=1;
   bc=1;
   ii=-1;
   while(bc){
    p++;
    submenustrings[cmo][++ii]=*p;
    switch( *p ){
    case '{': bc+=1; break;
    case '}': bc-=1; break;
    case 0:   gui_seriousproblem("broken menustring, missing } or ended too soon"); return 0; break;
    }
   }
   submenustrings[cmo][ii]=0;
   break;

  case '}':
   gui_seriousproblem("bad } in menustring"); return 0;
   break;
  default:
   choices[cmo][i++]=*p;
   break;
  }
  p++;
 }
 if(i){ gui_seriousproblem("rubbish at end of menustring or missing ;"); return 0; }
 if(num_choices==0){ gui_seriousproblem("empty menu"); return 0; }
 // get every menu option to be the same length
 if(makeextraspaceforsubmenuarrow)maxlength+=2; // extra space for " >" at the end of menu items with submenus
 for(i=0; i<num_choices; i++){
  strextendtolength( choices[i], maxlength, 256, ' ');
  if( submenustrings[i][0] ) choices[i][maxlength-1]='>';
 }
 
 int buttonxsize = buttonwidth(scale,choices[0]); int buttonysize = buttonheight(scale);
 xe=buttonxsize+menupadding; ye=buttonysize*num_choices+menupadding+menuspacing*(num_choices-1); 
 //save rectangle 
 Pixmap px = gui_saverect(x,y,xe,ye);
 //draw
 GcolDirect(gui.bg_dark); RectangleFill(x,y,xe,ye);
 GcolDirect(gui.bd_bright); Rectangle(x,y,xe,ye);
 gui.redraw=1; gui.dontwaitnextturn=1;

 while( !choice && !gui.resized ){ // display menu buttons and get user choice
  gui_StartLoop();

  for(i=0; i<num_choices; i++){
   if( gui__button( x+menupadding/2, y+menupadding/2 + (buttonysize+menuspacing)*i + buttonysize, scale, choices[i] ) ) choice=i+1;
  }
  if(gui.mb && !gui_boxcheck(x,y,xe,ye, gui.mx,gui.my)){ choice=0; gui_EndLoop(0); break; }
 
  gui_EndLoop(1);
 } 

 int outval = choice;  
 if( choice && submenustrings[choice-1][0] ){
  int smresult = gui__menu( x+menupadding/2+24, y+menupadding/2 + (buttonysize+menuspacing)*choice - menuspacing, scale, submenustrings[choice-1] );
  if(smresult)
   outval = choice | (smresult<<5);
  else 
   outval = 0;
 }

 //gui.requestredraw=1; gui.dontwaitnextturn=1;
 if(gui.resized){
  XFreePixmap(Mydisplay,px);
  gui.requestredraw=1;
  outval=0;
 }else{
  gui_restorerect(px, x,y,xe,ye);
 }
 restoreomousepos;
 return outval;
}

void gui_clickwait(){
 int rd   = gui.redraw,
     rrd  = gui.requestredraw,
     dwnt = gui.dontwaitnextturn;
 gui.redraw = 0; gui.requestredraw = 0; gui.dontwaitnextturn = 0;
 while(gui.mb){
  //gui_HandleEvents(0);
  gui_StartLoop();
  //Wait(1);
  //Refresh();
 }
 gui.redraw = rd; gui.requestredraw = rrd; gui.dontwaitnextturn = dwnt;
}

// menubutton scale parameter:
// 0bABCD
//	AB	scale setting for the button that opens the menu
//	CD	scale setting for the menu items

int gui__menubutton(int x,int y,int scale, char *title,char *menu){
 if( gui.requestredraw ) return 0;
 int r,_ox,_oy,_mx,_my;
 if( gui__button(x,y,scale>>2,title) && !gui.redraw ){
  _mx=gui.mx; _my=gui.my; _ox=gui.omx; _oy=gui.omy;
  r = gui__menu(x,y,scale & 0b11,menu);
  if( gui_boxcheck(x,y,buttonwidth(scale>>2,title),-buttonheight(scale>>2), gui.mx,gui.my) ){
   gui_clickwait();
   gui.mx=x;gui.my=y;
   gui__button(x,y,scale>>2,title);
   //Gcol(255,0,0); Rectangle(x,y,buttonwidth(scale,title),-buttonheight(scale)); Refresh();
  }
  #if 1
  else{
   gui.omx=_mx; gui.omy=_my; gui.mx=-1; gui.my=-1;
   gui__button(x,y,scale>>2,title);
   gui.omx=_ox; gui.omy=_oy; gui.mx=_mx; gui.my=_my;
  }
  #endif
  gui.dontwaitnextturn=1;
 }else{
  return 0;
 }
 return r;
}

void gui__label(int x, int y, int scale, char *label){
 if( gui.requestredraw || !gui.redraw ) return;

 void (*PrintFunc)(int,int,unsigned char*);
 switch(scale&3){
 case 0b00: PrintFunc=Print;  break;
 case 0b01: PrintFunc=Print2; break;
 case 0b10: PrintFunc=Print4; break;
 case 0b11: PrintFunc=Print3; break;
 }
 
 GcolDirect(gui.fg_dark);
 PrintFunc(x,y,label);
 //Gcol(255,128,0); Plot69(x,y); Circle(x,y,10);
}

#define inttoolsize 12
#define inttoolpad  2

int gui_inttool(int x, int y, int *in){
 if(gui.requestredraw) return 0;

 int i, xe,ye, changed=0;
 xe = inttoolsize*32; ye = inttoolsize;

 if( !gui.omb && gui.mb && gui_boxcheck(x,y,xe-1,ye, gui.mx, gui.my) ){
  *in ^= 1 << (int) ((double)( (inttoolsize*32)-(gui.mx - x) )/(inttoolsize*32)*32) ;
  changed=1;
 }

 if( gui.redraw || changed ){
  GcolDirect(gui.bg_dark);   RectangleFill(x-inttoolpad,y-inttoolpad,xe+inttoolpad*1,ye+inttoolpad*2);
  GcolDirect(gui.fg_dark);   Rectangle    (x-inttoolpad,y-inttoolpad,xe+inttoolpad*1,ye+inttoolpad*2);
  //GcolDirect(gui.fg_dim;
  if(1)
  for(i=0; i<32; i++){
   if(*in & (1<<i))
    //CircleFill(x+inttoolsize/2 + inttoolsize*i,y+inttoolsize/2,inttoolsize/2);
    RectangleFill( x + inttoolsize*(31-i) ,y,inttoolsize-1,inttoolsize+1);
   else 
    //Circle    (x+inttoolsize/2 + inttoolsize*i,y+inttoolsize/2,inttoolsize/1);
    Rectangle    ( x + inttoolsize*(31-i),y,inttoolsize-2,inttoolsize);
  }
  if(0)
  for(i=1;i<32/4;i++){
   Line(x+inttoolsize*4*i,y-inttoolpad,x+inttoolsize*4*i,y+inttoolsize+inttoolpad);
  }
  //Gcol(255,128,0); Circle(x,y,6); Plot69(x,y);
 }
 return changed;
}

int gui__togglebutton(int x, int y, int scale, char *text, int *v){
 if(gui.requestredraw) return 0;
 char s[100]="x ";
 strcat(s,text);
 s[0]=227+!!*v;
 if( gui__button(x,y,scale,s) ){
  *v=!*v; s[0]=227+!!*v;
  int holdredraw = gui.redraw;
  gui.redraw=1;
  gui__button(x,y,scale,s);
  gui.redraw=holdredraw;
  return 1;
 }
 return 0;
}

#define slidersize 14

#define SGN(in) (((in)>0)-((in)<0))

int gui__slider(int x,int y,int sz,int vertical,int *state, double *v){
 if( gui.requestredraw ) return *state;

 int xe,ye, rd, active;
 if(vertical){
  xe = slidersize;
  ye = sz;
  if(sz<0) y-=sz;
 }else{
  xe = sz;
  ye = slidersize;
  if(sz<0) x-=sz;
 }

 //sz -= slidersize*(sz>0 ? -1 : 1 );

 if( gui_boxcheck(x,y,xe,ye, gui.mx,gui.my) && !(gui.omb&1) && (gui.mb&1) ){ 
  *state |= 1;
 }else if( !(gui.mb&1) ){ 
  *state = (*state << 1) & 0b11;
 }

 rd = gui.redraw || *state;

 if(*state&1){
  *v = vertical ? (gui.my-y-slidersize/2*SGN(sz))/(double)(sz-slidersize*SGN(sz)) : (gui.mx-x-slidersize/2*SGN(sz))/(double)(sz-slidersize*SGN(sz));
  if(*v<0)
   *v=0;
  else if(*v>1)
   *v=1;
 }

 if(rd){ 

  GcolDirect( *state&1 ? gui.bg_bright : gui.bg_dark  );
  RectangleFill(x+1,y+1, xe-1,ye-1);
  GcolDirect( *state&1 ? gui.bd_bright : gui.bd_normal );
  Rectangle(x,y,xe,ye);
  // Gcol(255,0,0); RectangleFill(x+1,y+1, xe-1,ye-1); /*test*/
  int pos = (sz-slidersize*SGN(sz)) * *v + slidersize/2*SGN(sz);
  if(vertical)
   //CircleFill( x + slidersize/2, y + pos, slidersize/2);
   RectangleFill( x + slidersize/2 -slidersize/4, y + pos -slidersize/4, slidersize/2, slidersize/2);
  else
   //CircleFill( x + pos, y + slidersize/2, slidersize/2);
   RectangleFill( x + pos -slidersize/4, y + slidersize/2 -slidersize/4, slidersize/2, slidersize/2);
  
 }
 
 return *state;
}


int gui_processlist(char array[16][256], char *list, int *text_width_return){
 //printf("gui_processlist start\n");
 int num_items=0, i=0, l=1, text_width=0;
 char *p = list;
 while( l ){
  switch( *p ){
  case 0: l=0;
  case ';':
   if(i>0){
    array[num_items][i]=0;
    if(text_width<i) text_width=i;
    num_items+=1;
   }
   i=-1;
   break;
  default:
   array[num_items][i]=*p;
   break;
  }
  i++;
  p++;
 }
 if(text_width_return != NULL) *text_width_return = text_width;
 //printf("gui_processlist end\n");
 return num_items;
}

// 0bAABBCC: AA is title size, BB is text size, cc is button size
// 0bXYXYXY

#define messageboxpad 10

int gui__qmsg(int scale, char *title, char *text, char *buttontitles){

 if(text==NULL){
  gui_seriousproblem("gui__message: not given a message");
  return 0;
 }

 char nulstr=0; if(title==NULL || !title[0]) title=&nulstr;
 if(buttontitles==NULL || !buttontitles[0]) buttontitles="OK";
 char lines[16][256]; int num_lines=0;
 char buttons[16][256]; int num_buttons=0;
 
 int text_width=0, text_height=0, title_height = 8*(!!(scale&0b010000)+1)*!!strlen(title), title_width = strlen(title)*8*(!!(scale&0b100000)+1), buttons_width=0;
 int x,y,xe,ye, titlex,titley,textx,texty,buttonx,buttony, bxpos;
 int i, choice;
 
 num_lines = gui_processlist(lines,text,&text_width);
 num_buttons = gui_processlist(buttons,buttontitles,NULL);

 text_width  = text_width * 8 * (!!(scale&0b001000)+1);
 text_height =  num_lines * 8 * (!!(scale&0b000100)+1);
 for(i=0; i<num_buttons; i++){
  buttons_width += buttonwidth(scale,buttons[i]);
 }
 buttons_width += messageboxpad*(num_buttons-1);
 xe = text_width + messageboxpad*2;
 ye = text_height + title_height + buttonheight(scale) + messageboxpad*3 + messageboxpad*!!strlen(title);
 if(xe<title_width+messageboxpad*2) xe = title_width + messageboxpad*2;
 if(xe<buttons_width+messageboxpad*2) xe = buttons_width + messageboxpad*2;

 gui.redraw=1; gui.dontwaitnextturn=1; //gui.requestredraw=1;

 int buttonpressed=0;
 
 while(!buttonpressed){
  gui_StartLoop();
 
  if(gui.redraw && !gui.requestredraw ){

   x = WinW/2 - xe/2; y = WinH/2 - ye/2;
   textx = WinW/2 - text_width/2; texty = y + messageboxpad + (messageboxpad+title_height)*!!*title;
   titlex = WinW/2 - title_width/2; titley = y + messageboxpad;
 
   buttonx = (x+xe)-messageboxpad-buttons_width;
   buttony = (y+ye)-messageboxpad;
   
   GcolDirect(gui.bg_normal); RectangleFill(x,y,xe,ye);
   GcolDirect(gui.fg_dark);   Rectangle    (x,y,xe,ye);

   for(i=0; i<num_lines; i++){
    gui__label(textx,texty + 8*(!!(scale&0b000100)+1)*i,(scale&0b1100)>>2,lines[i]);
   }
   if(*title) {
    gui__label(titlex,titley,(scale&0b110000)>>4,title);
    Line(titlex,titley+title_height+1,titlex+title_width,titley+title_height+1);
   }
  }

  bxpos = buttonx;
  for(i=0; i<num_buttons; i++){ 
   if(gui__button(bxpos, buttony,scale,buttons[i])){
    buttonpressed=1;
    choice = i;
   }
   bxpos += buttonwidth(scale,buttons[i]) + messageboxpad;
  }

  gui_EndLoop(1);
 }
 
 gui.requestredraw=1; gui.dontwaitnextturn=1;

 return choice;
}

#define gui_message(text) gui__qmsg(0,"",text,"")


void insertchars(char *string, char *in, unsigned int maxlen){
 int i=0, la = strlen(string), lb = strlen(in);
 char *temp = alloca( la+1 ); strcpy(temp,string);
 while( in[i] && i<maxlen-1 ){
  string[i] = in[i];
  i++;
 }
 string[i]=0;
 while( temp[i-lb] && i<maxlen-1 ){
  string[i] = temp[i-lb];
  i++;
 }
 string[i]=0;
}

void insertchar(char *string, char in, int buffsize){
 char inn[2] = { in, 0 };
 insertchars(string,inn,buffsize);
}

void delchars(char *string, unsigned int pos, unsigned int count){
 int i, len = strlen(string);
 for(i=pos; i<len; i++){
  string[i]=string[i+1];
 }
}

void savetexttofile(char *string, unsigned int len, char *filename){
 FILE *fout = Openout(filename);
 if(fout==NULL){ 
  printf("SHIT\n");
  return;
 }
 unsigned int i; 
 for(i=0;i<len;i++){
  fputc(string[i],fout);
 }
 fclose(fout);
}

void setcliptext(char *string, unsigned int len){
 savetexttofile(string,len,"/tmp/gui_myclipcrap");
 system("xclip -in /tmp/gui_myclipcrap -selection clipboard");
}

char* getcliptext(){
 system("xclip -out -selection clipboard > /tmp/gui_myclipcrap");
 FILE *fin = Openin("/tmp/gui_myclipcrap");
 if(fin==NULL || Ext(fin)==0){
  return NULL;
 }
 char *buf = malloc(Ext(fin)+1);
 int i;
 for(i=0; i<Ext(fin); i++){
  buf[i]=fgetc(fin);
 }
 buf[i]=0;
 fclose(fin);
 return buf;
}

char* copychars(char *dst, char *src, int num){
 int i;
 for(i=0; i<num; i++)
  *dst++ = *src++;
 *dst=0;
 return dst;
}

void new_insertchars(char *string, int *curlen, int maxlen, char *in, int pos){

 int la = strlen(string);
 int lb = strlen(in);

 if(pos<0)  pos=0;
 if(pos>la) pos=la;
 
 int l_a = la - (la-pos),     l_b = la - pos;
  
 char //*ta = alloca(l_a+1),
      //*tb = alloca(l_b+1),
      *t  = alloca(la+lb+1);


#if 0
 copychars(t,string,l_a);
 copychars(t+la,in,lb);
 copychars(t+la+lb,in,l_b);
#endif

 copychars( copychars( copychars(t,string,l_a) ,in,lb) ,string+l_a,l_b);

 copychars(string,t, la+lb<=maxlen ? la+lb : maxlen ); 
 
 *curlen=strlen(string);

}

void new_delchars(char *string, int *curlen, int pos, int count ){


 if(count<0){
  pos = pos + count;
  count=-count;
 }

 int olen = strlen(string);

 if( pos>olen ){
  pos = olen;
 }
 if( pos<0 ){
  pos = 0;
 }
 if( pos+count > olen ){
  count=olen-pos;
 }

 int nlen = olen - count;

 char *t = alloca(nlen+1);
 char *p=t;
 
 int i=0;
 
 for(i=0; i<olen+1; i++)
  if( i<pos || i>=pos+count)
   *p++ = string[i];

 strcpy(string,t);
 *curlen = strlen(string);
}

// -------- string editor -----------

// StringEdData: 0 state, 1 cursor position, 2 selection start, 3 selection length, 4 current string length, 5 maximum allowed length for the string (which is the buffersize-1)

struct StringEdData {
 int state;
 int cursor;
 int selectionstart;
 int selectionlen;
 int curlen;
 int maxlen;
};

void gui_updateSED(struct StringEdData* SED,char *buf, size_t bufsize){
 SED->curlen=strlen(buf);
 SED->maxlen=bufsize-1;
 SED->cursor=SED->curlen;
}
struct StringEdData* gui_prepareSED(char *buf, size_t bufsize){
 struct StringEdData *SED = calloc(1,sizeof(struct StringEdData));
 gui_updateSED(SED,buf,bufsize);
 return SED;
}

void gui___stred_cursorcheck(struct StringEdData *SED,char *string){
 if(SED->cursor<0) SED->cursor=0;
 if(SED->cursor>SED->curlen) SED->cursor=SED->curlen;
}

void gui___stred_delete(struct StringEdData *SED,char *string){
 if(SED->selectionlen){
  int pos,count;
  pos = SED->selectionlen>0 ? SED->selectionstart : SED->selectionstart + SED->selectionlen;
  count = abs(SED->selectionlen);
  new_delchars(string,&SED->curlen,pos,count);
  SED->selectionlen=0;
  SED->cursor = pos;
 }else{
  new_delchars(string,&SED->curlen,SED->cursor,1);
 }
 gui___stred_cursorcheck(SED,string);
}

void gui___stred_backspace(struct StringEdData *SED,char *string){
 if(SED->selectionlen){
  gui___stred_delete(SED,string);
 }else if(SED->cursor>0){
 SED->cursor = SED->cursor - 1;
  gui___stred_delete(SED,string);
 }
}

void gui___stred_insertchar(struct StringEdData *SED,char *string, char ch){
 if(SED->selectionlen){
  gui___stred_delete(SED,string); 
 }
 char tbuf[2] = { ch, 0 };
 new_insertchars(string,&SED->curlen,SED->maxlen,tbuf,SED->cursor);
 SED->cursor +=1;
 gui___stred_cursorcheck(SED,string);
}
void gui___stred_insertchars(struct StringEdData *SED,char *string, char *in){
 if(SED->selectionlen){
  gui___stred_delete(SED,string); 
 }
 new_insertchars(string,&SED->curlen,SED->maxlen,in,SED->cursor);
 SED->cursor += strlen(in);
 gui___stred_cursorcheck(SED,string);
}
void gui___stred_copy(struct StringEdData *SED,char *string){
 if( !SED->selectionlen ) return;
 int pos,count;
 pos = SED->selectionlen>0 ? SED->selectionstart : SED->selectionstart + SED->selectionlen;
 count = abs(SED->selectionlen);
 setcliptext(string+pos,count);
}
void gui___stred_cut(struct StringEdData *SED,char *string){
 if(!SED->selectionlen) return;
 gui___stred_copy(SED,string);
 gui___stred_delete(SED,string); 
}
void gui___stred_paste(struct StringEdData *SED,char *string){
 char *paste = getcliptext();
 if(paste){
  gui___stred_insertchars(SED,string,paste);
  free(paste);
 }
}
void gui___stred_selall(struct StringEdData *SED,char *string){
 SED->selectionstart = 0;
 SED->selectionlen = SED->curlen;
 SED->cursor = SED->curlen;
 gui___stred_cursorcheck(SED,string);
}
void gui___stred_clear(struct StringEdData *SED,char *string){
 *string=0; SED->curlen=0; SED->cursor=0; SED->selectionstart=0; SED->selectionlen=0;
}

#define textwidth(scale)   (8*((((scale)&0b10)>>1)+1))
#define textheight(scale)  (8*(((scale)&0b01)+1))
#define textwidth_(scale,str)   ((8*((((scale)&0b10)>>1)+1))*strlen(str))
#define textwidth__(scale,leng)   ((8*((((scale)&0b10)>>1)+1))*(leng))
#define stredpad 4
#define stredwidth(scale,w)  (stredpad*2+textwidth(scale)*(w))
#define stredheight(scale,h) (stredpad*2+textheight(scale)*(h))

// return 1 if the string was modified, 0 otherwise

int gui__stringeditor(int x, int y, int w, int h, int scale, struct StringEdData *SED, char *string ){
 if( gui.requestredraw ) return 0;
 x+=stredpad; y+=stredpad;

 int *state          = (int*)SED + 0,
     *cursor         = (int*)SED + 1,
     *selectionstart = (int*)SED + 2,
     *selectionlen   = (int*)SED + 3,
     *curlen         = (int*)SED + 4,
     *maxlen         = (int*)SED + 5; 
  
 int oldstate = *state,
     i,
     xe = stredwidth(scale,w) - stredpad*2,
     ye = stredheight(scale,h)- stredpad*2,

     rval=0,
     rd_border = gui.redraw,
     rd_all = gui.redraw,
     boxchek = gui_boxcheck(x-stredpad,y-stredpad,xe+stredpad*2,ye+stredpad*2, gui.mx,gui.my),
     innerboxchek = gui_boxcheck(x,y,xe,ye,gui.mx,gui.my);

 // become active if clicked in box or inactive if clicked outside box
 if( !gui.omb && gui.mb ){
  *state = boxchek;  
  if( oldstate != *state ) rd_border = 1;
 }
 if(!boxchek && !(gui.omb&1) && (gui.mb&1) && *selectionstart && *selectionlen){ // clear selection if click outside box
  *selectionstart=0; *selectionlen=0; rd_all=1;
 }

 if( *state ){

  // process mouse input
  if(boxchek){

   if(innerboxchek && gui.mb&1){ // left button
    *cursor = (gui.my-y)/textheight(scale)*w + (gui.mx-x)/textwidth(scale); //set cursor pos
    if(*cursor<0) *cursor=0; else if(*cursor>*curlen) *cursor=*curlen;
    if(gui.omb&1){ // click & drag : set selection 
     *selectionlen = *cursor - *selectionstart;
    }else{ // initial click: clear selection
     *selectionlen = 0; *selectionstart=*cursor;
    }
    rd_all=1; 
   }
   if(gui.mb&4){ // right click menu
    rd_border=1;
    GcolDirect( gui.bd_bright ); Rectangle( x-stredpad,y-stredpad,xe+stredpad*2,ye+stredpad*2 ); // must make sure the border becomes bright
    switch( gui__menu(gui.mx,gui.my,0,"Cut;Copy;Paste;Clear;Select all;") ){
    case 1: // cut
     rval=1; rd_all=1; gui___stred_cut(SED,string); break;
    case 2: // copy
     gui___stred_copy(SED,string); break;
    case 3: // paste
     rval=1; rd_all=1; gui___stred_paste(SED,string); break;
    case 4: // clear
     rval=1; rd_all=1; gui___stred_clear(SED,string); break;
    case 5: // select all
     rd_all=1; gui___stred_selall(SED,string); break;
    }
    *state = gui_boxcheck(x-stredpad,y-stredpad,xe+stredpad*2,ye+stredpad*2, gui.nmx,gui.nmy);
    gui.mb=0;
   }

  }


 // process keyboard input
  int ch;
  while( KeyBufferUsedSpace() ){ //system("beep -f 100 -l 10 &");
   ch = GET();
   //printf("%d %d %d\n",ch>>16&0xff,ch>>8&0xff,ch&0xff);
   if( ! (ch & 0x80000000) ){// if this is a key press (not a key release)
    if( ch&0xff ){ // keypress that produces a character
     switch( ch&0xff ){
     case 8: // backspace
      rval=1; rd_all=1; gui___stred_backspace(SED,string);
      break;
     case 127: // delete
      rval=1; rd_all=1; gui___stred_delete(SED,string);
      break;
     case 24:
      rval=1; rd_all=1; gui___stred_cut(SED,string); break;
      break;
     case 3:   // copy (ctrl + c)
      gui___stred_copy(SED,string); break;
      break;   
     case 22:  // paste (ctrl + v)
      rval=1; rd_all=1; gui___stred_paste(SED,string); break;
      break;
     case 1:   // select all (ctrl+a)
      rd_all=1; gui___stred_selall(SED,string); break;
      break;
     case 13:
      rd_border = 1; *state = 0;
     default:
      if( (ch&0xff)>=32 && (ch&0xff)<127 ){ // normal character
       rval=1; rd_all=1; gui___stred_insertchar(SED,string,ch&0xff);
      }
      break;
     }//endcase
    }else{ // keypress that doesn't produce a character 

     if( ((ch&0xff00)>>8)>=110 && ((ch&0xff00)>>8)<=116 ){ // arrow keys & home & end
      switch( ((ch&0xff00)>>8) ){
      case 111: *cursor -= w; rd_all=1; break; // up
      case 116: *cursor += w; rd_all=1; break; // down
      case 113: *cursor -= 1; rd_all=1; break; // left
      case 114: *cursor += 1; rd_all=1; break; // right
      case 110: *cursor = 0;  rd_all=1; break; // home
      case 115: *cursor = *curlen; rd_all=1; break; // end
      }
      gui___stred_cursorcheck(SED,string);
      if(ch&0x010000){ *selectionlen = *cursor - *selectionstart; } else { *selectionlen=0; *selectionstart=*cursor; }// shift
     }
    }//endif (whether or not it was a keypress that produces a character
   }//endif (whether or not it was a keypress rather than a keyrelease)
  }//endwhile (KeyBufferUsedSpace)

 }//endif (*state)

 // redraw
 if( rd_all ){
  
  // draw box
  GcolDirect(gui.bg_field);
  RectangleFill( x-stredpad,y-stredpad,xe+stredpad*2,ye+stredpad*2 );
  GcolDirect( *state ? gui.bd_bright : gui.bd_normal );
  Rectangle    ( x-stredpad,y-stredpad,xe+stredpad*2,ye+stredpad*2 );
  // draw characters
  void (*PrintFunc)(int,int,unsigned char*);
  switch(scale&3){
  case 0b00: PrintFunc=Print;  break;
  case 0b01: PrintFunc=Print2; break;
  case 0b10: PrintFunc=Print4; break;
  case 0b11: PrintFunc=Print3; break;
  }
  i=0;
  int xx=0,yy=0;
  char pch[2]; pch[1]=0;
  int s_st=*selectionstart,s_ed = s_st + *selectionlen,sw;
  if( s_st>s_ed ){ sw=s_st; s_st=s_ed; s_ed=sw; }
  while( string[i] && yy<h ){
  
   *pch = string[i]; 
   if( i>=s_st && i<s_ed ){
    GcolDirect(gui.fg_normal);
    RectangleFill( x  +  xx * 8*(((scale&0b10)>>1)+1), y  +  yy * 8*((scale&0b01)+1), 8*(((scale&0b10)>>1)+1), 8*((scale&0b01)+1) );
    GcolDirect(gui.bg_field);
    PrintFunc(x  +  xx * 8*(((scale&0b10)>>1)+1), y  +  yy * 8*((scale&0b01)+1), pch);
   }else{
    GcolDirect(gui.fg_normal);
    PrintFunc(x  +  xx * 8*(((scale&0b10)>>1)+1), y  +  yy * 8*((scale&0b01)+1), pch);
   }
   xx += 1;
   if( xx >= w ){
    xx=0;
    yy += 1;
   }
   i++;
  }
  // draw cursor
  if(*cursor/w < h){
   GcolDirect(gui.bd_normal);
   Line(x + *cursor % w *8*(((scale&0b10)>>1)+1), y + *cursor / w *8*((scale&0b01)+1), x + *cursor % w *8*(((scale&0b10)>>1)+1), y + *cursor / w *8*((scale&0b01)+1) + 8*((scale&0b01)+1));
   Line(-1+x + *cursor % w *8*(((scale&0b10)>>1)+1), y + *cursor / w *8*((scale&0b01)+1), -1+x + *cursor % w *8*(((scale&0b10)>>1)+1), y + *cursor / w *8*((scale&0b01)+1) + 8*((scale&0b01)+1));
  }
  //endif rd_all
 }else if(rd_border){
  GcolDirect( *state ? gui.bd_bright : gui.bd_normal );
  Rectangle    ( x-stredpad,y-stredpad,xe+stredpad*2,ye+stredpad*2 );
 }

 
 return rval;
}


// =================================================================================
// =================================================================================
// =================================================================================
// =================================================================================

struct gui___coltool_component {
 int sliderst;
 struct StringEdData stredst;
 char stredstr[4];
 double v;
};

void gui___coltool_fixstring(struct gui___coltool_component *C){
 int val = C->v*255;
 snprintf(C->stredstr,4,"%d",val);
 C->stredst.state=0;
 C->stredst.cursor=strlen(C->stredstr);
 C->stredst.selectionstart=0;
 C->stredst.selectionlen=0;
 C->stredst.curlen=strlen(C->stredstr);
}

void gui___coltool_fixv(struct gui___coltool_component *C){
 C->v = (double)atoi(C->stredstr)/255;
 if(C->v>1) C->v=1;
 if(C->v<0) C->v=0;
}

void gui___coltool_initcomponent(struct gui___coltool_component *C,unsigned char col){
 C->v = (double)col/255;
 gui___coltool_fixstring(C);
 C->stredst.maxlen=3;
 C->sliderst=2;
}

#define coltoolpad 8

int gui___coltool_componenteditor(int x, int y, struct gui___coltool_component *C){
 if(gui.requestredraw) return 0;
 int rval=0;
 
 int holdrd = gui.redraw;

 gui__slider(x,y,255+slidersize,0,&C->sliderst,&C->v);
 if(C->sliderst){
  // update string and set gui.redraw
  gui___coltool_fixstring(C);
  gui.redraw=1;
  rval=1;
 }   

 if( gui__stringeditor(x + 255+slidersize +coltoolpad,y,3,1,0,&C->stredst,C->stredstr) ){
  gui___coltool_fixv(C);
  gui.redraw=1; gui__slider(x,y,255+slidersize,0,&C->sliderst,&C->v);
  rval=1;  
 }

 gui.redraw=holdrd;

 #if 0
 if(gui.redraw){ 
  Gcol(255,255,255);
  Circle(x,y,10);
  Plot69(x,y);
 }
 #endif

 return rval; 
}

#define coltool_yspacing 20
#define coltool_previewsize 50

int gui_coltool(int *col){
 saveomousepos;
 //gui.requestredraw=1;
 gui.redraw=1; gui.dontwaitnextturn=1;
 struct gui___coltool_component R,G,B;
 gui___coltool_initcomponent(&R,(*col&0xff0000)>>16);
 gui___coltool_initcomponent(&G,(*col&0xff00)>>8);
 gui___coltool_initcomponent(&B,*col&0xff);
 int x,y,xe,ye, loop=1, OK=0, outcol, reqrd=0;
 xe = 255+slidersize+stredwidth(0,3)+coltool_previewsize+coltoolpad*4;
 ye = coltool_yspacing*3+stredheight(0,1)+coltoolpad*3;
 x = WinW/2-xe/2;
 y = WinH/2-ye/2;
 Pixmap px = gui_saverect(x,y,xe,ye);
 while(loop){
  gui_StartLoop();

  if(gui.redraw){ 
   x = WinW/2-xe/2;
   y = WinH/2-ye/2;
   GcolDirect(gui.bg_normal); RectangleFill(x,y,xe,ye);
   GcolDirect(gui.fg_dark);   Rectangle    (x,y,xe,ye);
  }

  if( gui_button(x+xe-(coltoolpad+buttonwidth(0,"OK")),y+ye-coltoolpad,"OK") ){
   //gui.requestredraw=1;
   loop=0;
   OK=1;
  }
  if( gui_button(x+xe-(coltoolpad*2+buttonwidth(0,"OK")+buttonwidth(0,"Cancel")),y+ye-coltoolpad,"Cancel") ){
   //gui.requestredraw=1;
   loop=0;
  }

  if( gui_button(x+coltoolpad,y+ye-coltoolpad,"Invert") ){
   R.v = 1 - R.v; gui___coltool_fixstring(&R);
   G.v = 1 - G.v; gui___coltool_fixstring(&G);
   B.v = 1 - B.v; gui___coltool_fixstring(&B);
   gui.redraw=1;
  }
  
  if(
  gui___coltool_componenteditor(x+coltoolpad,y+coltoolpad,&R) |
  gui___coltool_componenteditor(x+coltoolpad,y+coltoolpad+coltool_yspacing,&G) |
  gui___coltool_componenteditor(x+coltoolpad,y+coltoolpad+coltool_yspacing*2,&B) |
  gui.redraw ||
  gui.requestredraw
  ){
   outcol = (int)(R.v*255)<<16 | (int)(G.v*255)<<8 | (int)(B.v*255);
   if(!gui.requestredraw){
    GcolDirect(MyColour2(outcol));
    RectangleFill( (x+xe)-(coltoolpad+coltool_previewsize),y+coltoolpad+coltool_yspacing*3/2-coltool_previewsize/2,coltool_previewsize,coltool_previewsize);
   }
  }

  //if( gui_button(30,30,"cls") ) Cls();  
  reqrd |= gui.resized | gui.exposed;
  gui_EndLoop(1);
 }
 
 if(OK) *col = outcol;
 
 if(reqrd){
  XFreePixmap(Mydisplay,px);
  gui.requestredraw=1;
 }else{ 
  gui_restorerect(px, x,y,xe,ye);
 }

 restoreomousepos;
 return OK;
}

// =================================================================================

struct gui_intboxstate {
 struct StringEdData sed;
 char str[12];
 int old;
};

#define intboxpad 4

#define intboxwidth(scale,w) (buttonwidth(scale," ")*2+intboxpad*2+stredwidth(scale,w))

int gui__intbox(int x, int y, int scale, int w, struct gui_intboxstate *ibs, int *in){
 if( gui.requestredraw ) return 0;

 int holdredrawflag = gui.redraw;
 int rd=0;

 if( *in != ibs->old ) rd=1;

 if( gui__button(x,y+buttonheight(scale),scale,"-") ){
  *in = *in - 1;
  rd=1;
 } 

 if( gui__button(x+buttonwidth(scale," ")+intboxpad*2+stredwidth(scale,w),y+buttonheight(scale),scale,"+") ){
  *in = *in + 1;
  rd=1;
 } 

 if(rd){
  gui.redraw=1;
  snprintf(ibs->str,sizeof(ibs->str),"%d",*in);
  ibs->sed.curlen=strlen(ibs->str); // set new curlen for string ed
  ibs->sed.cursor=ibs->sed.curlen;      // set cursor to the end of the string
  ibs->sed.state=0;                // clear state flag and selection
  ibs->sed.selectionstart=0;
  ibs->sed.selectionlen=0;
  ibs->sed.maxlen=sizeof(ibs->str)-1;
 }

 if( gui__stringeditor(x+buttonwidth(scale," ")+intboxpad,y,w,1,scale,&ibs->sed,ibs->str) ){
  *in = atoi(ibs->str);
  rd=1;
 }
 
 ibs->old = *in;
 
 gui.redraw=holdredrawflag;
 return rd;
}


// =================================================================================
// =================================================================================
// =================================================================================

int gui_cs_hexvalchar(char c){
 if( c>='0' && c<='9' ){
  return c-'0';
 }
 c = c & ~32;
 if( c>='A' && c<='F'){
  return c-'A'+10;
 }
 return 0;
}
int gui_cs_hexvalstring(char *s){
 int i,out=0;
 for(i=0; i<2*3; i++){
  if(!s[i]) return out;
  out |= gui_cs_hexvalchar(s[i])<<4*(5-i);
 }
 return out;
}
void gui_setcolourscheme(char *colourschemestring){
 int *cols = &gui.fg_normal;
 int i;
 for(i=0; i<63; i++){
  //printf("i %d: \"%c\" (%d)   %d\n",i,colourschemestring[i],colourschemestring[i], i%7);
  if(
    !(
     (colourschemestring[i]>='0' && colourschemestring[i]<='9')
     ||
     (colourschemestring[i]>='A' && colourschemestring[i]<='F')
     ||
     (colourschemestring[i]>='a' && colourschemestring[i]<='f')
     ||
     ( colourschemestring[i]==';' && ((i%7)==6)) 
     )
    ){
   gui_seriousproblem("Invalid colour scheme");
   return;
  }//endif
 }//next
 // import
 char temp[10]; temp[0]=0;
 char *tp = colourschemestring;
 for(i=0; i<9; i++){
  tp=strcopyuntil(temp,tp,";");
  cols[i]=MyColour2(gui_cs_hexvalstring(temp));
 }
 GcolBGDirect(gui.bg_normal);
 gui.requestredraw=1;
 gui.redraw=1;
}

// =================================================================================
// =================================================================================
// =================================================================================

#define testbufferlength (16*4+1)

#ifndef gui_forreal

void keypresstestbox(int x, int y, int *state){
 const int wh = 24;
 if( gui.requestredraw ) return;
 int redraw = gui.redraw;
 if(gui.mb){
  *state = gui_boxcheck(x, y, wh, wh, gui.mx,gui.my);
  redraw=1;
 }
 if( *state ){ 
  if( KeyBufferUsedSpace() ){
   int bgcol;
   int ch = GET();
   char str[2] = { 0,0 };
   str[0] = ch & 0xff;
   if( ch & 0x80000000 )
    bgcol = MyColour2( 0x00007f );
   else
    bgcol = MyColour2( 0x7f0000 );
   GcolDirect(bgcol);
   RectangleFill(x,y,wh,wh);
   GcolDirect(gui.fg_normal);
   Print3(x+wh/4,y+wh/4,str);
   redraw=1; 
  }
 }
 if(redraw){
  GcolDirect(*state?gui.bd_bright:gui.bd_normal);
  Rectangle(x,y,wh,wh);
 }
}

void no_op_func(){
}
void testclosehandler(){
 gui_CloseHandler = NULL;
 if( gui__qmsg(0b000011,"Quit?","Do you want to quit?","No;Yes;") ) Quit();
 gui_CloseHandler = testclosehandler;
}

int main(int argc, char **argv){

 gui_init(510,340);

 gui_CloseHandler = testclosehandler;

 int blah,i;
 //char testmenustr[]="One;Two{Aa;Bb;Cc{Xx;Yy{agh{fin;};};};};Three;a;b;c;d;e;f{here;};";
 char testmenustr[]="One;Two{Xxx;Yyy;Zzz{blah1;blah2;};};Three;";
 //Gcol(255,255,255);
 int myint=0b11;

 int sliderst=2;
 double sliderv=0;

 char numstr[100]; numstr[0]=0;

 char *whoah=malloc(testbufferlength); snprintf(whoah,testbufferlength,"this;is a;menu{this;is a;submenu;};");
 struct StringEdData stredd = { 0, 0, 0, 0, strlen(whoah), testbufferlength-1 };

 int coltoolint=0x4080ff;

 Pixmap testpx;
 int saveorloadfortestpixmap=0;

 struct gui_intboxstate ibs = { 0,0,0,0,11,11, 0 };
 int ibin=123;

 int keypresstesterstate=0;
 
 int refreshonofftogglebutton=0;
 
 //RefreshOff();
 while(1){
  gui_StartLoop();

  /*
  for( i=0; i<4; i++)
   gui__stringeditor(10,10+60*i,4,2, i, stredd, whoah);
  */

#if 0
  if(gui.redraw || gui.omb != gui.mb){
   GcolDirect(gui.bg_normal);
   CircleFill(WinW/2,WinH/2,50);
   GcolDirect(gui.bd_normal);
   if(gui.mb)
    CircleFill(WinW/2,WinH/2,50);
   else
    Circle    (WinW/2,WinH/2,50);
   system("beep -f 100 -l 10 &");
  }
  printf("gui.mb: "); showbin(gui.mb); printf("\n");
#endif
  //printf("gui.mb: "); showbin(gui.mb); printf("\n");
  if( gui_button2(WinW/2,WinH/2-40,"Test") ) Cls();//system("Shia");
  if( gui.redraw ){
   Gcol(255,255,255);
   Circle(WinW/2,WinH/2-40,10);
  }

 
  gui__label(180,190,myint,"this");
 
  //if( gui_button3(8,20+8,"thanks") ) gui__menu(8,20+8,"one;two{aa;bb;};three;");
  blah = gui__menubutton(8,20+15,0b0000,"MenuBtn",testmenustr);
  if(blah) for(i=0; i<7; i++){
   printf("menulevel %d : %d\n",i,blah&31); blah = blah >> 5;
  }
  blah = gui__menubutton(8+70,20+15,0b0101,"menubutton2","one;two{penistos;blah;};");
  blah = gui__menubutton(8+172,20+15,0b1000,"mb3","yes;");
  //Quit();

  if( gui_inttool(30,WinH-30,&myint) ) gui.requestredraw=1;

  if( gui__togglebutton(40,WinH-100,0b00,"Test",&myint) ) gui.requestredraw=1;

  if(myint&4)
   gui__slider(20,50,-120,myint&1,&sliderst,&sliderv);
  else
   gui__slider(20,50,120,myint&1,&sliderst,&sliderv);

  if(sliderst&2){
   snprintf(numstr,50,"%f",sliderv);
   gui.requestredraw=1;
  }
 
  gui__label(180,215,0,numstr);
  
  if( gui__stringeditor(100,100,16,4, 0b00, &stredd, whoah) );// system("beep -f 100 -l 7 &");
  gui__menubutton(100,100+stredheight(0,4)+buttonheight(0)+4,0,"Testmenu",whoah);
 
 
  if( gui_button(WinW-75,100,"blah") ){
   gui_coltool(&coltoolint);
  }
  if( gui__togglebutton(WinW-75,125,0,"blah2",&saveorloadfortestpixmap) ){

   if(saveorloadfortestpixmap)
    testpx = gui_saverect(10,10,120,120);
   else 
    gui_restorerect(testpx,10,10,120,120);

  }
  if( gui_button(WinW-75,150,"blah3") ){
   for(i=0; i<250; i++){ GcolDirect(Rnd(0xffffff+1)); Circle(Rnd(WinW),Rnd(WinH),Rnd(100)); }
  }
  if( gui__togglebutton(WinW-75,175,0,"blah4",&refreshonofftogglebutton) ){
   if( refreshonofftogglebutton ){
    //tb();
    RefreshOff();
    gui.requestredraw=1;
   }else{
    //system("Shia");
    RefreshOn();
    gui.requestredraw=1;
   }
  }

  if( gui__intbox(WinW-150,190,0b01, 11, &ibs, &myint) ){
   //tb();
   gui.requestredraw=1;
  }

  //if( gui_button (8,WinH-30,"Circle") ) Circle(Rnd(WinW),Rnd(WinH),150);
  if( gui_button (8,WinH-40,"SP") ) gui_seriousproblem("blah");
  //if( gui_button (8+30,WinH-40,"MSG") ) myint=gui__qmsg(Rnd(0b111111+1),"Title","This is some text;blah one two three;;dbz forever","one;two;three;four;");
  if( gui_button (8+30,WinH-40,"Quit") ) if( gui__qmsg(0b0,"Quit","Are you sure?","Cancel;Quit;") ) Quit();
  //if( gui_button (8+30,WinH-40,"MSG") ) gui__message(0b111111,NULL,"This is some text;blah one two three;dbz forever",NULL);
  //if( gui_button (8+30,WinH-40,"MSG") ) gui__message(0b111111,"very long title","s","s");
  //if( gui_button (8+30,WinH-40,"MSG") ) gui__message(0b111111,NULL,NULL,NULL);
  //if( gui_button4(WinW-155,WinH-80,"Eject -t") ) system("eject -t &");

  //if( gui.redraw ) system("beep -f 100 -l 10 &");

  keypresstestbox(350,100,&keypresstesterstate);

  ClearKeyboardBuffer();

  gui_EndLoop(1);
 }

 return 0;
}

#endif
