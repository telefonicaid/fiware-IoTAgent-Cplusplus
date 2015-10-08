/**
* Copyright 2015 Telefonica Investigación y Desarrollo, S.A.U
*
* This file is part of iotagent project.
*
* iotagent is free software: you can redistribute it and/or modify
* it under the terms of the GNU Affero General Public License as published
* by the Free Software Foundation, either version 3 of the License,
* or (at your option) any later version.
*
* iotagent is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU Affero General Public License for more details.
*
* You should have received a copy of the GNU Affero General Public License
* along with iotagent. If not, see http://www.gnu.org/licenses/.
*
* For those usages not covered by the GNU Affero General Public License
* please contact with iot_support at tid dot es
*/
#ifndef __TDA_H__
#define __TDA_H__

#include <time.h>

#include <sys/time.h>

#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <tinyxml.h>
#include "pthread.h"
//#include "sched.h"
#include "ESP_Attribute.h"
// ---------------------------------
// XML Definition
// ---------------------------------
#define XML_SECTION_ROOT "espsensor"
#define XML_SECTION_PLUGINS "plugins"
#define XML_SECTION_PREPROCESSORS "preprocessors"
#define XML_SECTION_POSTPROCESSORS "postprocessors"
#define XML_SECTION_INPUTS "inputs"
#define XML_SECTION_OUTPUTS "outputs"
#define XML_SECTION_COMMANDS "commands"

#define XML_TAG_HEADER "header"
#define XML_TAG_BODY "body"
#define XML_TAG_FOOTER "footer"

// ---------------------------------
// Types Definition
// ---------------------------------
#define uint64 unsigned long long
#define ccInt64 long long

#define mapContains(map, key) (map.find(key) != map.end())
#define mapInsert(map, keyType, valueType, key, value) \
  map.insert(std::pair<keyType, valueType>(key, value))
#define getParam(map, key) \
  map.find(key) != map.end() ? map[key] : std::string("")
typedef std::map<std::string, std::string> CC_ParamsType;
typedef std::pair<std::string, std::string> CC_ParamsPair;

#ifdef WIN32
#define SLEEP(X) _sleep(X);
#else
#include <unistd.h>
#define SLEEP(X) usleep(X * 1000);
#endif  // WIN32

#define INT32_MAX_RANGE 2147483647
#define INT64_MAX_RANGE 9223372036854775807ll

#define RUNNER_MAX_BUFFER 1024 * 100  // 100KB

/* --------------- */
/* CONTEXT         */
/* --------------- */
class ESP_Context {
 private:
  char* input;
  int input_size;
  int index_start;
  int index_end;

  char* output;
  int output_size;

  int index_end_saved;  // To save index
 public:
  ESP_Context();
  ~ESP_Context();
  bool close;                              // Set to close Sensor
  CC_AttributesType temp;                  // Temp Measures
  std::vector<CC_AttributesType> results;  // Final Measures

  // Methods
  void clear();
  void addParams(CC_ParamsType params);
  int getInputSize();
  int getOutputSize();
  char* getInputData();
  char* getOutputData();
  void addInputData(const char* data, int len);
  int getAvailableInputData();
  int getAvailableOutputData();
  bool inputDataIsAvailable(int bytes);
  bool readPInputData(int bytes, const char*& result);
  bool validateInputData(
      int offset = 0);   // Discard index_end - index_data bytes
  bool addResultData();  // Copy Temp to new Result (will not clear temp)
  int searchAttributeValue(ESP_Attribute* attribute);  // Search if the value is
                                                       // present on the buffer,
                                                       // returning the index
                                                       // where it starts
  int searchAttributeValueInRange(ESP_Attribute* attribute, int start,
                                  int size);
  int getSizeFromIndex(int index);  // Returns the size from index_end to index

  void saveIndex();
  void restoreIndex();

  // Query
  // ESP_Attribute *getAttributeRefByNameFromResults(std::string name); // get
  // Attribute Ref from results

  // Output
  void addOutputData(const char* data, int len);
  void removeOutputData(int nbytes);
};

// ---------------------------------
// TAGS
// ---------------------------------
class ESP_Tag_Base {
 public:
  int index;  // subparser index
  ESP_Tag_Base* parent;
  std::vector<ESP_Tag_Base*> subtags;
  bool _parsed;
  bool _repeat;  // Set to true to run again on parsing

