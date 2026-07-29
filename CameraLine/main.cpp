#include "Parameter.h"
#include "SphinxLib.h"// SphinxLib (GigE Vision SDK)
#pragma comment(lib, "x64\\Release\\SphinxLib.lib")


//  ----------------------------------- Trigger Setting
bool g_bUseTrigger = true;
#define LINE_CAM_TRIGGER_SOURCE        "LineIn1"
// "RisingEdge"  or  "FallingEdge"
#define LINE_CAM_TRIGGER_ACTIVATION    "RisingEdge"


// ----------------------------------- Buffer Setting
std::vector<std::vector<BYTE>> g_imgBuf1;
size_t g_payloadBytes1 = 0;
int g_nBufferSize = 8;

// ---------------------------------- Image Size
int g_nWidth = 4096;
int g_nHeight = 1000;





DISCOVERY g_discovery;
static constexpr SPHINX_CAMNR CAM1_NR = 1;

bool EnableFirewallException(); // enable Firewall Exception, incase it got disabled earlier
bool ForceIPConfiguration();    // sets the IPAddr, subnet mask, gateway and mac addr
bool DiscoverAndConnect();      // GEVInit + GEVInitXml + OpenStream + AllocateBuffers  
bool ConnectCamera(SPHINX_CAMNR camNr, int discoveryIndex);
bool OpenStreamAndAllocate(SPHINX_CAMNR camNr,
    std::vector<std::vector<BYTE>>& bufs,
    size_t& payloadBytes);
BYTE WINAPI OnGEVError(BYTE camNr, char* errStr, BYTE /*detailedLog*/);


bool AcquireShot(SPHINX_CAMNR camNr);
bool ApplySettings(SPHINX_CAMNR camNr,
    int width, int height,
    int offsetX, int offsetY,
    double exposureUs, string strGain,
    double lineRateHz);
bool ConfigureTrigger(SPHINX_CAMNR camNr);
bool RunAcquisitionLoop(SPHINX_CAMNR camNr,
    std::vector<std::vector<BYTE>>& imgBufs,
    int linesPerImage, int imgWidth, int imgHeight,
    cv::Mat& outAssembled);


void ShowAllFeatures(SPHINX_CAMNR camNr);
void ShowEnumList(string strEnumName); // Display list of posible enumeration

void TestRun();

int main()
{
    WORD res;
    //EnableFirewallException();
    //ForceIPConfiguration();
    DiscoverAndConnect();

   
    ShowAllFeatures(CAM1_NR);
    ShowEnumList("PixelFormat");
    ShowEnumList("TestPattern");
    ShowEnumList("TriggerSelector");
    ShowEnumList("TriggerSource");
    ShowEnumList("AcquisitionMode");
    ShowEnumList("NED_AnalogGain");

    //Test pattern
    const char* fmtStr = "NED_GreyDiagonalRamp";

    res = GEVSetFeatureEnumeration(CAM1_NR, "TestPattern", fmtStr, (int)strlen(fmtStr));
    if (res != GEV_STATUS_SUCCESS) cout << "TestPatternError" << endl;

    AcquireShot(CAM1_NR);
    return 0;
}

