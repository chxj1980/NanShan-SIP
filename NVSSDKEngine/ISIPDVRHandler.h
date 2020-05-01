
#include "IDVRHandler.h"

class ISIPDVRHandler : public IDVRHandler
{
public:
	/***********************网络库接口***********************************/

	//************************************
	// Method:    connectDVR    连接DVR
	// Returns:   bool          连接是否成功
	// Parameter: char * sIP    IP地址
	// Parameter: int nPort     DVR端口号
	// Parameter: char * sUser  用户名
	// Parameter: char * sPwd   密码
	// Parameter: int nChannel  通道号
	//************************************
	virtual bool connectDVR(char* sIP,int nPort,char* sUser,char* sPwd,int nChannel)=0;

	//************************************
	// Method:    connectDVREx                    连接DVR
	// Returns:   bool                            连接是否成功
	// Parameter: PCONNECT_PARAM pCParam          参数结构体具体内容与上面接口同
	//************************************
	virtual bool connectDVREx(PCONNECT_PARAM pCParam) = 0;

	//************************************
	// Method:    disconnectDVR 与DVR断开连接
	// Returns:   void
	//************************************
	virtual void disconnectDVR()=0;

	//************************************
	// Method:    freeLoginInfo     删除登陆信息
	// Returns:   void
	//************************************
	virtual void freeLoginInfo()=0;

	//************************************
	// Method:    checkDVR         检测DVR状态
	// Returns:   LONG             大于0表示DVR登陆成功具体数值为DVR下通道数  -1表示DVR登陆失败
	// Parameter: char * sIP       DVR的IP地址
	// Parameter: int nPort        DVR端口
	// Parameter: char * sUser     用户名
	// Parameter: char * sPwd      密码
	// Parameter: CLInfo pInfo[]   用于保存通道的相关信息
	// Parameter: int nMaxCount    最大通道数
	//************************************
	virtual LONG checkDVR(char* sIP, int nPort, char* sUser, char* sPwd,CLInfo pInfo[],int nMaxCount)=0;

	//************************************
	// Method:    checkDVREx                         检测DVR状态
	// Returns:   LONG                               大于0表示DVR登陆成功具体数值为DVR下通道数  -1表示DVR登陆失败
	// Parameter: PCONNECT_PARAM pCParam             参数结构体具体内容与上面接口差不多，通道号可以为空
	// Parameter: CLInfo pInfo[]                     用于保存通道的相关信息
	// Parameter: int nMaxCount                      最大通道数
	//************************************
	virtual LONG checkDVREx(PCONNECT_PARAM pCParam,CLInfo pInfo[],int nMaxCount)=0;

	//************************************
	// Method:    setDVR           设置DVR图片及叠字符相关参数
	// Returns:   void
	// Parameter: char * sIP       DVR的IP地址
	// Parameter: int nPort        DVR端口
	// Parameter: char * sUser     用户名
	// Parameter: char * sPwd      密码
	// Parameter: CLInfo pInfo[]   用于保存通道的相关信息
	// Parameter: int nInfoCount   最大容量
	//************************************
	virtual void setDVR(char* sIP, int nPort, char* sUser, char* sPwd,CLInfo pInfo[],int nInfoCount)=0;

	//************************************
	// Method:    setDVREx                                   设置DVR图片及叠字符相关参数
	// Returns:   void
	// Parameter: PCONNECT_PARAM pCParam                     参数结构体具体内容与上面接口差不多，通道号可以为空
	// Parameter: CLInfo pInfo[]                             用于保存通道的相关信息
	// Parameter: int nInfoCount                             最大容量
	//************************************
	virtual void setDVREx(PCONNECT_PARAM pCParam,CLInfo pInfo[],int nInfoCount)=0;

	//************************************
	// Method:    releaseHandler    删除句柄
	// Returns:   void
	//************************************
	virtual void releaseHandler()=0;

	//************************************
	// Method:    captureStream                              开始远程播放并从DVR取流
	// Returns:   bool                                       操作成攻与否
	// Parameter: LPCAPTURESTREAMCALLBACK lpCallBack         取流回调函数
	// Parameter: void * pUser                               用户指针，用于区分到底是哪个对象的数据
	// Parameter: int nGetVideoType                          码流类型
	//************************************
	virtual bool captureStream(LPCAPTURESTREAMCALLBACK lpCallBack, void* pUser, int nGetVideoType)=0;

	//************************************
	// Method:    captureStreamEx                            开始远程播放并从DVR取流
	// Returns:   bool                                       操作成攻与否
	// Parameter: PCAPTURE_PARAM pParam                      参数结构体具体内容与上面接口同
	//************************************
	virtual bool captureStreamEx(PCAPTURE_PARAM pParam)=0;

	//************************************
	// Method:    stopCaptureStream     停止
	// Returns:   bool
	//************************************
	virtual bool stopCaptureStream()=0;

	//************************************
	// Method:    getRecordFileEx                       获取历史视频的信息
	// Returns:   int                                   小于0失败 大于0 历史视频数
	// Parameter: int nType                             历史视频类型
	// Parameter: char * startTime                      视频开始时间
	// Parameter: char * endTime                        视频结束时间
	// Parameter: RECORDFILE pRecordFile[]              视频信息数组
	// Parameter: int nMaxFileCount                     最多获取视频数
	//************************************
	virtual int getRecordFileEx(int nType,char* startTime,char* endTime,RECORDFILE pRecordFile[],int nMaxFileCount)=0;

	//************************************
	// Method:    getRecordFile                         获取历史视频的信息
	// Returns:   RECORDFILE*                           视频信息数组
	// Parameter: int nType                             历史视频类型
	// Parameter: char * startTime                      视频开始时间
	// Parameter: char * endTime                        视频结束时间
	// Parameter: int * nFileCount                      最多获取视频数
	//************************************
	virtual RECORDFILE* getRecordFile(int nType,char* startTime,char* endTime,int* nFileCount)=0;