  std::string _name;
  std::string _type;
  int _byteoffset;       // Offset in bytes
  int _datasize;         // Size in Bytes
  int _datatype;         // Type of stored data
  int _datacode;         // Type of encoding for input data
  ESP_Attribute _value;  // Base Value Common for most of the tags
  bool _loop;            // To loop tag automatically forever
  std::string _valueref;

  // Base Methods
  ESP_Tag_Base();
  virtual void reset();  // Overwrittable
  int run(ESP_Context* context);
  ESP_Tag_Base* getNextTag();
  void setChildrenParsed(bool parent, bool status);
  virtual ~ESP_Tag_Base();

  // Virtual or Overridable Methods
  void parseElement(TiXmlElement* element);
  virtual void parseCustomElement(TiXmlElement* element);
  virtual int execute(ESP_Context* context) = 0;  // -1 error parsing, 0 not
                                                  // enough nada, 1 ok parsing

  // Utils
  static ESP_Tag_Base* searchParent(ESP_Tag_Base* node, std::string type);

  // Enums
  enum ExecuteResultType {
    EXECUTE_RESULT_ERROR,
    EXECUTE_RESULT_IDLE,
    EXECUTE_RESULT_OK,
    EXECUTE_RESULT_OK_RETURN  // Return Valid and Continue Later
  };
};

class ESP_Tag_Header : public ESP_Tag_Base {
 public:
  ~ESP_Tag_Header() {}
  void parseCustomElement(TiXmlElement* element);
  int execute(ESP_Context* context);
};

class ESP_Tag_Body : public ESP_Tag_Base {
 public:
  ~ESP_Tag_Body() {}
  void parseCustomElement(TiXmlElement* element);
  int execute(ESP_Context* context);
};

class ESP_Tag_Footer : public ESP_Tag_Base {
 public:
  ~ESP_Tag_Footer() {}
  void parseCustomElement(TiXmlElement* element);
  int execute(ESP_Context* context);
};

/* --------------------------------------- */
/* INPUT,OUTPUT,PREPROCESSOR,POSTPROCESSOR */
/* --------------------------------------- */
class ESP_Base {
 public:
  virtual bool checkParams();
};

class ESP_Preprocessor_Base : public ESP_Base {
 private:
  CC_ParamsType params;

 public:
  std::string _type;
  std::string _name;
  std::string _script;

  virtual bool initialize() = 0;
  virtual bool execute(ESP_Context* context) = 0;
  virtual bool terminate() = 0;
  virtual const char* getResultData() = 0;
  virtual int getResultSize() = 0;

  void parseElement(TiXmlElement* element);
  virtual void parseCustomElement(
      TiXmlElement* element) = 0;  // To be implemented on subclasses
};

class ESP_Postprocessor_Base : public ESP_Base {
 private:
  CC_ParamsType params;

 public:
  std::string _name;
  std::string _type;

  virtual bool initialize() = 0;
  virtual bool execute(CC_AttributesType* attributes) = 0;
  virtual bool terminate() = 0;
  virtual const char* getResultData() = 0;
  virtual int getResultSize() = 0;
  virtual bool isResultValid() = 0;

  void parseElement(TiXmlElement* element);
  virtual void parseCustomElement(
      TiXmlElement* element) = 0;  // To be implemented on subclasses
  virtual std::string getType();
};

class ESP_Input_Base : public ESP_Base {
 public:
  int _sid;  // id of server if available
  int _mode;
  std::string _type;
  std::string _name;
  std::vector<std::string> _preprocessors;
  pthread_mutex_t mutex;

  ESP_Input_Base();
  virtual ~ESP_Input_Base();
  std::vector<ESP_Preprocessor_Base*> preprocessors;

  virtual int openServer() = 0;  // Inits id
  virtual int acceptServer() = 0;
  virtual bool stopServer() = 0;   // Stops accept
  virtual bool closeServer() = 0;  // Free Resources

  virtual int openClient() = 0;          // Inits id
  virtual bool stopClient(int id) = 0;   // Stops client
  virtual bool closeClient(int id) = 0;  // Free Resources
  virtual int readClient(int id, char* buffer, int len) = 0;
  virtual int writeClient(int id, char* buffer, int len) = 0;

  void parseElement(TiXmlElement* element);
  virtual void parseCustomElement(TiXmlElement* element) = 0;

