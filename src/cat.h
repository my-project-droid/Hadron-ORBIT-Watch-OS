// Persistent care simulation. Age counts powered-on seconds, not calendar time.
struct CatState {uint32_t magic,age;uint8_t hunger,happy,poo,pee,sick,stage;};
CatState cat;int catAction=0;uint32_t catTick=0,catSave=0,evolveAt=0;String catMessage="Hello!";
bool catCareMenu=false; int catToy=0; uint32_t catEffectAt=0; int catEffect=-1;
// Food and room motion are deliberately RAM-only: leftovers vanish on reboot.
bool catFood=false;int catFoodBites=0,catRoomX=62;uint32_t catMotionAt=0,catBiteAt=0;
const int catBowlX=96,catEatX=77;
const char* careNames[]={"Feed","Pet","Clean","Medicine","Play / toy","Back to room"};
void saveCat(){EEPROM.put(808,cat);EEPROM.commit();catSave=millis();}
void loadCat(){EEPROM.get(808,cat);if(cat.magic!=0x43415432||cat.hunger>100||cat.happy>100||cat.stage>2){cat={0x43415432,0,30,70,0,0,0,0};saveCat();}catTick=millis();catFood=false;catFoodBites=0;catRoomX=62;catMotionAt=millis();catBiteAt=0;}
void careCat(){
 if(!catCareMenu){catCareMenu=true;catAction=0;dirty=true;return;}
 if(catAction==5){catCareMenu=false;dirty=true;return;}
 catEffect=catAction;catEffectAt=millis();
 if(catAction==0){if(!catFood){catFood=true;catFoodBites=5;catBiteAt=0;}catMessage=cat.hunger==0?"Full! Food stays":"Dinner time!";}
 else if(catAction==1){cat.happy=min(100,(int)cat.happy+20);catMessage="Purr...";}
 else if(catAction==2){cat.poo=cat.pee=0;catMessage="All clean";}
 else if(catAction==3){cat.sick=0;catMessage="Feeling better";}
 else{catToy=(catToy+1)%(cat.stage+1);cat.happy=min(100,(int)cat.happy+10);catMessage="Play time!";}
 saveCat();catCareMenu=false;dirty=true;
}
void catUpdate(){uint32_t now=millis(),seconds=(now-catTick)/1000;if(seconds){uint32_t old=cat.age;cat.age+=seconds;catTick+=seconds*1000;
 if(cat.age/600!=old/600){cat.hunger=min(100,(int)cat.hunger+3);cat.happy=cat.happy>2?cat.happy-2:0;}
 if(cat.age/1800!=old/1800)cat.pee=1;if(cat.age/3600!=old/3600)cat.poo=1;
 if(cat.hunger>85||((cat.poo||cat.pee)&&cat.age/7200!=old/7200))cat.sick=1;
 uint8_t stage=cat.age>=21UL*86400?2:cat.age>=7UL*86400?1:0;if(stage!=cat.stage){cat.stage=stage;evolveAt=now;saveCat();}
 if(now-catSave>=300000)saveCat();}
 // Walk toward the bowl before eating; only completed bites change hunger.
 if(now-catMotionAt>=180){catMotionAt=now;
  if(catFood&&cat.hunger>0){
   if(catRoomX!=catEatX){catRoomX+=catRoomX<catEatX?1:-1;catBiteAt=0;}
   else if(!catBiteAt){catBiteAt=now;}
   else if(now-catBiteAt>=900){catBiteAt=now;cat.hunger=cat.hunger>5?cat.hunger-5:0;
    catFoodBites--;catMessage="Nom nom!";if(catFoodBites<=0){catFood=false;catMessage="Yummy!";}saveCat();}
  }else{catBiteAt=0;uint32_t phase=(now/250)%120;int target=phase<32?48+(int)phase:phase<88?79:70;
   if(catRoomX!=target)catRoomX+=catRoomX<target?1:-1;}
 }
 if(page==PET&&now-lastFrame>=180){lastFrame=now;dirty=true;}}
