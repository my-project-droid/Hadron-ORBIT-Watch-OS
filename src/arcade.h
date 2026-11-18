int terrainHeight(float x){if(x>=51&&x<=77)return 53;return 43+int(5*sin(x*0.12f)+3*cos(x*0.24f));}
// Original compact games, designed for a three-way switch.
int arcadeKind=0,ax=60,ay=0,points=0,enemyShift=0,enemyDir=1,enemyDown=0,bulletX=0,bulletY=-1,rockX=128;
bool arcadeRun=false,arcadeOver=false,invaders[18];uint32_t arcadeTick=0,enemyTick=0;
float landX=64,landY=16,landVX=0,landVY=0,fuel=100;int thrust=0,simMode=0;float simAngle=0;bool simPaused=false; bool simFacts=false; int factIndex=0;
uint16_t blocks[20];int piece=0,rotation=0,pieceX=3,pieceY=0;uint32_t fallTick=0;
const uint16_t shapes[7][4]={{0x000f,0x1111,0x000f,0x1111},{0x0033,0x0033,0x0033,0x0033},{0x0072,0x0131,0x0027,0x0232},{0x0036,0x0231,0x0036,0x0231},{0x0063,0x0132,0x0063,0x0132},{0x0071,0x0113,0x0047,0x0322},{0x0074,0x0311,0x0017,0x0223}};
bool fits(int x,int y,int r){uint16_t s=shapes[piece][r];for(int j=0;j<4;j++)for(int i=0;i<4;i++)if(s&(1<<(j*4+i))){int xx=x+i,yy=y+j;if(xx<0||xx>=10||yy>=20||(yy>=0&&(blocks[yy]&(1<<xx))))return false;}return true;}
void newPiece(){piece=random(7);rotation=0;pieceX=3;pieceY=0;if(!fits(pieceX,pieceY,0))arcadeOver=true;}
void dropPiece(){if(fits(pieceX,pieceY+1,rotation)){pieceY++;return;}uint16_t s=shapes[piece][rotation];for(int j=0;j<4;j++)for(int i=0;i<4;i++)if(s&(1<<(j*4+i)))blocks[pieceY+j]|=1<<(pieceX+i);for(int y=19;y>=0;y--)if(blocks[y]==1023){for(int k=y;k>0;k--)blocks[k]=blocks[k-1];blocks[0]=0;points++;y++;}newPiece();}
void resetArcade(){ax=60;ay=0;points=0;enemyShift=0;enemyDir=1;enemyDown=0;bulletY=-1;rockX=128;arcadeOver=false;arcadeRun=false;for(auto &b:invaders)b=true;memset(blocks,0,sizeof(blocks));newPiece();landX=64;landY=16;landVX=landVY=0;fuel=100;thrust=0;arcadeTick=enemyTick=fallTick=millis();}
void arcadeMove(int d){if(page==SPACE_SIM){if(simFacts)factIndex=(factIndex+d+6)%6;else simMode=(simMode+d+10)%10;dirty=true;return;}if(arcadeKind==0)ax=constrain(ax+d*7,5,122);else if(arcadeKind==1){if(fits(pieceX+d,pieceY,rotation))pieceX+=d;}else if(arcadeKind==2){if(ay==0)ay=18;}else landVX=constrain(landVX+d*0.12f,-1.0f,1.0f);dirty=true;}
void arcadeSelect(){if(page==SPACE_SIM){simFacts=!simFacts;factIndex=0;dirty=true;return;}if(!arcadeRun||arcadeOver){resetArcade();arcadeRun=true;}else if(arcadeKind==0&&bulletY<0){bulletX=ax;bulletY=48;}else if(arcadeKind==1){int r=(rotation+1)%4;if(fits(pieceX,pieceY,r))rotation=r;}else if(arcadeKind==2){if(ay==0)ay=18;}else thrust=thrust?0:1;dirty=true;}
void arcadeUpdate(){if(page==SPACE_SIM){if(!simPaused&&millis()-arcadeTick>100){arcadeTick=millis();simAngle+=0.04f;dirty=true;}return;}if(page!=ARCADE||!arcadeRun||arcadeOver||millis()-arcadeTick<70)return;arcadeTick=millis();dirty=true;
 if(arcadeKind==0){if(bulletY>=0){bulletY-=4;for(int i=0;i<18;i++)if(invaders[i]){int x=12+(i%6)*17+enemyShift,y=17+(i/6)*8+enemyDown;if(abs(bulletX-x)<6&&abs(bulletY-y)<5){invaders[i]=false;bulletY=-1;points++;break;}}}if(millis()-enemyTick> (700-difficulty*180)){enemyTick=millis();enemyShift+=enemyDir*3;if(enemyShift>15||enemyShift<-7){enemyDir=-enemyDir;enemyDown+=3;}if(enemyDown>=18||points==18)arcadeOver=true;}}
 else if(arcadeKind==1){if(millis()-fallTick> (900-difficulty*250)){fallTick=millis();dropPiece();}}
 else if(arcadeKind==2){rockX-=2+difficulty;if(rockX<-8){rockX=128+random(30);points++;}if(ay>0)ay--;int h=ay?int(24*sin(PI*ay/18)):0;if(rockX<25&&rockX>9&&h<10)arcadeOver=true;}
 else{landVY+=0.025f;if(thrust>0&&fuel>0){landVY-=0.065f;fuel-=0.7f;}landX+=landVX;landY+=landVY;if(landX<4||landX>124||landY<12)arcadeOver=true;if(landY+5>=terrainHeight(landX)){arcadeOver=true;points=landX>=51&&landX<=77&&abs(landVX)<0.25f&&abs(landVY)<0.45f?1:0;landY=terrainHeight(landX)-5;}}
}
// Relative orbital rates; illustrative phases and compressed distances.
const char* planetNames[]={"Mercury","Venus","Earth","Mars","Jupiter","Saturn","Uranus","Neptune"};
const float years[]={0.241,0.615,1,1.881,11.86,29.46,84.01,164.8};
const char* majorMoons[]={"No moons","No moons","Moon","Phobos, Deimos","Io Europa Ganymede Callisto","Titan Rhea Iapetus Dione","Miranda Ariel Umbriel Titania Oberon","Triton Nereid Proteus"};
const int moonDots[]={0,0,1,2,4,4,5,3};
// Stable educational facts; major moons only, not an exhaustive catalog.
const char* spaceFacts[10][6]={
 {"Eight planets orbit Sun","Four rocky inner planets","Four giant outer planets","Mercury/Venus: no moons","Distances here compressed","Phases are illustrative"},
 {"ISS: orbital laboratory","Truss supports arrays","Solar arrays supply power","Modules house crew","International partnership","Drawing is a schematic"},
 {"Mercury: nearest to Sun","Smallest of 8 planets","Rocky, cratered surface","No natural moons","Year: about 88 days","Very thin exosphere"},
 {"Venus: thick atmosphere","Hottest planet surface","No natural moons","Rotates retrograde","Year: about 225 days","Similar size to Earth"},
 {"Earth: liquid oceans","One natural moon","Moon: cratered surface","Moon always same face","Year: about 365 days","Moon causes ocean tides"},
 {"Mars: rocky red planet","Two small moons","Phobos: inner moon","Deimos: outer moon","Year: about 687 days","Iron oxides redden soil"},
 {"Jupiter: largest planet","Io: volcanic world","Europa: likely ocean","Ganymede: largest moon","Callisto: many craters","Year: about 11.86 years"},
 {"Saturn: icy ring system","Titan: thick atmosphere","Titan: methane lakes","Rhea: icy moon","Iapetus: two-tone surface","Dione: icy moon"},
 {"Uranus: tilted ice giant","Miranda: varied terrain","Ariel: icy moon","Umbriel: dark surface","Titania: largest here","Oberon: cratered moon"},
 {"Neptune: outermost planet","Triton: backwards orbit","Triton: nitrogen geysers","Nereid: elongated orbit","Proteus: irregular shape","Year: about 165 years"}
};
void renderSpace(){
 if(simFacts){text(8,simMode<2?(simMode==0?"Solar system facts":"ISS facts"):planetNames[simMode-2]);String fact=spaceFacts[simMode][factIndex];int cut=fact.length()>25?fact.lastIndexOf(' ',25):-1;if(cut<0)cut=min(25,(int)fact.length());text(23,fact.substring(0,cut));text(34,fact.substring(cut));text(45,String(factIndex+1)+" / 6");text(54,"+/- fact; SEL model");return;}

 if(simMode==0){text(8,"Solar system: 8 planets");oled.drawDisc(64,32,3);for(int i=0;i<8;i++){float a=simAngle/years[i];int r=9+i*7;oled.drawEllipse(64,32,r,r/3);oled.drawDisc(64+cos(a)*r,32+sin(a)*r/3,1);}text(55,"SEL facts | +/- planet");}
 else if(simMode==1){text(8,"ISS structure schematic");oled.drawHLine(15,30,98);for(int x=16;x<113;x+=27){oled.drawFrame(x,13,10,14);oled.drawFrame(x,34,10,14);for(int y=16;y<46;y+=4)oled.drawHLine(x,y,10);}oled.drawFrame(58,18,12,25);oled.drawFrame(51,26,27,7);oled.drawCircle(64,47,3);text(55,"SEL facts | +/- model");}
 else{int p=simMode-2;text(8,planetNames[p]);oled.drawDisc(64,30,p<4?5:8);if(p==5)oled.drawEllipse(64,30,14,4);for(int i=0;i<moonDots[p];i++){int r=13+i*4;float a=simAngle/(i+1);oled.drawEllipse(64,30,r,r/2);oled.drawDisc(64+cos(a)*r,30+sin(a)*r/2,1);}String names=majorMoons[p];int offset=names.length()>25?((millis()/900)%max(1,(int)names.length()-24)):0;text(49,names.substring(offset,offset+25));text(56,"SEL facts | +/- planet");}}
