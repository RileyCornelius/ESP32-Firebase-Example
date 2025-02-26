/**
 * ABOUT:
 *
 * The non-blocking (async) example to create the Firestore document.
 *
 * This example uses the UserAuth class for authentication, and the DefaultNetwork class for network interface configuration.
 * See examples/App/AppInitialization and examples/App/NetworkInterfaces for more authentication and network examples.
 *
 * The complete usage guidelines, please read README.md or visit https://github.com/mobizt/FirebaseClient
 *
 * SYNTAX:
 *
 * 1.------------------------
 *
 * Firestore::Documents::createDocument(<AsyncClient>, <Firestore::Parent>, <documentPath>, <DocumentMask>, <Document>, <AsyncResultCallback>, <uid>);
 *
 * Firestore::Documents::createDocument(<AsyncClient>, <Firestore::Parent>, <collectionId>, <documentId>, <DocumentMask>, <Document>, <AsyncResultCallback>, <uid>);
 *
 * <AsyncClient> - The async client.
 * <Firestore::Parent> - The Firestore::Parent object included project Id and database Id in its constructor.
 * <documentPath> - The relative path of document to create in the collection.
 * <DocumentMask> - The fields to return. If not set, returns all fields. Use comma (,) to separate between the field names.
 * <collectionId> - The relative path of document collection id to create the document.
 * <documentId> - The document id of document to be created.
 * <Document> - The Firestore document.
 * <AsyncResultCallback> - The async result callback (AsyncResultCallback).
 * <uid> - The user specified UID of async result (optional).
 *
 * The Firebase project Id should be only the name without the firebaseio.com.
 * The Firestore database id should be (default) or empty "".
 */

#include <Arduino.h>
#include <LittleFS.h>
#include <FirebaseClient.h>
#include <WiFiClientSecure.h>
#include <sys/time.h>

#include <Benchmark.h>
#include <SimpleTimer.h>

#include <CredentialsManager/CredentialsManager.h>

static const char *WIFI_SSID;
static const char *WIFI_PASSWORD;

static const char *API_KEY;
static const char *USER_EMAIL;
static const char *USER_PASSWORD;
static const char *FIREBASE_PROJECT_ID;

DefaultNetwork network; // initialize with boolean parameter to enable/disable network reconnection

FirebaseApp app;
WiFiClientSecure sslClient;

using AsyncClient = AsyncClientClass;
AsyncClient aClient(sslClient, getNetwork(network));
Firestore::Documents Docs;

FirebaseCredential firebaseCredential;
WifiCredential wifiCredential;

void asyncCB(AsyncResult &aResult);
void printResult(AsyncResult &aResult);
String getTimestampString();

void setup()
{
    Serial.begin(115200);
    delay(3000); // wait for the serial monitor to connect
    Serial.println("Starting...");

    // Read configuration files
    if (!LittleFS.begin())
    {
        Serial.println("An Error has occurred while mounting LittleFS");
        return;
    }

    CredentialsManager credentialsManager(LittleFS);
    firebaseCredential = credentialsManager.getFirebaseCredential();
    wifiCredential = credentialsManager.getWifiCredential();

    if (wifiCredential.isEmpty())
    {
        Serial.println("Failed to read configuration file");
        return;
    }
    WIFI_SSID = wifiCredential.ssid.c_str();
    WIFI_PASSWORD = wifiCredential.password.c_str();

    if (firebaseCredential.isEmpty())
    {
        Serial.println("Firebase configuration is empty");
        return;
    }

    // API_KEY = firebaseCredential.apiKey.c_str();
    // USER_EMAIL = firebaseCredential.userEmail.c_str();
    // USER_PASSWORD = firebaseCredential.userPassword.c_str();
    FIREBASE_PROJECT_ID = firebaseCredential.projectId.c_str();

    // Connect to Wi-Fi
    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());

    // Setup Firebase
    Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);
    sslClient.setInsecure();

    Serial.println("Initializing the app...");
    // UserAuth userAuth(API_KEY, USER_EMAIL, USER_PASSWORD);
    NoAuth userAuth;
    initializeApp(aClient, app, getAuth(userAuth), asyncCB, "authTask");
    app.getApp<Firestore::Documents>(Docs);
    Serial.println("Initialized the app");

    // Set time using NTP server
    tm timeinfo;
    configTzTime("UTC0", "0.pool.ntp.org", "1.pool.ntp.org", "2.pool.ntp.org");
    getLocalTime(&timeinfo);
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void loop()
{
    // The async task handler should run inside the main loop
    // without blocking delay or bypassing with millis code blocks.
    app.loop();
    Docs.loop();

    static unsigned long dataMillis = 0;
    if (app.ready() && (millis() - dataMillis > 10000 || dataMillis == 0))
    {
        dataMillis = millis();
        // In the console, you can create the ancestor document "example_collection/doc_1" before running this example
        // to avoid non-existent ancestor documents case.
        String documentPath = "example_collection/doc_1/data_1";

        Values::TimestampValue timestamp(getTimestampString());
        Values::StringValue deviceId(WiFi.macAddress());
        Values::IntegerValue temperature(random(100));
        Values::IntegerValue humidity(random(100));

        Document doc;
        doc.add("timestamp", Values::Value(timestamp));
        doc.add("deviceId", Values::Value(deviceId));
        doc.add("temperature", Values::Value(temperature));
        doc.add("humidity", Values::Value(humidity));

        // The value of Values::xxxValue, Values::Value and Document can be printed on Serial.
        Serial.println("Creating a document... ");
        BENCHMARK_MICROS_BEGIN(Created);
        Docs.createDocument(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), documentPath, DocumentMask(), doc, asyncCB, "createDocumentTask");
        BENCHMARK_MICROS_END(Created);
    }
}

void asyncCB(AsyncResult &aResult) { printResult(aResult); }

void printResult(AsyncResult &aResult)
{
    if (aResult.isEvent())
    {
        Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.appEvent().message().c_str(), aResult.appEvent().code());
    }

    if (aResult.isDebug())
    {
        Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());
    }

    if (aResult.isError())
    {
        Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());
    }

    if (aResult.available())
    {
        Firebase.printf("task: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());
    }
}

String getTimestampString()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t now = tv.tv_sec;
    struct tm ts = *localtime(&now);
    char buf[100];
    sprintf(buf, "%04d-%02d-%02dT%02d:%02d:%02d.%06ldZ",
            ts.tm_year + 1900, ts.tm_mon + 1, ts.tm_mday,
            ts.tm_hour, ts.tm_min, ts.tm_sec, tv.tv_usec);
    return String(buf);
}