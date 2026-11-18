#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
// Compact on-watch text entry. No credentials are persisted.
String typed,typingTitle,joinSSID,latitude="",longitude="",appStatus="Select: choose Wi-Fi",factAnswer;
int keyIndex=0,keyRow=0,keyLayer=0,typeTask=0,weatherStage=0,answerOffset=0;
bool keyPicking=false,joinWaiting=false,joinPending=false,weatherScan=false;
uint32_t joinStarted=0;
WiFiEventHandler joinDisconnectEvent;int lastJoinReason=0;
void setupJoinEvents(){joinDisconnectEvent=WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected& e){lastJoinReason=e.reason;});}
String joinPassword;
uint8_t joinBssid[6];int joinChannel=1;bool joinHasBssid=false;
const char* keyRows[3][5]={
 {"qwertyuiop","asdfghjkl","zxcvbnm","0123456789",""},
 {"QWERTYUIOP","ASDFGHJKL","ZXCVBNM","0123456789",""},
 {"!@#$%^&*()","-_=+[]{}",";:'\"`,.<>","/?\\|~",""}
};
const char* keyActions[]={"SPACE","DEL","CASE","123/#","DONE"};
int keyboardSize(){return keyRow==4?5:strlen(keyRows[keyLayer][keyRow]);}
void keyboardStart(int task,String title,String initial=""){typeTask=task;typingTitle=title;typed=initial;keyIndex=0;keyRow=0;keyLayer=0;keyPicking=false;page=KEYBOARD;backFocus=false;dirty=true;}
// Original compact ELIZA-style rules, not copied third-party code or an LLM.
String reflectEliza(String s){s=" "+s+" ";s.replace(" my "," your ");s.replace(" am "," are ");s.replace(" me "," you ");s.replace(" i "," you ");s.trim();return s;}
String answerFact(String q){q.toLowerCase();q.trim();
 if(q.indexOf("hello")>=0||q=="hi")return "Hello. What would you like to talk about?";
 if(q.indexOf("computer")>=0||q.indexOf(" ai")>=0)return "I use scripted ELIZA-style rules, not real understanding. What interests you about computers?";
 const char* keys[]={"i feel ","i am ","i want ","i need ","my ","because ","can you ","i can't "};
 const char* prompts[]={"Why do you feel ","How long have you been ","What would it mean to have ","Why do you need ","Tell me more about your ","Is that the only reason: ","What makes you want me to ","What would help you to "};
 for(int i=0;i<8;i++){int n=q.indexOf(keys[i]);if(n>=0)return String(prompts[i])+reflectEliza(q.substring(n+strlen(keys[i])))+"?";}
 if(q.indexOf("mother")>=0||q.indexOf("father")>=0)return "Tell me more about your family.";
 if(q.indexOf("yes")>=0)return "You seem quite sure. Can you explain?";
 const char* defaults[]={"Please tell me more.","How does that make you feel?","What is most important to you about that?","Can you give an example?"};return defaults[random(4)];}
bool coordinateValid(String s,float lo,float hi){if(!s.length())return false;bool digit=false,dot=false;for(unsigned i=0;i<s.length();i++){char c=s[i];if(c>='0'&&c<='9')digit=true;else if(c=='.'&&!dot)dot=true;else if(c=='-'&&i==0){}else return false;}float n=s.toFloat();return digit&&n>=lo&&n<=hi;}
String jsonNumber(String body,const char* key){int p=body.lastIndexOf(String("\"")+key+"\"");if(p<0)return "?";p=body.indexOf(':',p)+1;while(p<(int)body.length()&&body[p]==' ')p++;int end=p;while(end<(int)body.length()&&(isDigit(body[end])||body[end]=='-'||body[end]=='.'))end++;return end>p?body.substring(p,end):"?";}
void fetchWeather(){if(WiFi.status()!=WL_CONNECTED){appStatus="Not connected. Select: Wi-Fi";return;}appStatus="Fetching weather...";dirty=true;render();
 BearSSL::WiFiClientSecure client;client.setInsecure(); // Public weather only; never send secrets to this endpoint.
 client.setTimeout(6000);HTTPClient http;http.setTimeout(6000);
 String url="https://api.open-meteo.com/v1/forecast?latitude="+latitude+"&longitude="+longitude+"&current=temperature_2m,wind_speed_10m,weather_code";
 if(!http.begin(client,url)){appStatus="Weather connection failed";return;}
 int code=http.GET();if(code==200&&http.getSize()<8192){String body=http.getString();appStatus="Temp "+jsonNumber(body,"temperature_2m")+" C\nWind "+jsonNumber(body,"wind_speed_10m")+" km/h\nCode "+jsonNumber(body,"weather_code");}else appStatus="Weather failed: "+String(code);http.end();dirty=true;}
