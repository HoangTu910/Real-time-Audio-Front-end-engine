#ifndef RTAFE_ERRORS_CODE_HPP
#define RTAFE_ERRORS_CODE_HPP

typedef enum RtafeErrRet {
    kOk = 0,
    kErrorInvalidParam,
    kErrorFileIO,
    kErrorUnsupportedFormat,
    kErrorMemoryAllocation,
    kErrorProcessing,
    kErrorInitAudioInputFailed,
    kErrorInitAudioOutputFailed
} RtafeErrRet;

#endif /* RTAFE_ERRORS_CODE_HPP */   