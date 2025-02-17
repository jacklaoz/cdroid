#include <app.h>
#include <cdlog.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <cdgraph.h>
#include <windowmanager.h>
#include <assets.h>
#include <cairomm/surface.h>
#include <inputmethodmanager.h>
#include <getopt.h>
#include <fcntl.h>
#include <string.h>
#include <thread>
#include <cdinput.h>
#include <inputeventsource.h>
#include <mutex>
#include <cla.h>
#include <core/build.h>
#include <core/atexit.h>

#if defined(__linux__)||defined(__unix__)
extern "C" char *__progname;
#elif defined(_WIN32)||defined(_WIN64)
extern "C" unsigned long  GetModuleFileNameA(void* hModule, char* lpFilename, unsigned long nSize);
#endif

namespace cdroid{

App*App::mInst=nullptr;

static CLA::Argument ARGS[]={
   {CLA::EntryType::Option, "a", "alpha",  "UI layer global alpha[0,255]", CLA::ValueType::Int, (int)CLA::EntryFlags::Manditory},
   {CLA::EntryType::Option, "" , "density","UI Density"  ,    CLA::ValueType::Int , (int)CLA::EntryFlags::Optional },
   {CLA::EntryType::Option, "", "data",   "app data path",        CLA::ValueType::String, (int)CLA::EntryFlags::Optional },
   {CLA::EntryType::Option, "m", "monkey", "events playback path",  CLA::ValueType::String, (int)CLA::EntryFlags::Optional },
   {CLA::EntryType::Option, "r", "record", "events record path", CLA::ValueType::String,   (int)CLA::EntryFlags::Optional},
   {CLA::EntryType::Option, "R", "rotate", "display rotate ", CLA::ValueType::Int,   (int)CLA::EntryFlags::Optional},
   {CLA::EntryType::Option, "f", "framedelay","animation frame delay",CLA::ValueType::Int,   (int)CLA::EntryFlags::Optional},
   {CLA::EntryType::Switch, "h", "help", "display help info ", CLA::ValueType::None,   (int)CLA::EntryFlags::Optional},
   {CLA::EntryType::Switch, "d", "debug", "open debug", CLA::ValueType::None,   (int)CLA::EntryFlags::Optional},
   {CLA::EntryType::Option, "l", "logo" , "show logo" , CLA::ValueType::String, (int)CLA::EntryFlags::Optional},
   {CLA::EntryType::Switch, "", "fps"  , "Show FPS ",CLA::ValueType::None,   (int)CLA::EntryFlags::Optional}
};

App::App(int argc,const char*argv[],const std::vector<CLA::Argument>&extoptions){
    int rotation;
    LogParseModules(argc,argv);
    mQuitFlag = false;
    mExitCode = 0;
    mInst = this;
    cla.addArguments(ARGS,sizeof(ARGS)/sizeof(CLA::Argument));
    cla.addArguments(extoptions.data(),extoptions.size());
    cla.setSwitchChars("-");
    cla.parse(argc,argv);
    rotation = (getArgAsInt("rotate",0)/90)%4;
    if(hasSwitch("help")){
        std::cout<<cla.getUsageString()<<std::endl;
        std::cout<<"params.count="<<getParamCount()<<std::endl;
        exit(EXIT_SUCCESS);
        LogSetModuleLevel(nullptr,LOG_FATAL);
        mQuitFlag=1;
    }
    Typeface::setContext(this);
    onInit();
#if defined(__linux__)||defined(__unix__)
    setName(std::string(argc?argv[0]:__progname));
#elif (defined(_WIN32)||defined(_WIN64))
    char progName[260];
    GetModuleFileNameA(nullptr,progName,sizeof(progName));
    setName(std::string(argc?argv[0]:progName));
#endif
    LOGI("\033[1;35m          ┏━┓┏┓╋╋╋┏┓┏┓");
    LOGI("\033[1;35m          ┃┏╋┛┣┳┳━╋╋┛┃");
    LOGI("\033[1;35m          ┃┗┫╋┃┏┫╋┃┃╋┃");
    LOGI("\033[1;35m          ┗━┻━┻┛┗━┻┻━┛");

    LOGI("cdroid %s Build:%d Commit:%s",BUILD::VERSION::Release.c_str(),BUILD::VERSION::BuildNumber,BUILD::VERSION::CommitID.c_str());
    LOGI("https://www.gitee.com/houstudio/cdroid\n");
    LOGI("App [%s] started c++=%d",mName.c_str(),__cplusplus);
	
    View::VIEW_DEBUG = hasSwitch("debug");
    Looper::prepareMainLooper();
    Choreographer::setFrameDelay(getArgAsInt("framedelay",Choreographer::DEFAULT_FRAME_DELAY));
    WindowManager::getInstance().setDisplayRotation(rotation);
    GraphDevice::getInstance().setRotation(rotation).setLogo(getArg("logo")).showFPS(hasSwitch("fps")).init();
    Typeface::loadPreinstalledSystemFontMap();
    setOpacity(getArgAsInt("alpha",255));
    Typeface::loadFaceFromResource(this);
    DisplayMetrics::DENSITY_DEVICE = getArgAsInt("density",DisplayMetrics::getDeviceDensity());
    AtExit::registerCallback([this](){
        LOGD("Exit...");
        mQuitFlag = true;
    });
    InputEventSource*inputsource=&InputEventSource::getInstance();//(getArg("record",""));
    addEventHandler(inputsource);
    inputsource->playback(getArg("monkey",""));
}

App::~App(){
    WindowManager::getInstance().shutDown();
    InputMethodManager::getInstance().shutDown();
    delete Looper::getMainLooper();
    delete &GraphDevice::getInstance();
}

void App::onInit(){
    LOGD("onInit");
    GFXInit();
    mDisplayMetrics.setToDefaults();
    addResource(getDataPath()+std::string("cdroid.pak"),"cdroid");
}

const std::string App::getDataPath()const{
    std::string path=getArg("data","./");
    if(path.back()!='/')path+='/';
    return path;
}

App& App::getInstance(){
    if(mInst==nullptr)
        mInst = new App;
    return *mInst;
}

const std::string App::getArg(const std::string&key,const std::string&def)const{
    std::string value = def;
    cla.find(key,value);
    return value;
}

bool App::hasArg(const std::string&key)const{
    return cla.find(key);
}

bool App::hasSwitch(const std::string&key)const{
    return cla.findSwitch(key);
}

void App::setArg(const std::string&key,const std::string&value){
    cla.setArgument(key,value);
}

int App::getArgAsInt(const std::string&key,int def)const{
    int value = def;
    cla.find(key,value);
    return value;
}

float App::getArgAsFloat(const std::string&key,float def)const{
    float value = def;
    cla.find(key,value);
    return value;
}

double App::getArgAsDouble(const std::string&key,double def)const{
    double value = def;
    cla.find(key,value);
    return value;
}

size_t App::getParamCount()const{
    return cla.getParamCount();
}

std::string App::getParam(int idx,const std::string&def)const{
    std::string value = def;
    cla.getParam(idx,value);
    return value;
}

void App::setOpacity(unsigned char alpha){
    auto primarySurface = GraphDevice::getInstance().getPrimarySurface();
    if(primarySurface){
        GFXSurfaceSetOpacity(primarySurface,alpha);
        LOGD("alpha=%d",alpha);
    }
}

void App::addEventHandler(const EventHandler*handler){
    Looper::getMainLooper()->addEventHandler(handler);
}

void App::removeEventHandler(const EventHandler*handler){
    Looper::getMainLooper()->removeEventHandler(handler);
}

int App::exec(){
    Looper*looper = Looper::getMainLooper();
    while(!mQuitFlag)looper->pollAll(1);
    return mExitCode;
}

void App::exit(int code){
    mQuitFlag = true;
    mExitCode = code;
}

void App::setName(const std::string&appname){
    mName = appname;
    size_t pos = mName.find_last_of("/");
    if(pos!=std::string::npos)
        mName = mName.substr(pos+1);
    addResource(getDataPath()+mName+std::string(".pak"));
}

const std::string& App::getName(){
    return mName;
}

}