void keyboardDone(){if(typeTask==0){if(typed.length()!=0 && (typed.length()<8 || typed.length()>63)){typingTitle="Password: 8-63 chars";return;}
 lastJoinReason=0;joinPassword=typed;typed="";stopMonitor();stopRadio();stopWeb();
 wifi_promiscuous_enable(0);WiFi.scanDelete();weatherScan=false;
 WiFi.persistent(false);WiFi.setAutoReconnect(false);WiFi.mode(WIFI_STA);
 WiFi.disconnect();wifi_set_sleep_type(NONE_SLEEP_T);wifi_set_opmode(STATION_MODE);joinPending=true;joinWaiting=true;joinStarted=millis();
 appStatus="Resetting Wi-Fi...";page=WEATHER;}
 else if(typeTask==1){if(!coordinateValid(typed,-90,90)){typed="";typingTitle="Invalid latitude; retry";return;}latitude=typed;keyboardStart(2,"Longitude (-180..180)");return;}
 else if(typeTask==2){if(!coordinateValid(typed,-180,180)){typed="";typingTitle="Invalid longitude; retry";return;}longitude=typed;page=WEATHER;fetchWeather();}
 else if(typeTask==5){message=radio&&esp_now_send(broadcastMac,(uint8_t*)typed.c_str(),typed.length())==0?"Queued: "+typed:"Send failed";typed="";page=COMMS;}
 else if(typeTask==4){writeNote(typed);typed="";}
 else{factAnswer=answerFact(typed);answerOffset=0;page=FACTS;}dirty=true;}
void connectedSelect(){if(page==KEYBOARD){
 if(!keyPicking){keyPicking=true;keyIndex=0;}
 else if(keyRow==4){
  if(keyIndex==0){if(typed.length()<(typeTask==5?31:typeTask==0?63:typeTask==4?200:typeTask==3?100:12))typed+=' ';}
  else if(keyIndex==1){if(typed.length())typed.remove(typed.length()-1);}
  else if(keyIndex==2){keyLayer=keyLayer==1?0:1;}
  else if(keyIndex==3){keyLayer=keyLayer==2?0:2;}
  else {keyboardDone();return;}
  keyPicking=false;
 }else{if(typed.length()<(typeTask==5?31:typeTask==0?63:typeTask==4?200:typeTask==3?100:12))typed+=keyRows[keyLayer][keyRow][keyIndex];keyPicking=false;}
 }
 else if(page==JOIN_LIST){if(count>0){joinSSID=WiFi.SSID(net);joinChannel=WiFi.channel(net);const uint8_t* mac=WiFi.BSSID(net);joinHasBssid=mac!=nullptr;if(mac)memcpy(joinBssid,mac,6);if(!joinSSID.length()){appStatus="Hidden SSID unsupported";page=WEATHER;}else if(WiFi.encryptionType(net)==ENC_TYPE_NONE){typed="";typeTask=0;keyboardDone();}else keyboardStart(0,"Wi-Fi password");}}
 else if(page==WEATHER){if(joinWaiting)return;if(WiFi.status()==WL_CONNECTED)keyboardStart(1,"Latitude (-90..90)",latitude);else{stopMonitor();stopRadio();stopWeb();wifi_promiscuous_enable(0);WiFi.mode(WIFI_STA);WiFi.scanDelete();WiFi.scanNetworks(true,false);weatherScan=true;page=JOIN_SCAN;}}
 else if(page==FACTS)keyboardStart(3,"ELIZA: your message");dirty=true;}
