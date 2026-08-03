#ifndef HTSP_ERRORS_CODE_HPP
#define HTSP_ERRORS_CODE_HPP

typedef enum HtspErrRet {
    kOk = 0,
    kErrorInvalidParam,
    kErrorFileIO,
    kErrorUnsupportedFormat,
    kErrorMemoryAllocation,
    kErrorProcessing,
    kErrorInitAudioInputFailed,
    kErrorInitAudioOutputFailed,
    kErrorInvalidChannelCount
} HtspErrRet;

#endif /* HTSP_ERRORS_CODE_HPP */   