	//************************************
	// Method:    downloadRecordFile                         下载历史视频
	// Returns:   LONG                                       小于0表示失败 其它为下载标识用于停止下载
	// Parameter: char * fileName                            视频名
	// Parameter: char * startTime                           视频开始时间
	// Parameter: char * endTime                             视频结束时间
	// Parameter: int nFileSize                              文件大小（也常用于传入一些其它必需的参数）
	// Parameter: char * saveFile                            保存文件路径
	// Parameter: LPDOWNLOADRECORDFILECALLBACK lpCallBack    下载进度回调
	// Parameter: void * pUser                               用户指针，用于区分到底是哪个对象的数据
	//************************************
	virtual intptr_t downloadRecordFile(char* fileName,char* startTime, char* endTime,int nFileSize,char* saveFile,LPDOWNLOADRECORDFILECALLBACK lpCallBack,void* pUser)=0;

	//************************************
	// Method:    downloadRecordFileEx                       下载历史视频
	// Returns:   LONG                                       小于0表示失败 其它为下载标识用于停止下载
	// Parameter: RECORDFILE RecordFile                      参数结构体，所需与上面接口相同
	// Parameter: char * saveFile                            保存文件路径
	// Parameter: LPDOWNLOADRECORDFILECALLBACK lpCallBack    下载进度回调
	// Parameter: void * pUser                               用户指针，用于区分到底是哪个对象的数据
	//************************************
	virtual intptr_t downloadRecordFileEx(RECORDFILE RecordFile,char* saveFile,LPDOWNLOADRECORDFILECALLBACK lpCallBack,void* pUser)=0;

	//************************************
	// Method:    stopDownloadRecordFile                        停止历史视频下载
	// Returns:   bool                                          操作是否成功
	// Parameter: LONG ldownFileHandle                          下载标识由downloadRecordFile函数产生
	//************************************
	virtual bool stopDownloadRecordFile(intptr_t ldownFileHandle)=0;

	//************************************
	// Method:    replayRecordFile                              远程回放
	// Returns:   LONG                                          回放句柄，供相关接口使用
	// Parameter: int hWnd                                      回放的窗口句柄
	// Parameter: char * fileName                               回放文件名
	// Parameter: char * startTime                              开始时间
	// Parameter: char * endTime                                结束时间
	// Parameter: int nFileSize                                 文件大小（也常用于传入一些其它必需的参数）
	//************************************
	virtual intptr_t replayRecordFile(int hWnd,char* fileName,char* startTime, char* endTime,int nFileSize)=0;

	//************************************
	// Method:    replayRecordFileEx                            远程回放
	// Returns:   LONG                                          回放句柄，供相关接口使用
	// Parameter: int hWnd                                      回放的窗口句柄
	// Parameter: RECORDFILE RecordFile                         录像文件的相关信息
	//************************************
	virtual intptr_t replayRecordFileEx(int hWnd,RECORDFILE RecordFile)=0;

	//************************************
	// Method:    controlReplayRecordFile                       控件远程回放
	// Returns:   bool                                          操作成功与否
	// Parameter: LONG lReplayHandle                            回放句柄
	// Parameter: int nCmd                                      控制指令
	// Parameter: int nInValue                                  一些操件的输入参数
	// Parameter: int * outValue                                一些操件的输出参数
	//************************************
	virtual bool controlReplayRecordFile(intptr_t lReplayHandle,int nCmd,int nInValue,int* outValue)=0;

	//************************************
	// Method:    stopReplayRecordFile                          停止远程回放
	// Returns:   bool                                          操作成功与否
	// Parameter: LONG lReplayHandle                            回放句柄
	//************************************
	virtual bool stopReplayRecordFile(intptr_t lReplayHandle)=0;

	//************************************
	// Method:    startTalk                                     开启语音
	// Returns:   bool                                          操作成功与否
	//************************************
	virtual bool startTalk()=0;

	//************************************
	// Method:    stopTalk                                      关闭语音
	// Returns:   bool                                          操作成功与否
	//************************************
	virtual bool stopTalk()=0;

	//************************************
	// Method:    controlPTZ                                    云台控制
	// Returns:   bool                                          操作成功与否
	// Parameter: int nCmd                                      控制指令
	// Parameter: bool bEnable                                  是启用操件还是停止操件
	//************************************
	virtual bool controlPTZ(int nCmd, bool bEnable)=0;

	//************************************
	// Method:    presetPTZ                                     云台预置位设置
	// Returns:   bool                                          操作成功与否
	// Parameter: int nCmd                                      控件指令
	// Parameter: int nIndex                                    预置点的序号
	//************************************
	virtual bool presetPTZ(int nCmd, int nIndex)=0;

	//************************************
	// Method:    showSettingPane                               显示配置页面
	// Returns:   bool                                          操作成功与否
	//************************************
	virtual bool showSettingPane()=0;

	//************************************
	// Method:    alarm_startCapture                            开始报警回传
	// Returns:   bool                                          操作成功与否
	// Parameter: int nPort                                     监听端口，一般传0就可以了
	// Parameter: LPALARMCALLBACK lpCallBack                    回传回调
	//************************************
	virtual bool alarm_startCapture(int nPort,LPALARMCALLBACK lpCallBack)=0;

	//************************************
	// Method:    alarm_stop                                    停止报警回传
	// Returns:   bool                                          操作成功与否
	//************************************
	virtual bool alarm_stop()=0;

	//************************************
	// Method:    log_QueryEx                                   查看设备日志
	// Returns:   int                                           日志条数
	// Parameter: LOGQUERYTYPE nQueryType                       所需日志类型
	// Parameter: LOGERRORTYPE nErrorType                       错误类型
	// Parameter: char * startTime                              开始时间
	// Parameter: char * endTime                                结束时间
	// Parameter: LOGINFO pLogInfo[]                            日志数组
	// Parameter: int nMaxLogCount                              最多查询日志条数
	//************************************
	virtual int log_QueryEx(LOGQUERYTYPE nQueryType, LOGERRORTYPE nErrorType,char* startTime,char* endTime,LOGINFO pLogInfo[],int nMaxLogCount)=0;