void arcadeRender(){if(page==SPACE_SIM){renderSpace();return;}
 const char* names[]={"Space Invaders","Tetris","Geometry Jump","Lunar Lander"};text(8,String(names[arcadeKind])+" "+String(points));
 if(!arcadeRun){text(24,arcadeKind==0?"+/- move; SEL shoot":arcadeKind==1?"+/- move; SEL rotate":arcadeKind==2?"SEL or +/-: jump":"+/- steer; SEL engine toggle");text(38,arcadeKind==3?"Land slow on center pad":"Hold SEL: back");text(52,"Select to start");return;}
 if(arcadeKind==0){for(int i=0;i<18;i++)if(invaders[i]){int x=12+i%6*17+enemyShift,y=17+i/6*8+enemyDown;oled.drawBox(x-3,y-2,7,4);oled.drawPixel(x-4,y+3);oled.drawPixel(x+4,y+3);}oled.drawTriangle(ax-5,53,ax+5,53,ax,47);if(bulletY>=12)oled.drawVLine(bulletX,bulletY,3);}
 else if(arcadeKind==1){oled.drawFrame(48,0,32,57);for(int y=0;y<20;y++)for(int x=0;x<10;x++)if(blocks[y]&(1<<x))oled.drawBox(49+x*3,y* (56.0f/20),2,2);uint16_t s=shapes[piece][rotation];for(int j=0;j<4;j++)for(int i=0;i<4;i++)if(s&(1<<(j*4+i)))oled.drawBox(49+(pieceX+i)*3,(pieceY+j)*(56.0f/20),2,2);}
 else if(arcadeKind==2){int h=ay?int(24*sin(PI*ay/18)):0;oled.drawHLine(0,54,128);oled.drawFrame(16,46-h,8,8);oled.drawTriangle(rockX,54,rockX+8,54,rockX+4,43);}
 else{for(int x=0;x<128;x++)oled.drawVLine(x,terrainHeight(x),57-terrainHeight(x));oled.drawHLine(51,53,27);
 int x=landX,y=landY;oled.drawFrame(x-3,y-4,7,6);oled.drawPixel(x-1,y-2);oled.drawBox(x-4,y+2,9,2);oled.drawLine(x-3,y+3,x-6,y+6);oled.drawLine(x+3,y+3,x+6,y+6);oled.drawHLine(x-8,y+6,4);oled.drawHLine(x+5,y+6,4);if(thrust&&fuel>0)oled.drawTriangle(x-2,y+4,x+2,y+4,x,y+8);text(19,"F "+String((int)fuel)+" "+(thrust?"ON":"OFF"));text(29,"Vy "+String(landVY,2));}
 if(arcadeOver){oled.setDrawColor(0);oled.drawBox(0,32,128,19);oled.setDrawColor(1);text(40,arcadeKind==3?(points?"Soft landing!":"Crashed / too fast"):arcadeKind==0&&points==18?"Fleet cleared!":"Game over");text(50,"SEL retry");}}