bool EnableFirewallException()
{
    // Get current EXE path
    char appPath[_MAX_PATH] = { 0 };
    GetModuleFileNameA(NULL, appPath, _MAX_PATH);

    BYTE status = 0;
    char str[] = "CameraLine";
    int error = GEVEnableFirewallException(appPath, str, &status);

    if (error == GEV_STATUS_SUCCESS) {
        // Note: use & (bitwise AND), not && (logical AND) for bitmask checks
        if (status & FIREWALL_IS_DISABLED) std::cout << "[Firewall] Disabled" << std::endl;
        if (status & FIREWALL_IS_ENABLED)  std::cout << "[Firewall] Rule enabled" << std::endl;
        if (status & FIREWALL_IS_ADDED)    std::cout << "[Firewall] Rule added" << std::endl;
        return true;
    } else {
        if (error == GEV_STATUS_ACCESS_DENIED)
            std::cout << "[Firewall] Access denied — run as Administrator" << std::endl;
        else
            std::cout << "[Firewall] Failed to enable exception" << std::endl;
        return false;
    }
}
bool ForceIPConfiguration()
{
    std::cout << ("[LineCameraController] ForceIPConfiguration 1 ") << std::endl;
    BYTE mac[6] = { 0xD8, 0x16, 0x0A, 0x00, 0xC0, 0x2F };

    DWORD cameraIp = inet_addr("169.254.230.156");
    DWORD subnet = inet_addr("255.255.255.0");
    DWORD gateway = inet_addr("192.168.001.001");
    DWORD adapterIp = inet_addr("192.168.11.26"); // PC NIC IP connected to the camera

    WORD status = GEVForceIp(cameraIp, subnet, gateway, mac, adapterIp);

    if (status == GEV_STATUS_SUCCESS) {
        std::cout << ("[LineCameraController] Force IP success\n") << std::endl;
        return true;
    } else {
        std::cout << ("[LineCameraController] Force IP failed : %d\n", status) << std::endl;
        return false;
    }
}
bool DiscoverAndConnect()
{
    std::cout << "[LineCameraController] Starting discovery..." << std::endl;

    WORD res = GEVDiscovery(&g_discovery, NULL, 500, FALSE, 0);

    if (res != GEV_STATUS_SUCCESS) {
        std::cout << "[LineCameraController] GEVDiscovery failed: "
            << GEVGetErrorString(res) << std::endl;
        return false;
    }

    std::cout << "[LineCameraController] Found " << (int)g_discovery.Count
        << " GigE camera(s)" << std::endl;

    bool bConnect = ConnectCamera(CAM1_NR, 0);
    bool bOpenStream = OpenStreamAndAllocate(CAM1_NR, g_imgBuf1, g_payloadBytes1);

    return bConnect && bOpenStream;
}
bool ConnectCamera(SPHINX_CAMNR camNr, int discoveryIndex)
{
    DEVICE_PARAM& dp = g_discovery.param[discoveryIndex];

    std::cout << "[LineCameraController] Connecting Cam" << (int)camNr
        << " → " << dp.model << " at index " << discoveryIndex << std::endl;

    CONNECTION con = { 0 };
    con.AdapterIP = dp.AdapterIP;
    con.AdapterMask = dp.AdapterMask;
    con.IP_CANCam = dp.IP;
    con.PortCtrl = 0; // 0 = automatic port
    con.PortData = 0;
    con.PortMessage = 0;
    strncpy_s(con.adapter_name, sizeof(con.adapter_name),
        dp.adapter_name, sizeof(con.adapter_name) - 1);

    WORD res = GEVInit(camNr, &con, OnGEVError, 0, EXCLUSIVE_ACCESS);
    if (res != GEV_STATUS_SUCCESS) {
        std::cout << "[LineCameraController] GEVInit Cam" << (int)camNr
            << " failed: " << GEVGetErrorString(res) << std::endl;
        return false;
    }

    // Load GenICam XML from device  --- [Shweta: commented this as it caused exception] : Fixed. There were insufficient files in the bin/release (*.xsd, *.dll, *.xsl)
    res = GEVInitXml(camNr);
    if (res != GEV_STATUS_SUCCESS) {
        std::cout << "[LineCameraController] GEVInitXml Cam" << (int)camNr
            << " failed: " << GEVGetErrorString(res) << std::endl;
        return false;
    }

#ifdef _DEBUG
    // Extend heartbeat in debug so single-step doesn't disconnect the camera
    GEVSetHeartbeatRate(camNr, linecam::network::DEBUG_HEARTBEAT_MS);
#endif

    std::cout << "[LineCameraController] Cam" << (int)camNr
        << " connected OK, Serial(" << dp.serial << ") Model:(" << dp.model << ")" << std::endl;

    std::string model(reinterpret_cast<char*>(dp.model), strnlen(reinterpret_cast<char*>(dp.model), sizeof(dp.model)));

    std::string serial(reinterpret_cast<char*>(dp.serial), strnlen(reinterpret_cast<char*>(dp.serial), sizeof(dp.serial)));

    cout << "Camera Model :  " << model << endl;
    cout << "Camera Serial :  " << serial << endl;

    return true;
}
bool OpenStreamAndAllocate(SPHINX_CAMNR camNr,
    std::vector<std::vector<BYTE>>& bufs,
    size_t& payloadBytes)
{
    // discoveryIndex = camNr - 1 (CAM1_NR=1 → index 0)
    DWORD adapterIp = g_discovery.param[camNr - 1].AdapterIP;

    WORD res = GEVOpenStreamChannel(camNr, adapterIp, 0, 0);
    if (res != GEV_STATUS_SUCCESS) {
        std::cout << "[LineCameraController] GEVOpenStreamChannel Cam" << (int)camNr
            << " failed: " << GEVGetErrorString(res) << std::endl;
        return false;
    }

    INT64 payload = 0;
    res = GEVGetFeatureInteger(camNr, "PayloadSize", &payload);
    if (res != GEV_STATUS_SUCCESS) {
        std::cout << "[LineCameraController] PayloadSize read failed Cam"
            << (int)camNr << ": " << GEVGetErrorString(res) << std::endl;
        return false;
    }
    payloadBytes = (size_t)payload;
    std::cout << "[LineCameraController] Cam" << (int)camNr
        << " PayloadSize=" << payloadBytes << " bytes" << std::endl;

    bufs.resize(g_nBufferSize);
    for (auto& b : bufs)
        b.resize(payloadBytes, 0);

    std::cout << "[LineCameraController] Cam" << (int)camNr << " stream open + "
        << bufs.size() << " buffers allocated" << std::endl;
    return true;
}
BYTE WINAPI OnGEVError(BYTE camNr, char* errStr, BYTE /*detailedLog*/)
{
    std::cout << "[SphinxLib][Cam" << (int)camNr << "] " << errStr << std::endl;
    return 0;
}
bool AcquireShot(SPHINX_CAMNR camNr)
{
    cv::Mat assembled; //The actual shotImage 

    // ── Step 1: Get parameters ────────────────────────────────
    int    nTotalHeight = g_nHeight*2, imgWidth = g_nWidth, imgHeight = g_nHeight, offsetX = 0, offsetY = 0;
    double exposureUs = 397.7, lineRateHz = 2500;
    string strGain = "x100";

    // ── Step 2: Apply camera settings ────────────────────────
    if (!ApplySettings(camNr, imgWidth, imgHeight, offsetX, offsetY,
        exposureUs, strGain, lineRateHz)) {
        std::cout << "[AcquireShot] ApplySettings failed" << std::endl;
        return false;
    }

    // ── Step 3: Configure trigger ─────────────────────────────
    if (!ConfigureTrigger(camNr)) {
        std::cout << "[AcquireShot] ConfigureTrigger failed" << std::endl;
        return false;
    }

    // ── Step 4: Arm acquisition ───────────────────────────────
    WORD res = GEVAcquisitionStart(camNr, 0); // 0 = continuous mode
    if (res != GEV_STATUS_SUCCESS) {
        std::cout << "[AcquireShot] GEVAcquisitionStart Cam failed: " << GEVGetErrorString(res) << std::endl;
        return false;
    }
    std::cout << "[AcquireShot] Cam armed — waiting for trigger" << std::endl;

    // ── Steps 5+6: Collect lines → assemble image ─────────────
    auto& bufs = g_imgBuf1;
    size_t payloadBytes = g_payloadBytes1;

    TestRun();
    bool ok = true;
    //bool ok = RunAcquisitionLoop(camNr,
    //    bufs, payloadBytes,
    //    nTotalHeight, imgWidth, imgHeight,
    //    assembled);

    // ── Step 6: Disarm ────────────────────────────────────────
    GEVAcquisitionStop(camNr);

    if (!ok || assembled.empty()) {
        std::cout << "[AcquireShot] Assembly failed for Cam" << std::endl;
        return false;
    }
    cv::imwrite(R"(C:\Users\000547\Documents\ラインカメラ\hehe.png)", assembled);
    return true;
}
bool ApplySettings(SPHINX_CAMNR camNr,
    int width, int height,
    int offsetX, int offsetY,
    double exposureUs, string strGain,
    double lineRateHz)
{
    std::cout << "[LineCameraController] Applying settings to Cam" << (int)camNr << std::endl;

    WORD res;
    bool ok = true;

    // ── Image geometry ────────────────────────────────────────
    res = GEVSetFeatureInteger(camNr, "Width", width);
    if (res != GEV_STATUS_SUCCESS) {
        std::cout << "  Width failed: " << GEVGetErrorString(res) << std::endl; ok = false;
    }

    // For line-scan cameras Height is typically locked to 1; skip if it fails
    res = GEVSetFeatureInteger(camNr, "Height", height);
    if (res != GEV_STATUS_SUCCESS) {
        std::cout << "  Height failed: " << GEVGetErrorString(res) << std::endl; ok = false;
    }

    res = GEVSetFeatureInteger(camNr, "OffsetX", offsetX);
    if (res != GEV_STATUS_SUCCESS) {
        std::cout << "  OffsetX failed: " << GEVGetErrorString(res) << std::endl; ok = false;
    }

    res = GEVSetFeatureInteger(camNr, "OffsetY", offsetY);
    if (res != GEV_STATUS_SUCCESS) {
        std::cout << "  OffsetX failed: " << GEVGetErrorString(res) << std::endl; ok = false;
    }
    // ── Pixel format ─────────────────────────────────────────
    const char* fmtStr = "RGB8Packed";

    res = GEVSetFeatureEnumeration(camNr, "PixelFormat", fmtStr, (int)strlen(fmtStr));
    if (res != GEV_STATUS_SUCCESS) {
        std::cout << "  PixelFormat failed: " << GEVGetErrorString(res) << std::endl; ok = false;
    }

    // ── Exposure ─────────────────────────────────────────────
    // ExposureTime is a float feature (microseconds)
    res = GEVSetFeatureFloat(camNr, "ExposureTime", exposureUs);
    if (res != GEV_STATUS_SUCCESS) {
        std::cout << "  ExposureTime failed: " << GEVGetErrorString(res) << std::endl; ok = false;
    }

    // ── Gain ─────────────────────────────────────────────────
    const char* gain = strGain.c_str();
    res = GEVSetFeatureEnumeration(camNr, "NED_AnalogGain", gain, (int)strlen(gain));
    if (res != GEV_STATUS_SUCCESS) {
        cout << "Set Gain Failed" << endl;
    }

    // ── Line rate (free-run mode) ─────────────────────────────
    // AcquisitionLineRate controls scan speed when not using encoder trigger
    if (!g_bUseTrigger) {
        res = GEVSetFeatureFloat(camNr, "AcquisitionLineRate", lineRateHz);
        if (res != GEV_STATUS_SUCCESS)
            std::cout << "  AcquisitionLineRate Failed" << std::endl;
    }

    if (ok)
        std::cout << "[LineCameraController] Cam" << (int)camNr << " settings applied OK" << std::endl;

    return ok;
}
bool ConfigureTrigger(SPHINX_CAMNR camNr)
{
    WORD res;

    if (g_bUseTrigger) {
        std::cout << "[ConfigureTrigger] Cam" << (int)camNr
            << " Hardware (" << LINE_CAM_TRIGGER_SOURCE
            << " " << LINE_CAM_TRIGGER_ACTIVATION << ")" << std::endl;

        // Try LineStart first, fall back to FrameStart
        res = GEVSetFeatureEnumeration(camNr, "TriggerSelector", "FrameActive", (int)strlen("FrameActive"));
        if (res != GEV_STATUS_SUCCESS) { cout << "TriggerSelectorErr" << endl; }

        res = GEVSetFeatureEnumeration(camNr, "TriggerMode", "On", (int)strlen("On"));
        if (res != GEV_STATUS_SUCCESS) {
            std::cout << "  TriggerMode On failed: " << GEVGetErrorString(res) << std::endl; return false;
        }

        res = GEVSetFeatureEnumeration(camNr, "TriggerSource", LINE_CAM_TRIGGER_SOURCE, (int)strlen(LINE_CAM_TRIGGER_SOURCE));
        if (res != GEV_STATUS_SUCCESS) {
            std::cout << "  TriggerSource failed: " << GEVGetErrorString(res) << std::endl; return false;
        }

        res = GEVSetFeatureEnumeration(camNr, "TriggerActivation", LINE_CAM_TRIGGER_ACTIVATION, (int)strlen(LINE_CAM_TRIGGER_ACTIVATION));
        if (res != GEV_STATUS_SUCCESS) {
            cout << "  TriggerActivation failed" << endl;
        }
    } else {
        std::cout << "[ConfigureTrigger] Cam" << (int)camNr << " Free-run" << std::endl;
        res = GEVSetFeatureEnumeration(camNr, "TriggerSelector", "FrameActive", (int)strlen("FrameActive"));
        if (res != GEV_STATUS_SUCCESS) {
            cout << "FrameActive Failed" << endl;
        }
        res = GEVSetFeatureEnumeration(camNr, "TriggerMode", "Off", (int)strlen("Off"));
        if (res != GEV_STATUS_SUCCESS) {
            cout << "TriggerMode Off Failed" << endl;
        }
        res = GEVSetFeatureEnumeration(camNr, "TriggerSelector", "ExposureStart", (int)strlen("ExposureStart"));
        if (res != GEV_STATUS_SUCCESS) {
            cout << "ExposureStart Failed" << endl;
        }
        res = GEVSetFeatureEnumeration(camNr, "TriggerMode", "Off", (int)strlen("Off"));
        if (res != GEV_STATUS_SUCCESS) {
            cout << "TriggerMode Off Failed" << endl;
        }
    }
    return true;
}