	//************************************
	// Method:    log_Query                                     查看设备日志
	// Returns:   LOGINFO*                                      日志数组
	// Parameter: LOGQUERYTYPE nQueryType                       所需日志类型
	// Parameter: LOGERRORTYPE nErrorType                       错误类型
	// Parameter: char * startTime                              开始时间
	// Parameter: char * endTime                                结束时间
	// Parameter: int * nLogCount                               最多查询日志条数
	//************************************
	virtual LOGINFO* log_Query(LOGQUERYTYPE nQueryType, LOGERRORTYPE nErrorType,char* startTime,char* endTime,int* nLogCount)=0;

	//************************************
	// Method:    setVideoEffect                                设置预览视频显示参数
	// Returns:   bool                                          操作成功与否
	// Parameter: DWORD dwBrightValue                           亮度，取值范围[1,10]
	// Parameter: DWORD dwContrastValue                         对比度，取值范围[1,10]
	// Parameter: DWORD dwSaturationValue                       饱和度，取值范围[1,10]
	// Parameter: DWORD dwHueValue                              色度，取值范围[1,10]
	//************************************
	virtual bool setVideoEffect(DWORD dwBrightValue,DWORD dwContrastValue, DWORD dwSaturationValue,DWORD dwHueValue)=0;

	//************************************
	// Method:    getVideoEffect                                获取预览视频显示参数
	// Returns:   bool                                          操作成功与否
	// Parameter: DWORD * pBrightValue                          亮度，取值范围[1,10]
	// Parameter: DWORD * pContrastValue                        对比度，取值范围[1,10]
	// Parameter: DWORD * pSaturationValue                      饱和度，取值范围[1,10]
	// Parameter: DWORD * pHueValue                             色度，取值范围[1,10]
	//************************************
	virtual bool getVideoEffect(DWORD *pBrightValue,DWORD *pContrastValue, DWORD *pSaturationValue,DWORD *pHueValue)=0;

	//************************************
	// Method:    setDVRConfig                                  设置设备的配置信息
	// Returns:   bool:                                         操作成功与否
	// Parameter: DWORD dwCommand                               设备配置命令
	// Parameter: LONG lChannel                                 通道号，如果命令不需要通道号，该参数无效，置为0xFFFFFFFF即可
	// Parameter: LPVOID lpInBuffer                             输入数据的缓冲指针
	// Parameter: DWORD dwInBufferSize                          输入数据的缓冲长度
	//************************************
	virtual bool setDVRConfig(DWORD dwCommand,LONG lChannel,LPVOID lpInBuffer,DWORD dwInBufferSize)=0;

	//************************************
	// Method:    getDVRConfig                                  获取设备的配置信息
	// Returns:   bool                                          操作成功与否
	// Parameter: DWORD dwCommand                               设备配置命令
	// Parameter: LONG lChannel                                 通道号
	// Parameter: LPVOID lpOutBuffer                            接收数据的缓冲指针
	// Parameter: DWORD dwOutBufferSize                         接收数据的缓冲长度
	// Parameter: LPDWORD lpBytesReturned                       实际收到的数据长度指针
	//************************************
	virtual bool getDVRConfig(DWORD dwCommand,LONG lChannel,LPVOID lpOutBuffer,DWORD dwOutBufferSize,LPDWORD lpBytesReturned)=0;

	//************************************
	// Method:    playBackSaveData                             捕获回放的录像数据，并保存成文件
	// Returns:   bool                                         操作成功与否
	// Parameter: LONG lReplayHandle                           回放句柄
	// Parameter: char * sFileName                             保存数据的文件路径
	//************************************
	virtual bool playBackSaveData(LONG lReplayHandle,char *sFileName)=0;

	//************************************
	// Method:    stopPlayBackSave                             停止捕获回放的录像数据
	// Returns:   bool                                         操作成功与否
	// Parameter: LONG lReplayHandle                           回放句柄
	//************************************
	virtual bool stopPlayBackSave(LONG lReplayHandle)=0;

	//************************************
	// Method:    getDownloadPos                               获取下载进度
	// Returns:   int                                          下载进度
	// Parameter: LONG lFileHandle                             下载句柄
	//************************************
	virtual int getDownloadPos(LONG lFileHandle)=0;

	//************************************
	// Method:    getDVRWorkState                              获取设备的工作状态
	// Returns:   bool                                         操作成功与否
	// Parameter: char dvrWorkState[]                          获取的设备工作状态结构体参数
	// Parameter: int arrayLen                                 最大容量
	//************************************
	virtual bool getDVRWorkState(char dvrWorkState[],int arrayLen)=0;

	//************************************
	// Method:    getDVRIPByResolveSvr                         通过解析服务器，获取设备的动态IP地址
	// Returns:   bool                                         操作成功与否
	// Parameter: char * sServerIP                             解析服务器的IP地址
	// Parameter: int wServerPort                              解析服务器的端口号
	// Parameter: char * sDVRName                              设备名称
	// Parameter: int wDVRNameLen                              设备名称的长度
	// Parameter: char * sDVRSerialNumber                      设备的序列号
	// Parameter: int wDVRSerialLen                            设备序列号的长度
	// Parameter: char * sGetIP                                获取到的设备IP地址指针
	//************************************
	virtual bool getDVRIPByResolveSvr(char *sServerIP,int wServerPort,char *sDVRName,int wDVRNameLen,char *sDVRSerialNumber,int wDVRSerialLen,char* sGetIP)=0;

	//************************************
	// Method:    setConnectTime                               设置网络连接超时时间和连接尝试次数
	// Returns:   bool                                         操作成功与否
	// Parameter: DWORD dwWaitTime                             超时时间，单位毫秒
	// Parameter: DWORD dwTryTimes                             连接尝试次数
	//************************************
	virtual bool setConnectTime(DWORD dwWaitTime,DWORD dwTryTimes)=0;