  struct InputMode {
    enum InputModeEnum { INPUT_MODE_CLIENT, INPUT_MODE_SERVER };
  };
};

class ESP_Output_Base : public ESP_Base {
 public:
  std::string _type;
  std::string _name;
  std::vector<std::string> _postprocessors;

  std::vector<ESP_Postprocessor_Base*> postprocessors;
  ESP_Output_Base();
  virtual bool execute(CC_AttributesType* attributes,
                       ESP_Postprocessor_Base* postproccesor,
                       std::map<std::string, void*> userData) = 0;

  void parseElement(TiXmlElement* element);
  virtual void parseCustomElement(TiXmlElement* element) = 0;

  virtual ~ESP_Output_Base(){};
};

/* ----------------- */
/* PLUGINS           */
/* ----------------- */
class ESP_Plugin_Base {};

class ESP_Plugin_Preprocessor_Base : public ESP_Plugin_Base {
 public:
  virtual ESP_Preprocessor_Base* createPreprocessor(TiXmlElement* element) = 0;
  static ESP_Plugin_Base* getSingleton();
};

class ESP_Plugin_Postprocessor_Base : public ESP_Plugin_Base {
 public:
  virtual ESP_Postprocessor_Base* createPostprocessor(
      TiXmlElement* element) = 0;
  static ESP_Plugin_Base* getSingleton();
};

class ESP_Plugin_Input_Base : public ESP_Plugin_Base {
 public:
  virtual ESP_Input_Base* createInput(TiXmlElement* element) = 0;
  static ESP_Plugin_Base* getSingleton();
};

class ESP_Plugin_Output_Base : public ESP_Plugin_Base {
 public:
  virtual ESP_Output_Base* createOutput(TiXmlElement* element) = 0;
  static ESP_Plugin_Base* getSingleton();
};

class ESP_Plugin_Parser_Base : public ESP_Plugin_Base {
 public:
  virtual ESP_Tag_Base* createParser(TiXmlElement* element) = 0;
  static ESP_Plugin_Base* getSingleton();
};

/* ----------------- */
/* COMMANDS          */
/* ----------------- */
class ESP_Command {
 public:
  ESP_Command();
  ~ESP_Command();
  void clear();

  std::string name;
  bool roe;   // resetonempty
  int index;  // 0,1,2 (runner index on header,parser,footer)
  ESP_Tag_Base* header;
  ESP_Tag_Base* body;
  ESP_Tag_Base* footer;
  CC_ParamsType params;

  // Methods
  void reset();
  int run(ESP_Context* context);

  // Enums
  struct ESP_COMMAND_INDEX {
    enum ESP_COMMAND_INDEX_ENUM {
      HEADER = 0,
      BODY = 1,
      FOOTER = 2,
      FINISHED = 3
    };
  };
};

/* --------------- */
/* SENSOR          */
/* --------------- */
class ESP_Runner;
class ESP_Runner_Server;
class ESP_Runner_Client;
class ESP_Sensor {
 public:
  int _id;
  int _type;
  std::string descriptorFile;
  TiXmlDocument doc;
  bool readyToRun;

  pthread_mutex_t mutexMapServer;
  pthread_mutex_t mutexMapClient;

  std::vector<ESP_Plugin_Input_Base*> input_plugins;
  std::vector<ESP_Plugin_Output_Base*> output_plugins;
  std::vector<ESP_Plugin_Parser_Base*> parser_plugins;
  std::vector<ESP_Plugin_Preprocessor_Base*> preprocessor_plugins;
  std::vector<ESP_Plugin_Postprocessor_Base*> postprocessor_plugins;

  std::vector<ESP_Preprocessor_Base*> preprocessors;
  std::vector<ESP_Postprocessor_Base*> postprocessors;
  std::vector<ESP_Input_Base*> inputs;
  std::vector<ESP_Output_Base*> outputs;

  std::map<unsigned long long, ESP_Runner_Server*> serverRunners;
  std::map<unsigned long long, ESP_Runner_Client*> clientRunners;

  std::map<std::string, TiXmlElement*> commands;  // List of elements from Xml

  // UserData
  std::map<std::string, void*> userData;