bool RunAcquisitionLoop(SPHINX_CAMNR camNr,
    std::vector<std::vector<BYTE>>& imgBufs,
    int nTotalHeight, int imgWidth, int imgHeight,
    cv::Mat& outAssembled)
{
    int bufCount = (int)imgBufs.size();
    int bufIndex = 0;
    int currentLine = 0;

    // Pre-allocate the output mat (linesPerImage rows × imgWidth cols, 8-bit color)
    outAssembled = cv::Mat::zeros(nTotalHeight, imgWidth, CV_8UC3);

    while (currentLine < nTotalHeight) {
        IMAGE_HEADER hdr = { 0 };
        BYTE* buf = imgBufs[bufIndex].data();

        WORD res = GEVGetImageBuffer(camNr, &hdr, buf); // blocking
        cout << "Test CamNr  " << (int)camNr << endl;
        if (res != GEV_STATUS_SUCCESS) {
            std::cout << "[AcquisitionLoop] Cam"
                << " GEVGetImageBuffer error: " << GEVGetErrorString(res) << std::endl;
            // Transient error — keep trying unless stop requested
            continue;
        }

        // ── Drop detection ────────────────────────────────────
        if (hdr.LostFrames > 0) {
            std::cout << "[AcquisitionLoop] Cam"
                << " LINE DROP: " << hdr.LostFrames
                << " lines lost." << std::endl;
        }

        // ── Missing packets in this line ──────────────────────
        if (hdr.MissingPacket > 0)
            std::cout << "[AcquisitionLoop] Cam"
            << " missing packets: " << hdr.MissingPacket << std::endl;

        // ── Copy scan line into assembler ─────────────────────
        //memcpy(outAssembled.ptr(currentLine), buf, hdr.SizeX);
        cv::Mat matData(hdr.SizeY + hdr.PaddingY, hdr.SizeX + hdr.PaddingX, CV_8UC3, buf);
        //cv::Mat matData(hdr.SizeY, hdr.SizeX, CV_8UC3, buf);
        matData.copyTo(outAssembled(cv::Rect(0, currentLine, matData.cols, matData.rows)));
        currentLine += imgHeight;

        bufIndex = (bufIndex + 1) % bufCount;
        cout << "Current line : " << currentLine << endl;
    }


    cv::cvtColor(outAssembled, outAssembled, cv::COLOR_BGR2RGB);

    std::cout << "[AcquisitionLoop] Cam assembled " << currentLine << " lines" << std::endl;
    return true;
}



