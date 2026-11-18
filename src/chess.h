// Compact chess: legal king-safe moves, automatic queen promotion.
// Castling and en passant are not supported in this mini variant.
int8_t cb[64];int chessCursor=0,chessFrom=-1,chessSide=1,chessStage=0;bool chessAI=true,chessEnded=false;String chessStatus;
int wellMode=0,waterCups=0;uint32_t reminderAt=0;bool reminderOn=false;
void wellSelect(){if(wellMode==0)waterCups=(waterCups+1)%17;else if(wellMode==1){reminderOn=!reminderOn;reminderAt=millis()+30UL*60000;}else keyboardStart(4,"Quick journal note");}
void wellRender(){text(9,"Everyday tools");text(23,wellMode==0?"Water counter":wellMode==1?"Stretch reminder":"Quick journal");text(38,wellMode==0?String(waterCups)+" cups (manual)":wellMode==1?(reminderOn?(int32_t(millis()-reminderAt)>=0?"Time to stretch!":String((reminderAt-millis())/60000)+" minutes left"):"SEL: start 30 minutes"):"SEL: write in diary slot");text(53,"+/- tool; SEL action");}
bool chessPseudo(const int8_t *b,int f,int t,bool attack=false){if(f==t)return false;int p=b[f],side=p>0?1:-1,k=abs(p),dx=t%8-f%8,dy=t/8-f/8;if(!p||(!attack&&b[t]*side>0))return false;
 if(k==1){int d=-side;if(attack)return abs(dx)==1&&dy==d;if(dx==0&&b[t]==0){if(dy==d)return true;if(dy==2*d&&f/8==(side==1?6:1)&&b[f+d*8]==0)return true;}return abs(dx)==1&&dy==d&&b[t]*side<0;}
 if(k==2)return abs(dx)*abs(dy)==2;
 if(k==6)return max(abs(dx),abs(dy))==1;
 bool line=(k==3?abs(dx)==abs(dy):k==4?(dx==0||dy==0):(dx==0||dy==0||abs(dx)==abs(dy)));if(!line)return false;
 int step=(dx>0)-(dx<0)+8*((dy>0)-(dy<0));for(int i=f+step;i!=t;i+=step)if(b[i])return false;return true;}