  // Callback
  void* dataCallbackUserData;
  void* resultCallbackUserData;
  void (*dataCallback)(void* userData, const char* buffer, int nread);
  void (*resultCallback)(void* userData, ESP_Runner* runner);
  void registerDataCallback(void* userData,
                            void (*cb)(void* userData, const char* buffer,
                                       int nread));
  void registerResultCallback(void* userData,
                              void (*cb)(void* userData, ESP_Runner* runner));

  // Run Callbacks
  void runDataCallback(const char* buffer, int nread);
  void runResultCallback(ESP_Runner* runner);

  // Run Methods
  ESP_Sensor();
  ~ESP_Sensor();
  bool loadFromFile(std::string fileName);
  bool parseRootElement(TiXmlElement* rootElement);

  bool parsePlugins(TiXmlElement* element);

  bool parsePreprocessors(TiXmlElement* element);
  bool parsePostprocessors(TiXmlElement* element);
  bool parseInputs(TiXmlElement* element);
  bool parseOutputs(TiXmlElement* element);
  bool parseCommands(TiXmlElement* element);
  bool updateInputPreprocessors();
  bool updateOutputPostprocessors();

  ESP_Command* createCommand(std::string name);
  bool createCommandDetails(ESP_Tag_Base* command, TiXmlElement* element);

  void run(std::string name, CC_ParamsType params);
  void stop();

  // API Methods
  std::vector<std::string> getCommands();
  void setUserData(std::map<std::string, void*> userData);
  ESP_Input_Base* getInputFromName(std::string name);

  // Plugins
  void addInputPlugin(ESP_Plugin_Input_Base* plugin);
  void addOutputPlugin(ESP_Plugin_Output_Base* plugin);
  void addPreprocessorPlugin(ESP_Plugin_Preprocessor_Base* plugin);
  void addPostprocessorPlugin(ESP_Plugin_Postprocessor_Base* plugin);
  void addParserPlugin(ESP_Plugin_Parser_Base* plugin);
  ESP_Tag_Base* createParserFromPlugins(TiXmlElement* element);
  ESP_Input_Base* createInputFromPlugins(TiXmlElement* element);
  ESP_Output_Base* createOutputFromPlugins(TiXmlElement* element);
  ESP_Preprocessor_Base* createPreprocessorFromPlugins(TiXmlElement* element);
  ESP_Postprocessor_Base* createPostprocessorFromPlugins(TiXmlElement* element);

  // Callbacks
  // static void ResultCallback(ESP_Sensor *sensor, ESP_Context *context);
};

/* --------------- */
/* RESULTS         */
/* --------------- */
class ESP_Result {
 public:
  std::vector<CC_AttributesType> attresults;
  std::vector<ESP_Attribute> ppresults;

  std::string findInputAttributeAsString(std::string name);
  std::string findInputAttributeAsString(std::string name, bool remove);
  std::string getOutputResultAsString(int ind);

  int getOutputSize();
};

/* --------------- */
/* RUNNERS         */
/* --------------- */
class ESP_PostProc_Result {
 private:
  char* data;
  int nlen;

 public:
  ESP_PostProc_Result(char* data, int nlen);

  ~ESP_PostProc_Result() {
    free(data);
    data = NULL;
  };
};

/* --------------- */
/* RUNNERS         */
/* --------------- */
class ESP_Runner {
 protected:
  pthread_t thread;

  ESP_Runner(ESP_Sensor* sensor);

  void initConditions();

 public:
  ~ESP_Runner();

  pthread_mutex_t mutexCond;
  pthread_cond_t condFinished;

  bool bFinished;

  bool runOnce;  // 1-Loop iteration max
  int _id;       // Id of input
  ESP_Command* command;

  // std::map <std::string, ESP_Attribute> mapResult;

  bool _keepRunning;
  ESP_Context context;
  ESP_Input_Base* input;
  ESP_Sensor* sensor;
  virtual void run() = 0;
  void signalFinish();
  void stopRunner(unsigned int msecs);
};

class ESP_Runner_Server : public ESP_Runner {
 public:
  static void* runThread(void* userData);

 public:
  ESP_Runner_Server(ESP_Sensor* sensor);
  void run();
};

class ESP_Runner_Client : public ESP_Runner {
 public:
  static void* runThread(void* userData);
  static ESP_Result runLoop(void* userData);

 public:
  ESP_Runner_Client(ESP_Sensor* sensor);
  void run();
};

#endif