	//************************************
	// Method:    startListen                                  启动监听，接收设备主动上传的报警等信息
	// Returns:   bool                                         操作成功与否
	// Parameter: char * sLocalIP                              PC机本地IP地址
	// Parameter: int wLocalPort                               PC本地监听端口号。
	//************************************
	virtual bool startListen (char *sLocalIP,int wLocalPort)=0;

	//************************************
	// Method:    stopListen                                  停止监听，接收设备主动上传的报警等信息
	// Returns:   bool                                        操作成功与否
	//************************************
	virtual bool stopListen()=0;

	//************************************
	// Method:    serialSend                                  设备串口发送数据
	// Returns:   bool                                        操作成功与否
	// Parameter: char * pBuf                                 发送数据的缓冲区指针
	// Parameter: DWORD nLength                               缓冲区的大小，最多1016字节
	// Parameter: char * pBuf2                                发送数据的缓冲区指针
	// Parameter: DWORD nLen2                                 缓冲区的大小，最多1016字节
	//************************************
	virtual bool serialSend(char* pBuf,DWORD nLength,char* pBuf2,DWORD nLen2)=0;

	//************************************
	// Method:    serialSendWithCallBack                      带回调信息的设备串口发送数据
	// Returns:   bool                                        操作成功与否
	// Parameter: char * pBuf                                 发送数据的缓冲区指针
	// Parameter: DWORD nLength                               缓冲区的大小，最多1016字节
	// Parameter: int nCameraID                               镜头ID
	//************************************
	virtual bool serialSendWithCallBack(char* pBuf,DWORD nLength,int nCameraID)=0;

	//************************************
	// Method:    buildSerialSend                             建立设备串口发送数据的通道
	// Returns:   bool                                        操作成功与否
	// Parameter: LONG lSerialPort                            串口号:1－232串口；2－485串口
	// Parameter: LPSERIALDATACALLBACK pSerialCallBack        回调函数
	// Parameter: bool bOpen                                  true  打开   false 关闭
	//************************************
	virtual bool buildSerialSend(LONG lSerialPort,LPSERIALDATACALLBACK pSerialCallBack,bool bOpen)=0;

	//************************************
	// Method:    makeKey                                     产生一个关键帧
	// Returns:   void
	//************************************
	virtual void makeKey()=0;

	//************************************
	// Method:    clientAudioStart                           启动语音广播的PC端声音捕获
	// Returns:   bool                                       操作成功与否
	//************************************
	virtual bool clientAudioStart()=0;

	//************************************
	// Method:    clientAudioStop                            关闭语音广播的PC端声音捕获
	// Returns:   bool                                       操作成功与否
	//************************************
	virtual bool clientAudioStop()=0;

	//************************************
	// Method:    addDVR                                     添加设备到可以接收PC机声音的广播组
	// Returns:   bool                                       操作成功与否
	//************************************
	virtual bool addDVR()=0;

	//************************************
	// Method:    delDVR                                     从可接收PC机声音的广播组里删除设备
	// Returns:   bool                                       操作成功与否
	//************************************
	virtual bool delDVR()=0;

	//************************************
	// Method:    setVoiceComClientVolume                    设置语音对讲客户端的音量
	// Returns:   bool                                       操作成功与否
	// Parameter: WORD wVolume                               设置音量，取值范围[0,0xffff]
	//************************************
	virtual bool setVoiceComClientVolume(WORD wVolume)=0;

	//************************************
	// Method:    controlPTZSpeed                            带速度的云台控制
	// Returns:   bool                                       操作成功与否
	// Parameter: int nCmd                                   操作指令
	// Parameter: bool bEnable                               动还是停
	// Parameter: int nSpeed                                 速度
	//************************************
	virtual bool controlPTZSpeed(int nCmd, bool bEnable,int nSpeed)=0;

	//************************************
	// Method:    getDecoderAddress                          获取解码器参数
	// Returns:   int                                        解码器// Parameter:0 - 255
	// Parameter: LONG lChannelIndex                         通道号
	//************************************
	virtual int getDecoderAddress(LONG lChannelIndex)=0;

	//************************************
	// Method:    controlPTZSpeedWithChannel                 带通道和速度的云台控制
	// Returns:   bool                                       操作成功与否
	// Parameter: int nCmd                                   操作指令
	// Parameter: bool bEnable                               动还是停
	// Parameter: int nSpeed                                 速度
	// Parameter: int nChannel                               通道号
	//************************************
	virtual bool controlPTZSpeedWithChannel(int nCmd, bool bEnable, int nSpeed, int nChannel)=0;

	//************************************
	// Method:    presetPTZWithChannel                       带通首的云台预置点设置
	// Returns:   bool                                       操作成功与否
	// Parameter: int nCmd                                   操作指令
	// Parameter: int nIndex                                 预置点编号
	// Parameter: int nChannel                               通道号
	//************************************
	virtual bool presetPTZWithChannel(int nCmd, int nIndex, int nChannel)=0;

	//************************************
	// Method:    controlPTZSpeedWithChannelEx               带通道和速度的云台控制
	// Returns:   bool                                       操作成功与否
	// Parameter: PCONTROL_PARAM pParam                      云台控制相关结构体
	//************************************
	virtual bool controlPTZSpeedWithChannelEx(PCONTROL_PARAM pParam)=0;

	//************************************
	// Method:    getPlayBackOsdTime                         获取录像回放时的OSD时间
	// Returns:   bool                                       操作成功与否
	// Parameter: LONG lReplayHandle                         播放句柄
	// Parameter: char * pOsdTime                            获取的OSD时间的指针
	//************************************
	virtual bool getPlayBackOsdTime(LONG  lReplayHandle, char *pOsdTime)=0;

