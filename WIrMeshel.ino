//************************************************************
// this is a simple example that uses the painlessMesh library
//
// 1. sends a silly message to every node on the mesh at a random time between 1 and 5 seconds
// 2. prints anything it receives to Serial.print
//
//
//************************************************************
#include "painlessMesh.h"

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

bool flag=false;  	// Флаг что импульс посчитан
long Summ=0; 		    // Количество импульсов
int imp_port=0;		  // Порт импульсного входа gpio0
int ad = 200;        // Время для проверки антидребезга в мс
long millis_ad = 0;       // переменная для хранения millis() проверки антидребезга

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain

Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );


void sendMessage() {
  String msg = "Node        ";
  msg += mesh.getNodeId();
  msg += ";        impulse        ";
  msg += Summ;
  msg += ";        myFreeMemory: " + String(ESP.getFreeHeap());
  
  mesh.sendBroadcast(msg);

  Serial.printf("Sending message: %s\n", msg.c_str());
  
  taskSendMessage.setInterval( random(TASK_SECOND * 60, TASK_SECOND * 100));  // between 1 and 5 seconds
}


// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
    Serial.printf("Changed connections %s\n",mesh.subConnectionJson().c_str());
}

void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void setup() {
  Serial.begin(115200);
  pinMode (imp_port, INPUT_PULLUP);

//mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();
}

void loop() {
  userScheduler.execute(); // it will run mesh scheduler as well
  mesh.update();
  
  if ((digitalRead(imp_port)== LOW )&&( flag==false))  
	{
		flag=true;
		Summ++;
    millis_ad = millis();
    //Serial.printf("port LOW, Summ =  %u\n", Summ);
    sendMessage();
	}
 
  if ((digitalRead(imp_port)== HIGH )&&(flag==true)&&(ad < millis()- millis_ad))   flag=false;
    
}