bool chessCheck(const int8_t *b,int side){int king=-1;for(int i=0;i<64;i++)if(b[i]==side*6)king=i;if(king<0)return true;for(int i=0;i<64;i++)if(b[i]*side<0&&chessPseudo(b,i,king,true))return true;return false;}
bool castleRights[4]={true,true,true,true};int epSquare=-1,promotionSquare=-1,promotionChoice=0;
const int promotionKinds[]={5,4,3,2};
bool attacked(const int8_t*b,int sq,int side){for(int i=0;i<64;i++)if(b[i]*side<0&&chessPseudo(b,i,sq,true))return true;return false;}
bool chessCastle(int f,int t,int side){
 int home=side==1?60:4;if(f!=home||abs(t-f)!=2||cb[f]!=side*6||chessCheck(cb,side))return false;
 bool right=t>f;int ri=(side==1?0:2)+(right?0:1),rook=home+(right?3:-4),step=right?1:-1;
 if(!castleRights[ri]||cb[rook]!=side*4)return false;
 for(int i=f+step;i!=rook;i+=step)if(cb[i])return false;
 int8_t b[64];memcpy(b,cb,64);b[f]=0;
 for(int i=1;i<=2;i++){int q=f+i*step;b[q]=side*6;if(attacked(b,q,side))return false;b[q]=0;}return true;
}
bool chessLegal(int f,int t,int side){
 if(cb[f]*side<=0||abs(cb[t])==6)return false;
 bool castle=abs(cb[f])==6&&chessCastle(f,t,side);
 bool ep=abs(cb[f])==1&&t==epSquare&&cb[t]==0&&abs(t%8-f%8)==1&&t/8-f/8==-side&&cb[t+side*8]==-side;
 if(!castle&&!ep&&!chessPseudo(cb,f,t))return false;
 int8_t b[64];memcpy(b,cb,64);b[t]=b[f];b[f]=0;if(ep)b[t+side*8]=0;
 if(castle){int r=f+(t>f?3:-4);b[(f+t)/2]=b[r];b[r]=0;}
 return !chessCheck(b,side);
}
void chessReset(){memset(cb,0,64);int back[]={4,2,3,5,6,3,2,4};for(int i=0;i<8;i++){cb[i]=-back[i];cb[i+8]=-1;cb[48+i]=1;cb[56+i]=back[i];}for(int i=0;i<4;i++)castleRights[i]=true;epSquare=-1;promotionSquare=-1;chessCursor=48;chessFrom=-1;chessSide=1;chessStage=0;chessEnded=false;chessStatus="White to move";}
void chessFinish(){chessSide=-chessSide;chessFrom=-1;bool any=false;for(int a=0;a<64&&!any;a++)for(int b=0;b<64;b++)if(chessLegal(a,b,chessSide)){any=true;break;}chessEnded=!any;chessStatus=!any?(chessCheck(cb,chessSide)?"Checkmate":"Stalemate"):chessCheck(cb,chessSide)?"Check!":chessSide==1?"White to move":"Black to move";}
void chessApply(int f,int t){int p=cb[f];if(abs(p)==1&&t==epSquare&&!cb[t]&&f%8!=t%8)cb[t+chessSide*8]=0;
 if(abs(p)==6){int off=p>0?0:2;castleRights[off]=castleRights[off+1]=false;if(abs(t-f)==2){int r=f+(t>f?3:-4);cb[(f+t)/2]=cb[r];cb[r]=0;}}
 const int rs[]={63,56,7,0};for(int i=0;i<4;i++)if(f==rs[i]||t==rs[i])castleRights[i]=false;
 epSquare=abs(p)==1&&abs(t-f)==16?(f+t)/2:-1;cb[t]=p;cb[f]=0;
}
void chessCommit(int f,int t){chessApply(f,t);if(abs(cb[t])==1&&(t/8==0||t/8==7)){if(chessAI&&chessSide==-1)cb[t]=-5;else{promotionSquare=t;promotionChoice=0;return;}}chessFinish();}
void chessMove(int d){if(promotionSquare>=0)promotionChoice=(promotionChoice+d+4)%4;else if(chessStage==0)chessAI=!chessAI;else chessCursor=(chessCursor+d+64)%64;}
void chessSelect(){if(promotionSquare>=0){cb[promotionSquare]=chessSide*promotionKinds[promotionChoice];promotionSquare=-1;chessFinish();return;}if(chessStage==0){chessStage=1;return;}if(chessEnded){chessReset();return;}if(chessAI&&chessSide==-1)return;if(chessFrom<0){if(cb[chessCursor]*chessSide>0)chessFrom=chessCursor;}else if(chessCursor==chessFrom)chessFrom=-1;else if(chessLegal(chessFrom,chessCursor,chessSide))chessCommit(chessFrom,chessCursor);else chessStatus="Illegal target";}
struct ChessState{int8_t b[64];bool rights[4];int ep,side;};
ChessState chessSnapshot(){ChessState s;memcpy(s.b,cb,64);memcpy(s.rights,castleRights,4);s.ep=epSquare;s.side=chessSide;return s;}
void chessRestore(const ChessState&s){memcpy(cb,s.b,64);memcpy(castleRights,s.rights,4);epSquare=s.ep;chessSide=s.side;}
const int chessValues[]={0,100,320,330,500,900,20000};
int searchNodes=0;uint32_t searchStart;bool searchAbort;
int chessEval(int side){int score=0;for(int i=0;i<64;i++)if(cb[i]){int center=6-abs(3-i%8)-abs(3-i/8);score+=(cb[i]>0?1:-1)*(chessValues[abs(cb[i])]+(abs(cb[i])==6?0:center*3));}return score*side;}
int chessSearch(int depth,int alpha,int beta,int side){if(++searchNodes%128==0){yield();if(millis()-searchStart>650)searchAbort=true;}if(searchAbort||depth==0)return chessEval(side);int best=-30000;bool any=false;
 for(int pass=0;pass<2;pass++)for(int f=0;f<64;f++)if(cb[f]*side>0)for(int t=0;t<64;t++){if((cb[t]!=0)!=(pass==0)||!chessLegal(f,t,side))continue;any=true;ChessState old=chessSnapshot();chessSide=side;chessApply(f,t);if(abs(cb[t])==1&&(t/8==0||t/8==7))cb[t]=side*5;int v=-chessSearch(depth-1,-beta,-alpha,-side);chessRestore(old);best=max(best,v);alpha=max(alpha,v);if(alpha>=beta||searchAbort)return best;}
 return any?best:chessCheck(cb,side)?-29000-depth:0;
}
void chessTick(){if(page!=CHESS||chessStage==0||!chessAI||chessSide!=-1||chessEnded||promotionSquare>=0)return;
 searchStart=millis();searchNodes=0;searchAbort=false;int bf=-1,bt=-1;
 for(int depth=1;depth<=difficulty+2;depth++){int best=-32000,df=-1,dt=-1;for(int f=0;f<64&&!searchAbort;f++)for(int t=0;t<64&&!searchAbort;t++)if(chessLegal(f,t,-1)){ChessState old=chessSnapshot();chessApply(f,t);if(cb[t]==-1&&t/8==7)cb[t]=-5;int v=-chessSearch(depth-1,-32000,32000,1);chessRestore(old);if(v>best){best=v;df=f;dt=t;}}if(!searchAbort&&df>=0){bf=df;bt=dt;}if(searchAbort)break;}
 if(bf>=0)chessCommit(bf,bt);dirty=true;
}
void oldChessRender(){if(chessStage==0){text(10,"Mini chess");text(25,chessAI?"> Against computer":"> Two people: pass watch");text(39,"+/- mode; SEL start");text(52,"No castling/en passant");return;}for(int i=0;i<64;i++){int x=(i%8)*7,y=(i/8)*7;if((i%8+i/8)%2)oled.drawPixel(x+6,y+6);if(i==chessCursor)oled.drawFrame(x,y,7,7);if(i==chessFrom)oled.drawHLine(x,y+6,6);if(cb[i]){char c=" PNBRQK"[abs(cb[i])];if(cb[i]<0)c=tolower(c);oled.drawStr(x+1,y+6,String(c).c_str());}}oled.drawStr(60,8,chessSide==1?"White":"Black");oled.drawStr(60,20,chessAI?"CPU":"2 players");oled.drawStr(60,32,chessFrom<0?"Pick piece":"Pick target");oled.drawStr(60,44,chessStatus.substring(0,13).c_str());}