	//************************************
	// Method:    setCurrentChannel                          设置当前通道
	// Returns:   void
	// Parameter: int nChannel                               通道号
	//************************************
	virtual void setCurrentChannel(int nChannel)=0;

	//************************************
	// Method:    playBackCaptureFile                        录像回放时抓图，并保存在文件中
	// Returns:   bool                                       操作成功与否
	// Parameter: LONG lReplayHandle                         播放句柄
	// Parameter: char * pFileName                           保存图片数据的文件路径
	//************************************
	virtual bool playBackCaptureFile(LONG lReplayHandle, char *pFileName)=0;

	//************************************
	// Method:    setCapturePictureMode                      设置当前截图模式
	// Returns:   bool                                       操作成功与否
	// Parameter: DWORD dwCaptureMode                        截图模式
	//************************************
	virtual bool setCapturePictureMode(DWORD dwCaptureMode)=0;

	//************************************
	// Method:    captureFileStream                          获取文件流
	// Returns:   LONG                                       文件流句柄
	// Parameter: LPCAPTURESTREAMCALLBACK lpCallBack         回调函数
	// Parameter: void * pUser                               用户指针
	// Parameter: int nStreamType                            流类型
	// Parameter: RECORDFILE FileInfo                        文件信息
	// Parameter: char * sType                               未知
	//************************************
	virtual intptr_t captureFileStream(LPCAPTURESTREAMCALLBACK lpCallBack, void* pUser, int nStreamType, RECORDFILE FileInfo, char* sType)=0;

	//************************************
	// Method:    getCameraInfo                              获取第三方软件的镜头信息
	// Returns:   LONG                                       未知
	// Parameter: int * nSize                                未知
	//************************************
	virtual LONG getCameraInfo(int *nSize)=0;

	//************************************
	// Method:    setPlayWindow                              设置播放窗口句柄(华三)
	// Returns:   int                                        操件结果 具体意思未知
	// Parameter: HWND palyWindow[]                          窗口句柄数组
	// Parameter: int nMaxCount                              最大容量
	//************************************
	virtual int setPlayWindow(HWND palyWindow[], int nMaxCount)=0;

	//************************************
	// Method:    startPlayerByCamera                        播放镜头到窗口句柄(华三)
	// Returns:   bool                                       操作成功与否
	// Parameter: HWND hwndPlay                              窗口句柄
	// Parameter: int nCameraID                              镜头ID
	//************************************
	virtual bool startPlayerByCamera(HWND hwndPlay, int nCameraID)=0;

	//************************************
	// Method:    stopPlayerByCamera                         停止播放窗口句柄(华三)
	// Returns:   bool                                       操作成功与否
	// Parameter: HWND hwndPlay                              窗口句柄
	// Parameter: int nCameraID                              镜头ID
	//************************************
	virtual bool  stopPlayerByCamera(HWND hwndPlay, int nCameraID)=0;

	//************************************
	// Method:    setCurrentCameraCode                       设置当前镜头编号
	// Returns:   void
	// Parameter: char * sCameraCode                         镜头编号
	//************************************
	virtual void setCurrentCameraCode(char* sCameraCode)=0;

	//************************************
	// Method:    startRecord                                开始录像
	// Returns:   bool                                       操作成功与否
	// Parameter: char * saveFile                            录像保存的路径
	//************************************
	virtual bool startRecord(char* saveFile)=0;

	//************************************
	// Method:    stopRecord                                 停止录像
	// Returns:   bool                                       操作成功与否
	//************************************
	virtual bool stopRecord()=0;

	/***********************播放库接口***********************************/

	//************************************
	// Method:    initialPlayer                              初始化播放
	// Returns:   bool                                       操作成功与否
	// Parameter: int nPort                                  播放通道
	// Parameter: int nDecodeType                            解码类型
	// Parameter: BYTE * buff                                帧缓存
	// Parameter: int nSize                                  缓存大小
	// Parameter: int nStreamType                            流类型
	//************************************
	virtual bool initialPlayer(int nPort,int nDecodeType,BYTE* buff, int nSize, int nStreamType)=0;

	//************************************
	// Method:    initialPlayerEx                            初始化播放
	// Returns:   bool                                       操作成功与否
	// Parameter: PINITPY_PARAM pIParam                      参数结构体具体内容与上面接口同
	//************************************
	virtual bool initialPlayerEx(PINITPY_PARAM pIParam)=0;

	//************************************
	// Method:    startPlayer                                开始播放
	// Returns:   bool                                       操作成功与否
	// Parameter: HWND hWnd                                  播放窗口句柄
	//************************************
	virtual bool startPlayer(HWND hWnd)=0;

	//************************************
	// Method:    startPlayer2                               开始播放
	// Returns:   bool                                       操作成功与否
	// Parameter: HWND hWnd                                  播放窗口句柄
	// Parameter: LPDRAWCBCALLLBACK lpDrawCBFun              回调函数
	// Parameter: void * pUser                               用户数据
	//************************************
	virtual bool startPlayer2(HWND hWnd, LPDRAWCBCALLLBACK lpDrawCBFun, void *pUser)=0;

	//************************************
	// Method:    startPlayer4Standard                       开始播放
	// Returns:   bool                                       操作成功与否
	// Parameter: HWND hWnd                                  播放窗口句柄
	// Parameter: LPDecCBFun lpDecCBFun                      回调函数
	// Parameter: int nCameraID                              镜头ID
	//************************************
	virtual bool startPlayer4Standard(HWND hWnd,LPDecCBFun lpDecCBFun, int nCameraID)=0;

	//************************************
	// Method:    startPlayer4StandardEx                     开始播放
	// Returns:   bool                                       操作成功与否
	// Parameter: PSTARTPYS_PARAM pParam                     参数结构体具体内容与上面接口同
	//************************************
	virtual bool startPlayer4StandardEx(PSTARTPYS_PARAM pParam)=0;