// Compact monochrome character: white silhouette with black facial details.
void catHeart(int x,int y){
 oled.drawDisc(x-2,y,2);oled.drawDisc(x+2,y,2);
 oled.drawTriangle(x-4,y,x+4,y,x,y+5);
}
void catSprite(int x,int floorY,uint32_t frame,bool sleeping){
 int r=8+cat.stage;int headY=floorY-15+(frame%8==0?1:0);
 bool blink=frame%24>=22;
 // A curved tail, kept behind the body.
 int sway=(frame/2)%4;
 oled.drawLine(x+8,floorY-4,x+14,floorY-8);
 oled.drawLine(x+14,floorY-8,x+15,floorY-13+sway);
 oled.drawLine(x+15,floorY-13+sway,x+12,floorY-15+sway);
 if(sleeping){
  oled.drawEllipse(x,floorY-5,13,6);
  oled.drawDisc(x-7,floorY-8,7);
  oled.drawTriangle(x-14,floorY-10,x-13,floorY-19,x-7,floorY-13);
  oled.drawTriangle(x-7,floorY-13,x-3,floorY-18,x,floorY-10);
  oled.setDrawColor(0);
  oled.drawLine(x-12,floorY-8,x-9,floorY-7);
  oled.drawLine(x-6,floorY-7,x-3,floorY-8);
  oled.setDrawColor(1);
  oled.drawStr(x+9,floorY-19,(frame/3)%2?"z":"Z");
  return;
 }
 oled.drawEllipse(x,floorY-6,8,8);
 int stride=(frame%12==0?1:0);
 oled.drawDisc(x-5,floorY+(sleeping?0:stride),3);oled.drawDisc(x+5,floorY-(sleeping?0:stride),3);
 oled.drawDisc(x,headY,r);
 oled.drawTriangle(x-r,headY-2,x-r,headY-r-6,x-1,headY-r+1);
 oled.drawTriangle(x+r,headY-2,x+r,headY-r-6,x+1,headY-r+1);
 oled.setDrawColor(0);
 // Tiny inner ears, wide eyes and a smiling muzzle.
 oled.drawLine(x-r+2,headY-r-2,x-r+3,headY-r+1);
 oled.drawLine(x+r-2,headY-r-2,x+r-3,headY-r+1);
 if(cat.sick){oled.drawLine(x-6,headY-1,x-2,headY+1);oled.drawLine(x+2,headY+1,x+6,headY-1);}
 else if(cat.happy>=85){oled.drawLine(x-6,headY+1,x-4,headY-1);oled.drawLine(x-4,headY-1,x-2,headY+1);oled.drawLine(x+2,headY+1,x+4,headY-1);oled.drawLine(x+4,headY-1,x+6,headY+1);}
 else if(blink){oled.drawLine(x-5,headY,x-3,headY);oled.drawLine(x+3,headY,x+5,headY);}
 else{oled.drawDisc(x-4,headY,2);oled.drawDisc(x+4,headY,2);}
 oled.drawLine(x-1,headY+3,x+1,headY+3);
 // Rounded cheek dots and tiny chest fluff.
 oled.drawPixel(x-6,headY+4);oled.drawPixel(x+6,headY+4);
 oled.drawLine(x-2,floorY-7,x,floorY-5);oled.drawLine(x,floorY-5,x+2,floorY-7);
 oled.drawLine(x,headY+4,x-2,headY+5);oled.drawLine(x,headY+4,x+2,headY+5);
 oled.drawLine(x-8,headY+2,x-6,headY+3);oled.drawLine(x+6,headY+3,x+8,headY+2);
 oled.drawPixel(x-5,floorY);oled.drawPixel(x+5,floorY);
 oled.setDrawColor(1);
 if(!blink&&!cat.sick&&cat.happy<85){oled.drawPixel(x-4,headY-1);oled.drawPixel(x+4,headY-1);}
 oled.drawLine(x-r-3,headY+1,x-r,headY+2);
 oled.drawLine(x+r,headY+2,x+r+3,headY+1);
}
// Side-on walking silhouette, with alternating front and rear paws.
void catWalk(int x,int y,uint32_t f){
 int step=(f%4<2)?2:-2;
 int bob=f%4==0?1:0;y-=bob;
 oled.drawEllipse(x,y-7,10,6);oled.drawDisc(x+10,y-12,7);
 oled.drawTriangle(x+6,y-14,x+7,y-22,x+11,y-16);
 oled.drawTriangle(x+12,y-16,x+16,y-21,x+16,y-12);
 oled.drawLine(x-7,y-5,x-8+step,y);oled.drawLine(x-3,y-5,x-3-step,y);
 oled.drawLine(x+4,y-5,x+4+step,y);oled.drawLine(x+8,y-5,x+8-step,y);
 oled.drawLine(x-10,y-8,x-16,y-15);oled.drawLine(x-16,y-15,x-14,y-20);
 oled.setDrawColor(0);
 if(f%20>=18)oled.drawLine(x+11,y-13,x+13,y-13);else oled.drawDisc(x+12,y-13,1);
 oled.drawPixel(x+14,y-10);oled.drawLine(x+7,y-17,x+8,y-15);
 oled.setDrawColor(1);oled.drawDisc(x-8+step,y,1);oled.drawDisc(x+4+step,y,1);
 oled.drawLine(x+15,y-10,x+19,y-11);
}
void catBar(int x,const char* label,int value){oled.drawStr(x,7,label);oled.drawFrame(x,9,39,5);if(value)oled.drawBox(x+1,10,constrain(value,0,100)*37/100,3);}
void drawCat(){
 uint32_t now=millis(),f=now/250;bool effect=catEffectAt&&now-catEffectAt<4500;
 catBar(0,"Hunger",cat.hunger);catBar(44,"Happy",cat.happy);catBar(88,"Sick",cat.sick?100:0);
 if(catCareMenu){
  int start=catAction<3?0:3;
  for(int i=start;i<start+3;i++)text(27+(i-start)*11,String(i==catAction?"> ":"  ")+careNames[i]);
  return;
 }
 oled.drawHLine(0,49,128);
 if(cat.pee){oled.drawEllipse(109,48,7,1);oled.drawPixel(114,46);}
 if(cat.poo){oled.drawDisc(119,47,3);oled.drawDisc(119,44,2);oled.drawPixel(119,41);}
 // One, two, then three toys: ball, yarn and feather wand.
 oled.drawCircle(12,46,3);oled.drawLine(10,44,14,48);
 if(cat.stage>=1){oled.drawCircle(25,45,4);oled.drawLine(21,45,29,45);oled.drawLine(25,41,25,49);}
 if(cat.stage>=2){oled.drawLine(33,48,39,37);oled.drawLine(39,37,44,40);oled.drawTriangle(43,40,49,38,47,44);}
 uint32_t c=f%120;int x=catRoomX;
 bool eating=catFood&&cat.hunger>0&&x==catEatX;
 if(catFood&&cat.hunger>0){
  if(eating){catWalk(x,48+(f%4<2?0:1),f);if(f%4<2){oled.drawPixel(94,39);oled.drawPixel(98,37);}}
  else catWalk(x,48,f);
 }else if(effect&&catEffect==4){catSprite(x,47-(f%8<4?2:0),f,false);oled.drawCircle(x-14,44-(f%8<4?3:0),3);}
 else if(c<32){catWalk(x,48,f);}
 else if(c<48){catSprite(x,47,f,false);if(c%8<3)oled.drawStr(x+17,31,"m");}
 else if(c<88){catSprite(x,48,f,true);}
 else if(c<104){int h=(c<96?c-88:104-c)/2;catSprite(x,47-h,f,false);}
 else{catWalk(x,47-(f%6<3?1:0),f);}
 if(catFood){oled.drawLine(88,44,91,48);oled.drawHLine(91,48,11);oled.drawLine(102,48,105,44);oled.drawHLine(88,44,18);
  for(int i=0;i<catFoodBites;i++)oled.drawDisc(91+i*2,42-(i%2),1);
 }
 if(effect&&catEffect==1)catHeart(x+18,28-(f%4));
 if(effect&&catEffect==2){oled.drawCircle(104,33,2);oled.drawCircle(112,28,3);}
 if(effect&&catEffect==3){oled.drawStr(99,30,"+");catHeart(109,35);}
 bool evolving=evolveAt&&now-evolveAt<4000;
 text(56,evolving?"Level up! New toy!":eating?"Nom nom!":catFood&&cat.hunger==0?"Full / food stays":effect?catMessage:String("SEL care / ")+(cat.stage==0?"kitten":cat.stage==1?"teen":"adult"));
}
