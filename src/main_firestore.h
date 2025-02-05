
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

#include <Benchmark.h>
#include <SimpleTimer.h>
#include <CredentialsManager.h>

static const char *WIFI_SSID;
static const char *WIFI_PASSWORD;

static const char *API_KEY;
static const char *USER_EMAIL;
static const char *USER_PASSWORD;
static const char *DATABASE_URL;
static const char *FIREBASE_PROJECT_ID;

DefaultNetwork network; // initilize with boolean parameter to enable/disable network reconnection

// UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD);
NoAuth user_auth;
FirebaseApp app;
WiFiClientSecure ssl_client;

using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client, getNetwork(network));
Firestore::Documents Docs;

FirebaseCredential firebaseCredential;
WifiCredential wifiCredential;

void asyncCB(AsyncResult &aResult);
void printResult(AsyncResult &aResult);
String getTimestampString(uint64_t sec, uint32_t nano);

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

    API_KEY = firebaseCredential.apiKey.c_str();
    USER_EMAIL = firebaseCredential.userEmail.c_str();
    USER_PASSWORD = firebaseCredential.userPassword.c_str();
    DATABASE_URL = firebaseCredential.realtimeDbUrl.c_str();
    FIREBASE_PROJECT_ID = firebaseCredential.projectId.c_str();

    // Connect to Wi-Fi

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
    Serial.println();

    // Setup Firebase

    Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);
    ssl_client.setInsecure();

    Serial.println("Initializing the app...");
    initializeApp(aClient, app, getAuth(user_auth), asyncCB, "authTask");

    // Binding the FirebaseApp for authentication handler.
    // To unbind, use Docs.resetApp();
    app.getApp<Firestore::Documents>(Docs);
}

void loop()
{
    // The async task handler should run inside the main loop
    // without blocking delay or bypassing with millis code blocks.

    app.loop();
    Docs.loop();

    static int data_count = 0;
    static unsigned long dataMillis = 0;
    if (app.ready() && (millis() - dataMillis > 20000 || dataMillis == 0))
    {
        dataMillis = millis();

        // We will create the documents in this parent path "test_doc_creation/doc_1/col_1/data_?"
        // (collection > document > collection > documents that contains fields).

        // Note: If new document created under non-existent ancestor documents as in this example
        // which the document "test_doc_creation/doc_1" does not exist, that document (doc_1) will not appear in queries and snapshot
        // https://cloud.google.com/firestore/docs/using-console#non-existent_ancestor_documents.

        // In the console, you can create the ancestor document "test_doc_creation/doc_1" before running this example
        // to avoid non-existent ancestor documents case.

        String documentPath = "example_collection/doc_1/data_";
        documentPath += data_count;
        // data_count++;

        Values::TimestampValue timestamp(getTimestampString(dataMillis, 0));
        Values::DoubleValue temperature(number_t(random(100), 6));
        Values::DoubleValue humidity(number_t(random(100), 6));

        Document<Values::Value> doc("temperature", Values::Value(temperature));
        doc.add("humidity", Values::Value(humidity));
        doc.add("timestamp", Values::Value(timestamp));

        // The value of Values::xxxValue, Values::Value and Document can be printed on Serial.
        Serial.println("Creating a document... ");
        BENCHMARK_MICROS_BEGIN(Create);
        Docs.createDocument(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), documentPath, DocumentMask(), doc, asyncCB, "createDocumentTask");
        BENCHMARK_MICROS_END(Create);
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

String getTimestampString(uint64_t sec, uint32_t nano)
{
    if (sec > 0x3afff4417f)
        sec = 0x3afff4417f;

    if (nano > 0x3b9ac9ff)
        nano = 0x3b9ac9ff;

    time_t now;
    struct tm ts;
    char buf[80];
    now = sec;
    ts = *localtime(&now);

    String format = "%Y-%m-%dT%H:%M:%S";

    if (nano > 0)
    {
        String fraction = String(double(nano) / 1000000000.0f, 9);
        fraction.remove(0, 1);
        format += fraction;
    }
    format += "Z";

    strftime(buf, sizeof(buf), format.c_str(), &ts);
    return buf;
}