	//************************************
	// Method:    addData                                    向解码器中加入数据
	// Returns:   bool                                       操作成功与否
	// Parameter: BYTE * buff                                数据缓存
	// Parameter: int nSize                                  缓存长度
	//************************************
	virtual bool addData(BYTE* buff, int nSize)=0;

	//************************************
	// Method:    stopPlayer                                 停止播放
	// Returns:   bool                                       操作成功与否
	//************************************
	virtual bool stopPlayer()=0;

	//************************************
	// Method:    locPlay_OpenFile                           播放本地文件
	// Returns:   bool                                       操作成功与否
	// Parameter: int nPort                                  播放通道号
	// Parameter: int nDecodeType                            解码类型
	// Parameter: int hWnd
	// Parameter: char * fileName
	//************************************
	virtual bool locPlay_OpenFile(int nPort,int nDecodeType,int hWnd,char* fileName)=0;

	//************************************
	// Method:    locPlay_OpenFileEx                         播放本地文件
	// Returns:   bool                                       操作成功与否
	// Parameter: PLOCPY_OPENF_PARAM_PARAM pParam            参数结构体具体内容与上面接口同
	//************************************
	virtual bool locPlay_OpenFileEx(PLOCPY_OPENF_PARAM_PARAM pParam)=0;

	//************************************
	// Method:    locPlay_Control                            本地播放控制
	// Returns:   bool                                       操作成功与否
	// Parameter: int nCmd                                   操作指令
	// Parameter: int nInValue                               输入参数
	// Parameter: int * outValue                             输出参数
	//************************************
	virtual bool locPlay_Control(int nCmd,int nInValue,int* outValue)=0;

	//************************************
	// Method:    locPlay_Stop                               本地播放停止
	// Returns:   bool                                       操作成功与否
	//************************************
	virtual bool locPlay_Stop()=0;

	//************************************
	// Method:    convertToBmpFile                           将抓图得到的图像数据保存成BMP文件
	// Returns:   bool                                       操作成功与否
	// Parameter: char * pBuf                                抓图回调函数中图像缓冲区
	// Parameter: LONG nSize                                 抓图回调函数中图像的大小
	// Parameter: LONG nWidth                                抓图回调函数中图像宽度
	// Parameter: LONG nHeight                               抓图回调函数中图像高度
	// Parameter: LONG nType                                 抓图回调函数中图像类型
	// Parameter: char * sFileName                           要保存的文件名。最好以BMP作为文件扩展名
	//************************************
	virtual bool convertToBmpFile(char * pBuf,LONG nSize,LONG nWidth,LONG nHeight,LONG nType,char *sFileName)=0;

	//************************************
	// Method:    oneByOneBack                               单帧回退
	// Returns:   bool                                       操作成功与否
	// Parameter: LONG nPort                                 解码器通道
	//************************************
	virtual bool oneByOneBack(LONG nPort)=0;

	//************************************
	// Method:    oneByOne                                   单帧播放
	// Returns:   bool                                       操作成功与否
	// Parameter: LONG nPort                                 解码器通道
	//************************************
	virtual bool oneByOne(LONG nPort)=0;

	//************************************
	// Method:    setPlayedTimeEx                            设置播放时间
	// Returns:   bool                                       操作成功与否
	// Parameter: LONG nPort                                 解码器通道
	// Parameter: DWORD nTime                                播放时间
	//************************************
	virtual bool setPlayedTimeEx(LONG nPort,DWORD nTime)=0;

	//************************************
	// Method:    setFileRefCallBack                         PlayM4_SetFileRefCallBack
	// Returns:   bool                                       操作成功与否
	// Parameter: LONG nPort                                 解码器通道
	// Parameter: void (__stdcall *pFileRefDone) (DWORD nPort,DWORD nUser)
	//                                                       回调
	// Parameter: DWORD nUser                                用户数据
	//************************************
	virtual bool setFileRefCallBack(LONG nPort,void (__stdcall *pFileRefDone) (DWORD nPort,DWORD nUser),DWORD nUser)=0;

	//************************************
	// Method:    setDisplayCallBack                         设置抓图回调函数
	// Returns:   bool                                       操作成功与否
	// Parameter: LONG nPort                                 解码器通道
	// Parameter: void (__stdcall *pFileRefDone)(long nPort,char * pBuf,long nSize,long nWidth,long nHeight,long nStamp,long nType,long nReserved)
	//
	//************************************
	virtual bool setDisplayCallBack(LONG nPort, void (__stdcall *pFileRefDone)(long nPort,char * pBuf,long nSize,long nWidth,long nHeight,long nStamp,long nType,long nReserved))=0;

	//************************************
	// Method:    capturePicture                             截图
	// Returns:   bool                                       操作成功与否
	// Parameter: char * sFilePath                           保存路径
	//************************************
	virtual bool capturePicture(char* sFilePath)=0;

	//************************************
	// Method:    reSetDDrawDevice                           刷新显示
	// Returns:   void
	// Parameter: HWND hWnd                                  播放窗口句柄
	//************************************
	virtual void reSetDDrawDevice(HWND hWnd)=0;

	//************************************
	// Method:    controlCameraSound                         控件播放声音
	// Returns:   bool                                       操作成功与否
	// Parameter: bool bOpenSound                            true 开声音 false 关
	//************************************
	virtual bool controlCameraSound(bool bOpenSound)=0;

	//************************************
	// Method:    openVideoFile                              打开视频文件
	// Returns:   LONG                                       视频文件句柄
	// Parameter: char * sFileName                           视频文件
	//************************************
	virtual intptr_t openVideoFile(char *sFileName)=0;

	//************************************
	// Method:    readVideoFile                              从视频文件中读取视频数据
	// Returns:   int                                        文件读取结果
	// Parameter: char * pBuff                               视频数据缓存
	// Parameter: int nLen                                   缓存大小
	// Parameter: int * pOutStreamType                       流类型
	//************************************
	virtual int readVideoFile(char *pBuff,int nLen, int *pOutStreamType)=0;