const char* chessPieceName(int p){const char* n[]={"empty","Pawn","Knight","Bishop","Rook","Queen","King"};return n[abs(p)];}
void chessRender(){
 if(promotionSquare>=0){text(10,"Pawn promotion");text(27,String("> ")+chessPieceName(promotionKinds[promotionChoice]));text(43,"+/- choose; SEL confirm");return;}
 if(chessStage==0){text(8,"CHESS / choose mode");text(20,chessAI?"> Computer search":"> Pass watch: 2 people");text(32,"+/- changes; SEL starts");text(44,"Uppercase=White, lower=Black");text(56,"Castle / promotion / EP");return;}
 oled.setFont(u8g2_font_4x6_tf);
 for(int i=0;i<64;i++){int x=i%8*7,y=i/8*7;bool light=(i%8+i/8)%2==0;
 if(light)oled.drawBox(x,y,7,7);else oled.drawFrame(x,y,7,7);
 oled.setDrawColor(light?0:1);
 if(cb[i]){char c=" PNBRQK"[abs(cb[i])];if(cb[i]<0)c=tolower(c);oled.drawStr(x+1,y+6,String(c).c_str());}
 if(chessFrom>=0&&chessLegal(chessFrom,i,chessSide))oled.drawBox(x+4,y+1,2,2);
 if(i==chessCursor)oled.drawFrame(x,y,7,7);
 if(i==chessFrom)oled.drawHLine(x+1,y+6,5);oled.setDrawColor(1);
 }
 oled.setFont(u8g2_font_5x8_tf);
 oled.drawStr(67,8,chessSide==1?"WHITE":"BLACK");
 String sq=String(char('a'+chessCursor%8))+String(8-chessCursor/8);
 oled.drawStr(67,19,(sq+" "+chessPieceName(cb[chessCursor])).c_str());
 oled.drawStr(67,30,chessFrom<0?"SEL pick":"SEL move");
 oled.drawStr(67,41,chessFrom<0?"+/- square":"Dots: legal");
 oled.drawStr(67,52,chessStatus.substring(0,12).c_str());
 oled.drawStr(67,63,"Hold: back");
}
