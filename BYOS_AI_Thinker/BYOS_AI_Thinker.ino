/************************************************************
 * **********************************************************
 * ************* BYOS - Be Your Own Saviour *****************
 * **********************************************************
 * **********************************************************
 */

 /**********************************************************
  * 
  * Header Files
  * 
  * ********************************************************
  */
  /**
   * Serial 
   */
   #include <HardwareSerial.h>
   
  /**
   * Wifi 
   */
   #include <WiFi.h>
   #include <WebServer.h>
   #include <WiFiClient.h>

  /**
   * Camera
   */
  #define CAMERA_MODEL_AI_THINKER
  #include "src/OV2640.h"
  #include "camera_pins.h"

  /**
   * GPS
   */
   #include <TinyGPS++.h>
   
   /**
    * GSM
    */
    #define TINY_GSM_MODEM_SIM800   // Define SIM800 before adding TinyGPS++.h
    #include <TinyGsmClient.h> // https://github.com/vshymanskyy/TinyGSM


  /**
   * Blynk
   */
//   #include <BlynkSimpleEsp32.h>
//   #include <BlynkSimpleSIM800.h> //https://github.com/blynkkk/blynk-library
   #include <BlynkSimpleTinyGSM.h>



  /**
   * Button
   */
   #include <AceButton.h> // https://github.com/bxparks/AceButton
   using namespace ace_button;

   



 /**********************************************************
  * 
  * Macros
  * 
  * ********************************************************
  */

  /**
   * Macros for Serial Debug
   */
   #define SerialDebug  Serial
   #define SerialGSM    Serial2
   #define SerialGPS    Serial
   #define SERIAL_GSM_RX  14
   #define SERIAL_GSM_TX  15
   #define SERIAL0_RX  3
   #define SERIAL0_TX  1

  /**
   * Macros for WiFi operation
   */
   #define SSID1        "rehanHOTSPOT"
   #define PWD1         "123456789"

  /**
   * Macros for Camera operation
   */
   // Select camera model
   //#define CAMERA_MODEL_WROVER_KIT
   //#define CAMERA_MODEL_ESP_EYE
   //#define CAMERA_MODEL_OV7670
   //#define CAMERA_MODEL_M5STACK_PSRAM
   //#define CAMERA_MODEL_M5STACK_WIDE

  /**
   * Macros for GPS operation
   */

  /**
   * Macros for GSM operation
   */

  /**
   * Macros for Blynk operation
   */

  /**
   * Macros for Button operation
   */
   #define EMERGENCY_BUTTON 13

  /**
   * Macros for Buzzer
   */
   #define BUZZER_PIN 12

  /**
   * Macros for Shocker circuit
   */
   #define SHOCKER_EN_PIN 4

   
 /******************************************************
  * 
  * Function Declaration
  * 
  */
   
  /**
   * Funtion to handle button interrupt
   */
   void handleEvent_sms(AceButton*, uint8_t, uint8_t);


 /**********************************************************
  * 
  * Global Variables
  * 
  * ********************************************************
  */

  /**
   * Global variables used for Serial Debug
   */
   
  /**
   * Global variables used for WiFi operation
   */

  /**
   * Global variables used for webserver
   */
   WebServer server(80);
   const char HEADER[] = "HTTP/1.1 200 OK\r\n" \
                      "Access-Control-Allow-Origin: *\r\n" \
                      "Content-Type: multipart/x-mixed-replace; boundary=123456789000000000000987654321\r\n";
   const char BOUNDARY[] = "\r\n--123456789000000000000987654321\r\n";
   const char CTNTTYPE[] = "Content-Type: image/jpeg\r\nContent-Length: ";
   const int hdrLen = strlen(HEADER);
   const int bdrLen = strlen(BOUNDARY);
   const int cntLen = strlen(CTNTTYPE);
   const char JHEADER[] = "HTTP/1.1 200 OK\r\n" \
                       "Content-disposition: inline; filename=capture.jpg\r\n" \
                       "Content-type: image/jpeg\r\n\r\n";
   const int jhdLen = strlen(JHEADER);
   bool StreamFlag = 0;

   

  /**
   * Global variables used  for Camera operation
   */
   OV2640 cam;

  /**
   * Global variables used for GPS operation
   */
   TinyGPSPlus gps;
   float latitude , longitude;
   String  lat_str , lng_str;
