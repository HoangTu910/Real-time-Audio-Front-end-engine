#ifndef WAV_FILE_MGR_HPP
#define WAV_FILE_MGR_HPP

#include "utils.hpp"
#include <cstdio>
#include <cstdint>

/**
 * WAV File Manager
 *
 * Reads WAV file frame-by-frame (non-overlapping).
 * Deinterleaves multi-channel audio into separate per-channel buffers (max 4).
 * Writes processed frames back with re-interleaving (no OLA — OLA is in DSP modules).
 *
 * Usage:
 *   WavFileMgr wav;
 *   wav.bOpenRead("input.wav", frameSize);
 *   while (wav.bHasNextFrame()) {
 *       sample_t **ch = wav.ppReadFrame();   // ch[0], ch[1], ...
 *       for (u16 c = 0; c < wav.uGetChannels(); c++)
 *           process(ch[c], frameSize);       // per-channel DSP
 *       wav.vWriteFrame(ch, frameSize);
 *   }
 *   wav.vCloseWrite();
 */
class WavFileMgr {
public:
    WavFileMgr();
    ~WavFileMgr();

    /* ── Constants ── */
    static const u16 MAX_CHANNELS = 4;

    /* ── Read API ── */

    /** Open WAV file for reading, sets frame size (no overlap, hop = frameSize) */
    bool bOpenRead(const char *filename, u16 frameSize);

    /** Read next frame (non-overlapping), deinterleaved into per-channel buffers.
     *  Returns array of channel pointers: ppBuf[0]=ch0, ppBuf[1]=ch1, ...
     *  Each call returns the next sequential frameSize samples. */
    sample_t** ppReadFrame();

    /** Check if more frames available */
    bool bHasNextFrame() const;

    /** Get total number of frames */
    u32 uGetTotalFrames() const;

    /** Get file info */
    u32 uGetSampleRate() const    { return m_sampleRate; }
    u16 uGetChannels() const      { return m_channels; }
    u16 uGetFrameSize() const     { return m_frameSize; }
    u16 uGetBitsPerSample() const { return m_bitsPerSample; }

    /** Close read file */
    void vCloseRead();

    /* ── Write API ── */

    /** Open WAV file for writing. Uses same format as input. */
    bool bOpenWrite(const char *filename);

    /** Write processed per-channel frames. Interleaves and writes directly.
     *  ppChannels[ch][0..frameSize-1] for each channel. */
    void vWriteFrame(sample_t **ppChannels, u16 frameSize);

    /** Finalize and close output WAV file (writes header) */
    void vCloseWrite();

    /* ── Format conversion helpers ── */

    /** Convert raw bytes to sample_t based on bitsPerSample */
    sample_t sConvertToSample(const u8 *data) const;

    /** Convert sample_t to raw bytes based on bitsPerSample */
    void vConvertFromSample(sample_t s, u8 *data) const;

private:
    /* WAV header fields */
    static const uint16_t WAV_FORMAT_PCM = 1;

    #pragma pack(push, 1)
    struct WavHeader {
        char     riffId[4];       /* "RIFF" */
        uint32_t fileSize;        /* file size - 8 */
        char     waveId[4];       /* "WAVE" */
        char     fmtId[4];        /* "fmt " */
        uint32_t fmtSize;         /* 16 for PCM */
        uint16_t audioFormat;     /* 1 = PCM */
        uint16_t numChannels;
        uint32_t sampleRate;
        uint32_t byteRate;
        uint16_t blockAlign;
        uint16_t bitsPerSample;
        char     dataId[4];       /* "data" */
        uint32_t dataSize;
    };
    #pragma pack(pop)

    /* Read state */
    FILE        *m_pReadFile;
    sample_t    *m_apChannelBuf[MAX_CHANNELS];   /* per-channel frame buffers [N] */
    sample_t    *m_apChannelPtrs[MAX_CHANNELS];   /* pointers returned to caller */
    u16          m_frameSize;         /* N */
    u16          m_hopSize;           /* = frameSize (no overlap) */
    u32          m_sampleRate;
    u16          m_channels;
    u16          m_bitsPerSample;
    u32          m_dataSizeBytes;     /* total data bytes in file */
    u32          m_samplesRead;       /* samples read so far (per channel) */
    u32          m_totalSamples;      /* total samples per channel */
    bool         m_bReadOpen;

    /* Write state */
    FILE        *m_pWriteFile;
    sample_t    *m_apOutputBuf[MAX_CHANNELS];    /* per-channel output buffers */
    u32          m_samplesWritten;    /* samples written so far */
    u32          m_outputSizeBytes;   /* total output data bytes */
    bool         m_bWriteOpen;

    /* Helpers */
    void vInitReadBuffer();
    void vCleanup();
};

#endif /* WAV_FILE_MGR_HPP */