void connectedMove(int d){if(page==KEYBOARD){if(keyPicking)keyIndex=(keyIndex+d+keyboardSize())%keyboardSize();else keyRow=(keyRow+d+5)%5;}else if(page==JOIN_LIST&&count>0)net=(net+d+count)%count;else if(page==FACTS){answerOffset=constrain(answerOffset+d*20,0,max(0,(int)factAnswer.length()-20));}dirty=true;}
void connectedTick(){if(weatherScan){int n=WiFi.scanComplete();if(n!=WIFI_SCAN_RUNNING){weatherScan=false;count=max(0,n);net=0;if(page==JOIN_SCAN)page=JOIN_LIST;dirty=true;}}
 if(joinWaiting){
  if(joinPending){if(millis()-joinStarted>=250){joinPending=false;WiFi.begin(joinSSID.c_str(),joinPassword.c_str(),joinChannel,joinHasBssid?joinBssid:nullptr,true);joinPassword="";joinStarted=millis();appStatus="Joining 2.4 GHz\n"+joinSSID.substring(0,25);dirty=true;}return;}
  int st=WiFi.status();
  if(st==WL_CONNECTED){joinWaiting=false;appStatus="Connected. SEL: location";dirty=true;}
  else if((millis()-joinStarted>2000&&(st==WL_CONNECT_FAILED||st==WL_WRONG_PASSWORD))||millis()-joinStarted>30000){
   joinWaiting=false;
   appStatus=st==WL_WRONG_PASSWORD?"Wrong password\nSEL: choose Wi-Fi":st==WL_NO_SSID_AVAIL?"Network not found\nUse nearby 2.4 GHz Wi-Fi":st==WL_CONNECT_FAILED?"Join rejected\nCheck password / WPA2":"Join timeout / reason "+String(lastJoinReason)+"\nTry WPA2 2.4 GHz hotspot";
   WiFi.disconnect();dirty=true;
  }
 }}

void connectedRender(){if(page==KEYBOARD){
 text(8,typingTitle.substring(0,25));
 text(17,typeTask==0?String(typed.length())+" chars (case sensitive)":typed.substring(max(0,(int)typed.length()-25)));
 for(int r=0;r<4;r++){
  int y=25+r*8;String row=keyRows[keyLayer][r];
  for(int c=0;c<(int)row.length();c++){
   int x=14+c*11;bool chosen=r==keyRow&&(!keyPicking||c==keyIndex);
   if(chosen){oled.drawBox(x-1,y-7,10,8);oled.setDrawColor(0);}
   oled.drawStr(x,y,String(row[c]).c_str());oled.setDrawColor(1);
  }
  if(r==keyRow)oled.drawStr(0,y,keyPicking?"*":">");
 }
 text(57,keyRow==4?(keyPicking?String("< ")+keyActions[keyIndex]+" >":"SPACE DEL CASE # DONE"):(keyPicking?"+/- key; SEL type":"+/- row; SEL choose"));
 }
 else if(page==JOIN_SCAN)text(25,"Scanning Wi-Fi...");
 else if(page==JOIN_LIST){text(9,"Choose Internet Wi-Fi");if(!count)text(28,"None found. Back / retry");for(int i=0;i<3&&net+i<count;i++)text(25+i*12,String(i==0?"> ":"  ")+WiFi.SSID(net+i).substring(0,22));}
 else if(page==FACTS){text(9,"ELIZA rules / SEL chat");for(int i=0;i<3;i++)text(25+i*12,factAnswer.substring(answerOffset+i*25,answerOffset+(i+1)*25));}
 else{ text(9,"Weather / SEL setup");int start=0;for(int i=0;i<3;i++){int end=appStatus.indexOf('\n',start);if(end<0)end=appStatus.length();text(25+i*12,appStatus.substring(start,end));start=end+1;if(start>=(int)appStatus.length())break;}}}