	//************************************
	// Method:    closeVideoFile                             关闭视频数据
	// Returns:   bool                                       操作成功与否
	//************************************
	virtual bool closeVideoFile()=0;

	//************************************
	// Method:    controlPlay                                播放控制
	// Returns:   bool                                       操作成功与否
	// Parameter: int nCmd                                   控制指令
	// Parameter: int param                                  播放进度
	// Parameter: int * outValue                             输出参数（获取到的进度等等）
	//************************************
	virtual bool controlPlay(int nCmd,int param, int *outValue)=0;

	//************************************
	// Method:    ClearPlayBuffer                            清除播放缓存
	// Returns:   void
	//************************************
	virtual void ClearPlayBuffer()=0;

	//************************************
	// Method:    pusePlayDecode                             暂停/播放
	// Returns:   void
	// Parameter: bool bPuse                                 true 暂停  false 播放
	//************************************
	virtual void pusePlayDecode(bool bPuse)=0;


	//zjf 20130926
	//************************************
	//局部放大
	/*
	// Parameter: bFlag: 无意义
	// 			  x1, y1, x2, y2分别为鼠标在播放窗体上的左上及右下的坐标, 全部为0时表示停止局部放大.
	// Returns:true 成功, false 失败*/
	//************************************
	virtual bool SetCropArea(bool bFlag, int x1, int y1, int x2, int y2)=0;
	//************************************
	//视频解码智能调节
	/*
	// Parameter: nCmd: 智能调节的类型, 1为去雾, 2为反色, 3为锐化, 4为亮度调节
	//			  bFlag: true为执行智能调节, false为取消
	//			  x1, y1: 去雾时表示起始点坐标, 高度调节时为参数
	//			  x2, y2: 亮度调节表示终结点坐标.
	// Returns: 0成功, <0失败*/
	//************************************
	virtual bool SetDecodeParam(int nCmd, bool bFlag, int x1, int y1, int x2, int y2)=0; 

	//************************************
	//设置YUV回调函数
	/*
	// Parameter: pCallBack: 回调函数, 定义见上面.
	// Returns:true 成功, false 失败*/
	//************************************
	virtual bool SetYUVCallBack(LPYUVCALLBACK pCallBack) = 0;
	/****以下三个接口是为了实现在客户端播放实时视频时的15秒快速回放****/

	//************************************
	//设置实时播放时保存流数据的时间, 默认15秒
	/*
	// Parameter:  identify: 句柄
	//			   nSaveTime: 保存的时间(单位:秒）
	// Returns: true成功, false失败*/
	//************************************
	virtual bool SetStreamSaveTime(int nSaveTime)=0;
	//************************************
	//回放保存的视频流
	/*
	// Parameter:  identify: 句柄;
	//			   hWnd: 窗口句柄
	// Returns: true成功, false失败*/
	//************************************
	virtual bool PlayBackSaveStream(HWND hWnd)=0;
	//************************************
	//停止回放保存的视频流
	/*
	// Parameter:  identify: 句柄
	// Returns: true成功, false失败*/
	//************************************
	virtual bool StopPlayBackSaveStream()=0;
	//zjf 20130926 end

	//pzm 20130927
	//************************************
	//设置图像的渲染模式，默认为D3D渲染
	/*
	// Parameter:nMode 渲染模式
	// OutParameter:无
	// Returns:成功返回true，否则返回false
	*/
	//************************************
	virtual bool SetRenderMode(int nMode)=0;

	//************************************
	//根据帧号定位,定位至nFrameNum最近的关键帧（往前查找）
	/*
	// Parameter:nFrameNum 帧号
	// OutParameter:无
	// Returns:成功返回true，否则返回false
	*/
	//************************************
	virtual bool SetPosByFrameNum(DWORD nFrameNum)=0;

	//************************************
	//截取beginTime和endTime之间的录像，保存至filename所指定的路径，beginTime和endTime为秒级时间
	/*
	// Parameter:beginFrame:截取开始的时间；
	//			 endFrame:截取结束的时间；
	//			 filename:录像保存的绝对路径
	// OutParameter:无
	// Returns:成功返回true，否则返回false
	*/
	//************************************
	virtual bool GetVideoCut(DWORD beginFrame, DWORD endFrame, LPSTR filename)=0;
	//************************************
	//中止文件剪切
	/*
	// Parameter:
	// OutParameter:无
	// Returns:成功返回true，否则返回false
	*/
	//************************************
	virtual bool StopCutVideo()=0;
	//************************************
	//设置文件结束回调
	/*
	// Parameter:
	// OutParameter:无
	// Returns:成功返回true，否则返回false
	*/
	//************************************
	virtual bool SetFileEndCallback(void *pUser, pFileOverCB pCallBack)=0;
	//************************************
	//获取媒体信息
	/*
	// Parameter:mInf用来获取媒体信息的结构指针
	// OutParameter:无
	// Returns:成功返回true，否则返回false
	*/
	//************************************
	virtual bool GetMediaInfo(MediaInf &mInf)=0;
	//************************************
	//视频解码获取配置参数
	/*
	// Parameter: nCmd: 智能调节的类型, 1为去雾, 2为反色, 3为锐化, 4为亮度调节				
	// OutParameter:
	//				当nCmd为1去雾调节时:
	//				x1				起始点X坐标
	//				y1				起始点Y坐标
	//				x2				结束点X坐标
	//				y2				结束点Y坐标
	//				当nCmd为4亮度调节时:
	//				x1 为gray1	亮度调节参数的灰度值1
	//				y1 为gray2	亮度调节参数的灰度值2
	// Returns: 0成功, <0失败*/
	//************************************
	virtual bool GetDecodeParam(int nCmd, int &x1, int &y1, int &x2, int &y2)=0;
	//pzm 20130927 end

	/***********************公用接口***********************************/

