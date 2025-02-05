#define FIRESTORE 1
#define REALTIME 2

#define DB_TYPE FIRESTORE

#if DB_TYPE == FIRESTORE
#include "main_firestore.h"
#elif DB_TYPE == REALTIME
#include "main_realtime.h"
#else
#include <Arduino.h>
void setup() {}
void loop() {}
#endif