//   HardwareSerial MyGPS(2);


  /**
   * Global variables used for GSM operation
   */
//   const char apn[]  = "internet.ng.airtel.com"; // Change for different SIM
   const char apn[]  = "airtelgprs.com"; // Change for different SIM

   const char user[] = "";   // Set NULL if not known or not set
   const char pass[] = "";   // Set NULL if not known or not set*/
   TinyGsm modem(Serial2);
   String mobile_number = "ADD_MOB_NUM";
   String message = "It's an Emergency. I'm at this location ";
   String message_with_data;

  /**
   * Global variables used for Blynk operation
   */
    char auth[] = "auth Key";
//   BlynkTimer timer;   // blynk timer 
   WidgetMap myMap(V0); 

  /**
   * Global variables used for Button operation
   */
   ButtonConfig config1;
   AceButton sms_button(&config1);
   static bool buttonPressed = 0;
   
  /**
   * Global variables used for Buzzer
   */

  /**
   * Global variables used for Shocker circuit
   */

 /**********************************************************
  * 
  * Helper Functions
  * 
  * ********************************************************
  */
  /**
   * Function to handle jpg image stream
   */
  void handle_jpg_stream(void)
  {
    char buf[32];
    int s;

    WiFiClient client = server.client();
    client.write(HEADER, hdrLen);
    client.write(BOUNDARY, bdrLen);
    while (true)
    {
      if (!client.connected()) break;
      cam.run();
      s = cam.getSize();
      client.write(CTNTTYPE, cntLen);
      sprintf( buf, "%d\r\n\r\n", s );
      client.write(buf, strlen(buf));
      client.write((char *)cam.getfb(), s);
      client.write(BOUNDARY, bdrLen);
    }
  }

  /**
   * Function to handle jpg
   */
   void handle_jpg(void)
   {
      WiFiClient client = server.client();

      cam.run();
      if (!client.connected()) 
          return;
      
      client.write(JHEADER, jhdLen);
      client.write((char *)cam.getfb(), cam.getSize());
    }

   /**
    * Function to handle Not Found condition
    */
    void handleNotFound()
    {
      String message = "Server is running!\n\n";
      message += "URI: ";
      message += server.uri();
      message += "\nMethod: ";
      message += (server.method() == HTTP_GET) ? "GET" : "POST";
      message += "\nArguments: ";
      message += server.args();
      message += "\n";
      server.send(200, "text / plain", message);
    }

    /**
     * Function to handle button event
     */
     void handleEvent_sms(AceButton* /* button */, uint8_t eventType,
                     uint8_t /* buttonState */) 
     {
        switch (eventType)
        {
          case AceButton::kEventPressed:
               SerialDebug.println("kEventPressed");
               StreamFlag = 1;
               digitalWrite(BUZZER_PIN, HIGH);
               digitalWrite(SHOCKER_EN_PIN, HIGH);
               //  delay(1000);
              message_with_data = message + "Latitude = " + (String)latitude + "Longitude = " + (String)longitude;
              modem.sendSMS(mobile_number, message_with_data);
              message_with_data = "";
              delay(1000);
              modem.callNumber(mobile_number);
            break;
          case AceButton::kEventReleased:
                SerialDebug.println("kEventReleased");
              break;
  }
}


 /**********************************************************
  * 
  * Setup Function
  * 
  * ********************************************************
  */
  void setup()
  {
    /* Same port will act as debug and GPS com port*/
    Serial.begin(9600, SERIAL_8N1, SERIAL0_RX, SERIAL0_TX);

    SerialGSM.begin(115200,SERIAL_8N1,SERIAL_GSM_RX,SERIAL_GSM_TX);


//    Serial.begin(115200);
    //while (!Serial);            //wait for serial connection.
    /* Configuring GPIOS */
    pinMode(BUZZER_PIN ,OUTPUT);
    pinMode(SHOCKER_EN_PIN,OUTPUT);
//    digitalWrite(12,HIGH);
    /* Configuring camera */
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    // Frame parameters
  //  config.frame_size = FRAMESIZE_UXGA;
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 2;

#if defined(CAMERA_MODEL_ESP_EYE)
    pinMode(13, INPUT_PULLUP);
    pinMode(14, INPUT_PULLUP);
#endif

    cam.init(config);

    SerialDebug.println("Camera Coniguration Done!");

    /* Wifi Configuration */
    IPAddress ip;

    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID1, PWD1);
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      SerialDebug.print(F("."));
    }
    ip = WiFi.localIP();
    SerialDebug.println(F("WiFi connected"));
    SerialDebug.println("");
    SerialDebug.println(ip);
    SerialDebug.print("Stream Link: http://");
    SerialDebug.print(ip);
    SerialDebug.println("/mjpeg/1");
    server.on("/mjpeg/1", HTTP_GET, handle_jpg_stream);
    server.on("/jpg", HTTP_GET, handle_jpg);
    server.onNotFound(handleNotFound);
    server.begin();

  /**************** GSM *******************/
    // Set GSM module baud rate and UART pins
    SerialDebug.println("Setting up GSM Module!");