	//************************************
	// Method:    queryPorperySupport                        查询性能支持
	// Returns:   bool                                       true 支持   false不支持
	// Parameter: XYPROPROPERTY propery                      性能参数
	//************************************
	virtual bool queryPorperySupport(XYPROPROPERTY propery)=0;

	//************************************
	// Method:    getLastError                               获取最后的错误码
	// Returns:   DWORD                                      错误编码
	//************************************
	virtual DWORD getLastError()=0;

	//2013-09-27 增加 协议接口 AllpurposeI
	//************************************
	// Method:    AllpurposeI                               获取最后的错误码
	// Returns:   int  0成功,-1失败                         操作成功与否
	// Parameter: char * pInPutPara ,char *pOutPutPara
	//************************************
	int  AllpurposeI(int nCmd,char *pInPutPara,char *pOutPutPara);
//Exteral Function:
	//************************************
	// Method:    Xsip_init									  初始化SIP
	// Returns:   bool										  true成功,false失败     操作成功与否	 
	// Parameter: pSipMsgCallBack pCB                         SIP回调函数
	// Parameter: void *pUser							      用户指针
	//************************************
	virtual bool Xsip_init(pNVSSipCallback pCB, void *pUser) = 0;

	//************************************
	// Method:    startPlayCamera                           播放实时视频
	// Returns:   bool                                      操作成功与否
	// Parameter: char* sCameraCode                         镜头ID
	// Parameter: LPREALSTREAMCALLBACK lpCallBack           实时流回调函数
	// Parameter: void* pUser                               用户指针，用于区分到底是哪个对象的数据
	//************************************
	virtual bool startPlayCamera(char* sCameraCode, LPREALSTREAMCALLBACK lpCallBack = NULL, void* pUser = NULL)=0;

	//************************************
	// Method:    stopPlayCamera                            停止播放实时视频
	// Returns:   bool                                      操作成功与否
	//************************************
	virtual bool stopPlayCamera(void)=0;

	//************************************
	// Method:    startPlayMonitor                          视频上墙
	// Returns:   bool                                       操作成功与否
	// Parameter: char* sCameraCode                          镜头ID
	// Parameter: char *sMonitorCode                         监视器ID
	//************************************
	virtual bool startPlayMonitor(char* sCameraCode, char *sMonitorCode)=0;

	//************************************
	// Method:    stopPlayerByCamera                         停止视频上墙
	// Returns:   bool                                       操作成功与否
	//************************************
	virtual bool stopPlayMonitor(void)=0;

	//************************************
	// Method:    getRecordFileBySource                 根据视频源获取历史视频的信息, 查找到的信息在pSipMsgCallBack中返回
	// Returns:   bool									操作成功与否
	// Parameter: int nType                             历史视频类型
	// Parameter: char * startTime                      视频开始时间
	// Parameter: char * endTime                        视频结束时间
	// Parameter: int nRecordSource                     视频源1:设备2：平台
	//************************************
	virtual bool getRecordFileBySource(int nType, char* startTime, char* endTime, int nRecordSource)=0;

	//************************************
	// Method:    downloadRecordFileBySource                 下载历史视频
	// Returns:   LONG                                       小于0表示失败 其它为下载标识用于停止下载
	// Parameter: char * fileName                            视频名
	// Parameter: char * startTime                           视频开始时间
	// Parameter: char * endTime                             视频结束时间
	// Parameter: int nFileSize                              文件大小（也常用于传入一些其它必需的参数）
	// Parameter: char * saveFile                            保存文件路径
	// Parameter: LPDOWNLOADRECORDFILECALLBACK lpCallBack    下载进度回调
	// Parameter: void * pUser                               用户指针，用于区分到底是哪个对象的数据
	// Parameter: int nRecordSource							 视频源 1: 设备 2：平台
	//************************************
	virtual LONG downloadRecordFileBySource(char* fileName,char* startTime, char* endTime,int nFileSize,char* saveFile,LPDOWNLOADRECORDFILECALLBACK lpCallBack,void* pUser, int nRecordSource)=0;

	//************************************
	// Method:    replayRecordFileWithCB					 带回调函数的远程回放
	// Returns:   LONG                                       回放句柄，供相关接口使用
	// Parameter: char * pFileName                           回放文件名
	// Parameter: char * pStartTime                          开始时间
	// Parameter: char * pEndTime                            结束时间
	// Parameter: int nFileSize                              文件大小（也常用于传入一些其它必需的参数）
	// Parameter: LPREPLAYCALLBACK lpCallBack				 回放流回调函数
	// Parameter: void * pUser                               用户指针，用于区分到底是哪个对象的数据
	//************************************
	virtual LONG replayRecordFileWithCB(char* pFileName,char* pStartTime, char* pEndTime,int nFileSize,LPREPLAYCALLBACK lpCallBack,void* pUser)=0;

	//************************************
	// Method:    presetPTZWithName                          带名称的云台预置点设置
	// Returns:   bool                                       操作成功与否
	// Parameter: int nCmd                                   操作指令
	// Parameter: int nParam                                 参数(速度或预置位）
	// Parameter: int nGroup                                 巡航组号
	// Parameter: char * pPosName                            预置位名称
	//************************************
	virtual bool presetPTZWithName(int nCmd, int nParam, int nGroup, char* pPosName)=0;

	//************************************
	// Method:    StartMatrixPatrol                          开始轮巡
	// Returns:   bool                                       操作成功与否
	// Parameter: char * pMatrixPlan                         预案名
	// Parameter: char * pCameraGroup                        镜头组
	// Parameter: int nPatrolTime                            轮巡时间
	// Parameter: char * pPosName                            预置位名称
	//************************************
	virtual bool StartMatrixPatrol(char * pMatrixPlan, char * pCameraGroup, int nPatrolTime);

	//************************************
	// Method:    StopMatrixPatrol                          停止轮巡
	// Returns:   bool                                      操作成功与否
	// Parameter: char * pMatrixPlan                        预案名
	//************************************
	virtual bool StopMatrixPatrol(char * pMatrixPlan);

};
