#ifndef RTAFE_ERRORS_CODE_HPP
#define RTAFE_ERRORS_CODE_HPP

typedef enum RtafeError {
    kOk = 0,
    kErrorInvalidParam,
    kErrorFileIO,
    kErrorUnsupportedFormat,
    kErrorMemoryAllocation,
    kErrorProcessing,
    kErrorInitAudioInputFailed,
    kErrorInitAudioOutputFailed
} RtafeError;

#endif /* RTAFE_ERRORS_CODE_HPP */   