void TestRun()
{
    PParameter pPara = PParameter(new Parameter());
    bool bDebugFlag = true;

    static TParas tp = []() -> TParas {

        return tp;
        }();

    int nProcess = 0;
    int nProcessOld = 1;

    if (bDebugFlag) {
        bool trackbar_flag = [&]() -> bool {

            CreateWindow(1);
            Track("show", 1, tp.show, 0);
            Track("nProcess", 1, nProcess, 1);
            return true;
            }();
    }
    cv::Mat matShow;
    //Software trigger
    //DWORD cmd = 1;
    //WORD err = GEVSetFeatureCommand(CAM1_NR, "TriggerSoftware", cmd);
    //if (err != GEV_STATUS_SUCCESS) { cout << "TriggerSoftware Failed" << endl; };
    do {
        if (nProcess != nProcessOld) {
            nProcessOld = nProcess;
            RunAcquisitionLoop(CAM1_NR, g_imgBuf1, g_nHeight, g_nWidth, g_nHeight, matShow);
        }

        if (bDebugFlag) {
            vector<pair<Mat, string>> ProcessImages{
                {matShow, "matShow"},
            };

            cvex::ShowProcess(ProcessImages, tp, pPara);

            switch (cv::waitKey(1)) {

            case 's':
                bDebugFlag = false;
                cv::destroyAllWindows();
                break; 
            //case 'l':
                //cout << "Pressed" << endl;
                //err = GEVSetFeatureCommand(CAM1_NR, "TriggerSoftware", cmd);
                //if (err != GEV_STATUS_SUCCESS) { cout << "TriggerSoftware Failed" << endl; };
            }
             
        }
    } while (bDebugFlag);
}

void ShowAllFeatures(SPHINX_CAMNR camNr)
{
    BYTE maxLevel;
    FeatureListPtr featureListPtr;
    WORD error = GEVGetFeatureList(camNr, &featureListPtr, &maxLevel);
    if (featureListPtr == NULL) {
        cout << "Data null" << endl;
        return;
    }
    while (featureListPtr != NULL) {
        for (int i = 0; i < featureListPtr->Level; i++) {
            printf("\t");
        }

        printf("%s\n", featureListPtr->Name);

        featureListPtr = featureListPtr->Next;
    }
}
void ShowEnumList(string strEnumName)
{

    const char* p = strEnumName.c_str();

    FEATURE_PARAMETER ft_param;
    bool error = GEVGetFeatureParameter(CAM1_NR,
        p,
        &ft_param);
    cout << "Enum : " << strEnumName << endl;
    cout << "Enum count : " << (int)ft_param.EnumerationCount << endl;

    for (BYTE i = 0; i < ft_param.EnumerationCount; i++) {
        char enumName[256] = { 0 };

        error = GEVGetFeatureEnumerationName(
            CAM1_NR,
            p,
            i,
            enumName,
            sizeof(enumName));

        cout << i << " : " << enumName << endl;
    }
}