//    delay(3000); // Time for initializing GSM modem

    // Restart takes quite some time
  // To skip it, call init() instead of restart()
    SerialDebug.println("Initializing modem...");
    modem.restart();
    String modemInfo = modem.getModemInfo();
    SerialDebug.print("Modem: ");
    SerialDebug.println(modemInfo);
   // Unlock your SIM card with a PIN
//    modem.simUnlock("1234");
    SerialDebug.print("Waiting for network...");
    if (!modem.waitForNetwork(240000L))
    {
      SerialDebug.println(" fail");
      delay(10000);
      return;
    }
    if (modem.isNetworkConnected())
    {
      SerialDebug.println("Network connected");
    }
    SerialDebug.print(F("Connecting to APN: "));
    SerialDebug.println(apn);
    if (!modem.gprsConnect(apn, user, pass))
    {
      SerialDebug.println(" fail");
      delay(10000);
      return;
    }
//  
//    SerialDebug.println(" OK");
        /* Blynk configuration */

//    Blynk.config(auth);  // in place of Blynk.begin(auth, ssid, pass);
//    Blynk.connect(3333); 
//      while (Blynk.connect() == false) {
//    // Wait until connected
//    SerialDebug.println("Blynk !");
//  }
  
//    Blynk.begin(auth, SSID1, PWD1);
//    Blynk.virtualWrite(V0, "clr");


    SerialDebug.println("Setting up Blynk interface!");       
    Blynk.begin(auth, modem, apn, user, pass);
    SerialDebug.println("\t\tConfiguring Interrupt\n\n");
    pinMode(EMERGENCY_BUTTON, INPUT);
    config1.setEventHandler(handleEvent_sms);
    sms_button.init(EMERGENCY_BUTTON);
                   digitalWrite(BUZZER_PIN, HIGH);

  
  }

 /**********************************************************
  * 
  * Loop Function
  * 
  * ********************************************************
  */
  int pin = 0; // remove this
  void loop()
{

  {// Dummy remove this
  Blynk.virtualWrite(V0, pin);
  pin++;
  SerialDebug.println(pin);

// digitalWrite(BUZZER_PIN, pin = !pin);
 delay(200);
  }
  while(StreamFlag)
  {
    server.handleClient();
  }
  
  sms_button.check();
  char ch = 0; 
  while (Serial.available() > 0) {
    ch = Serial.read();
   // Serial.write(ch);
    if (gps.encode(ch))
    {
      if (gps.location.isValid())
      {
        latitude = gps.location.lat();
        lat_str = String(latitude , 6);
        longitude = gps.location.lng();
        lng_str = String(longitude , 6);
        SerialDebug.print("Latitude = ");
        SerialDebug.println(lat_str);
        SerialDebug.print("Longitude = ");
        SerialDebug.println(lng_str);
//        Blynk.virtualWrite(V0, 1, latitude, longitude, "Location");
//        Blynk.virtualWrite(V1, lat_str);
//        Blynk.virtualWrite(V2, lng_str);
        break;
      }
     SerialDebug.println();  
    }
  }  
  Blynk.run();

}
