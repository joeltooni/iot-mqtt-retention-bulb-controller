// This #include statement was automatically added by the Particle IDE.
#include <HttpClient.h>
#include <MQTT.h>

// File name: Photon2SmartBulbController
// View output from the command line with
// particle serial monitor

// Define the http variable to be of type HttpClient.
HttpClient http;

// MQTT callback function declaration
void mqttCallback(char *topic, byte *payload, unsigned int length);

// MQTT client setup - connect to your machine's IP
MQTT mqttClient("172.26.30.167", 1883, mqttCallback);

// We always pass Http headers on each request to the Http Server
// Here, we only define a single header. The NULL, NULL pair is used
// to terminate the list of headers.

// The Content-Type header is used to inform the server
// of the type of message that it will be receiving. Here,
// we tell the server to expect to receive data marked up in
// JSON.

http_header_t headers[] = {
    {"Content-Type", "application/json"},
    {NULL, NULL}};

// Here we define structures to hold the request and the response data.
// These are declared with types defined in the header file included above.

http_request_t request;
http_response_t response;

// A variable to hold the device unique ID
String deviceID = "";

// We want to read A0
int photoResistor = A0;
int analogValue;

// LED control - using built-in LED (D7)
int ledPin = D7;
bool ledState = false;

unsigned long loop_timer;

void setup()
{

    // Specify the speed with which we will talk to the serial interface.
    // The serial interface is associated with our shell or command line interface.

    Serial.begin(9600);

    // The IP address of the server running on our machine.
    // Do not use localhost. The microcontroller would attempt
    // to visit itself with localhost.
    request.ip = IPAddress(172, 26, 30, 167);

    // Specify the port that our server is listening on.
    request.port = 1880;

    // get the unique id of this device as 24 hex characters
    deviceID = System.deviceID().c_str();

    // display the id to the command line interface
    Serial.println(deviceID);

    // Setup LED pin as output
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);

    // Connect to MQTT broker
    mqttClient.connect("Photon2_" + deviceID);
    if (mqttClient.isConnected())
    {
        Serial.println("Connected to MQTT broker");
        // Subscribe to LED control topic
        mqttClient.subscribe("Photon2LEDControl");
        Serial.println("Subscribed to Photon2LEDControl");
    }
    else
    {
        Serial.println("Failed to connect to MQTT broker");
    }

    // initialize the loop_timer variable to the current time
    loop_timer = millis();

    Serial.println("Light monitor setup complete");
}

// MQTT callback function - called when a message arrives
void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived on topic: ");
    Serial.println(topic);

    // Convert payload to string
    char message[length + 1];
    memcpy(message, payload, length);
    message[length] = '\0';

    Serial.print("Payload: ");
    Serial.println(message);

    // Parse the JSON to get LED command
    if (strstr(message, "\"led\":\"ON\"") != NULL)
    {
        digitalWrite(ledPin, HIGH);
        ledState = true;
        Serial.println("LED turned ON");
    }
    else if (strstr(message, "\"led\":\"OFF\"") != NULL)
    {
        digitalWrite(ledPin, LOW);
        ledState = false;
        Serial.println("LED turned OFF");
    }
}

// Provided with a response, display it to the command line interface.
void printResponse(http_response_t &response)
{
    Serial.println("HTTP Response: ");
    Serial.println(response.status);
    Serial.println(response.body);
}

void doPostRequest()
{

    // buf will hold the JSON string
    char json[1000] = {0};

    // This class makes handling JSON data easy.
    // Associate it with the buf array of char.
    JSONBufferWriter writer(json, sizeof(json) - 1);

    Serial.println("About to post");
    // Provide the path to the service
    // The HTTP IN node in Node-RED needs
    // is configured with a URL of microcontrollerLightValue
    request.path = "/microcontrollerLightValue";

    // convert the integer analogValue to a string
    std::string analogValueString = std::to_string(analogValue);
    char const *analogValueStr = analogValueString.c_str();

    // build a JSON string
    writer.beginObject();
    writer.name("deviceID").value(deviceID);
    writer.name("lightReading").value(analogValueStr);
    writer.endObject();

    // for debugging, view the JSON on the CLI
    Serial.println("JSON String");
    Serial.println(json);
    // assign the JSON string to the HTTP request body
    request.body = json;
    // post the request
    http.post(request, response, headers);
    // show response
    printResponse(response);

    // clear the buffer holding the JSON request
    writer.buffer()[std::min(writer.bufferSize(), writer.dataSize())] = 0;
}

void loop()
{
    // Keep MQTT connection alive and process incoming messages
    if (mqttClient.isConnected())
    {
        mqttClient.loop();
    }
    else
    {
        Serial.println("MQTT disconnected, reconnecting...");
        mqttClient.connect("Photon2_" + deviceID);
        if (mqttClient.isConnected())
        {
            mqttClient.subscribe("Photon2LEDControl");
        }
    }

    // every 5 seconds send an http post request
    if (millis() - loop_timer >= 5000UL)
    {
        loop_timer = millis();
        // read the light value from the photodiode
        analogValue = analogRead(photoResistor);
        Serial.printlnf("AnalogValue == %u", analogValue);
        doPostRequest();
    }
}