#include "WavFileMgr.hpp"
#include <cstring>

/* ──────────────────────────────────────────────────────────────────────────── */
/*  Constructor / Destructor                                                    */
/* ──────────────────────────────────────────────────────────────────────────── */

WavFileMgr::WavFileMgr()
    : m_pReadFile(nullptr)
    , m_frameSize(0)
    , m_hopSize(0)
    , m_sampleRate(0)
    , m_channels(0)
    , m_bitsPerSample(0)
    , m_dataSizeBytes(0)
    , m_samplesRead(0)
    , m_totalSamples(0)
    , m_bReadOpen(false)
    , m_pWriteFile(nullptr)
    , m_samplesWritten(0)
    , m_outputSizeBytes(0)
    , m_bWriteOpen(false)
{
    for (u16 i = 0; i < MAX_CHANNELS; i++) {
        m_apChannelBuf[i]  = nullptr;
        m_apOverlapBuf[i]  = nullptr;
        m_apChannelPtrs[i] = nullptr;
        m_apOutputBuf[i]   = nullptr;
    }
}

WavFileMgr::~WavFileMgr()
{
    vCleanup();
}

void WavFileMgr::vCleanup()
{
    vCloseRead();
    vCloseWrite();

    for (u16 i = 0; i < MAX_CHANNELS; i++) {
        delete[] m_apChannelBuf[i];
        m_apChannelBuf[i] = nullptr;
        delete[] m_apOverlapBuf[i];
        m_apOverlapBuf[i] = nullptr;
        delete[] m_apOutputBuf[i];
        m_apOutputBuf[i] = nullptr;
    }
}

/* ──────────────────────────────────────────────────────────────────────────── */
/*  Read API                                                                    */
/* ──────────────────────────────────────────────────────────────────────────── */

bool WavFileMgr::bOpenRead(const char *filename, u16 frameSize)
{
    if (m_bReadOpen) vCloseRead();

    m_pReadFile = fopen(filename, "rb");
    if (!m_pReadFile) {
        printf("Error: cannot open WAV file '%s'\n", filename);
        return false;
    }

    /* Read header */
    WavHeader hdr;
    if (fread(&hdr, sizeof(WavHeader), 1, m_pReadFile) != 1) {
        printf("Error: cannot read WAV header\n");
        fclose(m_pReadFile);
        m_pReadFile = nullptr;
        return false;
    }

    /* Validate */
    if (memcmp(hdr.riffId, "RIFF", 4) != 0 ||
        memcmp(hdr.waveId, "WAVE", 4) != 0 ||
        memcmp(hdr.fmtId,  "fmt ", 4) != 0 ||
        memcmp(hdr.dataId, "data", 4) != 0) {
        printf("Error: invalid WAV file format\n");
        fclose(m_pReadFile);
        m_pReadFile = nullptr;
        return false;
    }

    if (hdr.audioFormat != WAV_FORMAT_PCM) {
        printf("Error: only PCM WAV format supported\n");
        fclose(m_pReadFile);
        m_pReadFile = nullptr;
        return false;
    }

    if (hdr.bitsPerSample != 8 && hdr.bitsPerSample != 16 &&
        hdr.bitsPerSample != 24 && hdr.bitsPerSample != 32) {
        printf("Error: unsupported bitsPerSample=%d (supported: 8, 16, 24, 32)\n",
               hdr.bitsPerSample);
        fclose(m_pReadFile);
        m_pReadFile = nullptr;
        return false;
    }

    m_sampleRate    = hdr.sampleRate;
    m_channels      = hdr.numChannels;
    m_bitsPerSample = hdr.bitsPerSample;
    m_dataSizeBytes = hdr.dataSize;
    m_totalSamples  = m_dataSizeBytes / (m_channels * (m_bitsPerSample / 8));

    /* Frame setup: N = frameSize, hop = N/2 */
    m_frameSize = frameSize;
    m_hopSize   = frameSize / 2;

    /* Allocate buffers */
    vInitReadBuffer();

    m_samplesRead = 0;
    m_bReadOpen   = true;

    printf("WAV: %lu Hz, %d ch, %lu samples, frame=%d, hop=%d\n",
           (unsigned long)m_sampleRate, m_channels,
           (unsigned long)m_totalSamples, m_frameSize, m_hopSize);

    return true;
}

void WavFileMgr::vInitReadBuffer()
{
    for (u16 ch = 0; ch < MAX_CHANNELS; ch++) {
        delete[] m_apChannelBuf[ch];
        delete[] m_apOverlapBuf[ch];

        if (ch < m_channels) {
            m_apChannelBuf[ch] = new sample_t[m_frameSize];
            m_apOverlapBuf[ch] = new sample_t[m_hopSize];
            memset(m_apChannelBuf[ch], 0, sizeof(sample_t) * m_frameSize);
            memset(m_apOverlapBuf[ch], 0, sizeof(sample_t) * m_hopSize);
            m_apChannelPtrs[ch] = m_apChannelBuf[ch];
        } else {
            m_apChannelBuf[ch]  = nullptr;
            m_apOverlapBuf[ch]  = nullptr;
            m_apChannelPtrs[ch] = nullptr;
        }
    }
}

sample_t** WavFileMgr::ppReadFrame()
{
    if (!m_bReadOpen || !bHasNextFrame()) return nullptr;

    /*
     * 50% overlap read with multi-channel deinterleave:
     *
     *   Read interleaved: L0 R0 L1 R1 ... from file
     *   Deinterleave into per-channel buffers: ch0=[L0 L1 ...], ch1=[R0 R1 ...]
     *   Each channel buffer maintains its own overlap for OLA.
     */

    u16 bytesPerSample = m_bitsPerSample / 8;
    u16 frameBytes = m_channels * bytesPerSample;  /* bytes per interleaved sample */

    if (m_samplesRead == 0) {
        /* First frame: read full N time-domain samples */
        u32 toRead = m_frameSize;
        if (m_samplesRead + toRead > m_totalSamples) {
            toRead = m_totalSamples - m_samplesRead;
        }

        u32 rawBytes = m_frameSize * frameBytes;
        u8 *raw = new u8[rawBytes];
        memset(raw, 0, rawBytes);
        fread(raw, frameBytes, toRead, m_pReadFile);

        /* Deinterleave into per-channel buffers */
        for (u16 i = 0; i < m_frameSize; i++) {
            for (u16 ch = 0; ch < m_channels; ch++) {
                m_apChannelBuf[ch][i] = sConvertToSample(
                    raw + i * frameBytes + ch * bytesPerSample);
            }
        }

        delete[] raw;
        m_samplesRead += toRead;
    } else {
        /* Subsequent frames: shift left by N/2, read N/2 new */

        /* Shift each channel: copy right half to left half */
        for (u16 ch = 0; ch < m_channels; ch++) {
            memmove(m_apChannelBuf[ch], m_apChannelBuf[ch] + m_hopSize,
                    sizeof(sample_t) * m_hopSize);
        }

        /* Read new hopSize time-domain samples */
        u32 toRead = m_hopSize;
        if (m_samplesRead + toRead > m_totalSamples) {
            toRead = m_totalSamples - m_samplesRead;
        }

        u32 rawBytes = m_hopSize * frameBytes;
        u8 *raw = new u8[rawBytes];
        memset(raw, 0, rawBytes);
        fread(raw, frameBytes, toRead, m_pReadFile);

        /* Deinterleave into right half of per-channel buffers */
        for (u16 i = 0; i < m_hopSize; i++) {
            for (u16 ch = 0; ch < m_channels; ch++) {
                m_apChannelBuf[ch][m_hopSize + i] = sConvertToSample(
                    raw + i * frameBytes + ch * bytesPerSample);
            }
        }

        /* Zero-pad if less than hopSize samples available */
        for (u32 i = toRead; i < m_hopSize; i++) {
            for (u16 ch = 0; ch < m_channels; ch++) {
                m_apChannelBuf[ch][m_hopSize + i] = 0;
            }
        }

        delete[] raw;
        m_samplesRead += toRead;
    }

    return m_apChannelPtrs;
}

bool WavFileMgr::bHasNextFrame() const
{
    return m_bReadOpen && (m_samplesRead < m_totalSamples);
}

u32 WavFileMgr::uGetTotalFrames() const
{
    if (m_hopSize == 0) return 0;
    return (m_totalSamples + m_hopSize - 1) / m_hopSize;
}

void WavFileMgr::vCloseRead()
{
    if (m_pReadFile) {
        fclose(m_pReadFile);
        m_pReadFile = nullptr;
    }
    m_bReadOpen = false;
}

/* ──────────────────────────────────────────────────────────────────────────── */
/*  Write API                                                                   */
/* ──────────────────────────────────────────────────────────────────────────── */

bool WavFileMgr::bOpenWrite(const char *filename)
{
    if (m_bWriteOpen) vCloseWrite();

    m_pWriteFile = fopen(filename, "wb");
    if (!m_pWriteFile) {
        printf("Error: cannot create WAV file '%s'\n", filename);
        return false;
    }

    /* Allocate per-channel output accumulators */
    u32 estOutputSamples = m_totalSamples + m_frameSize;  /* extra for overlap */
    for (u16 ch = 0; ch < m_channels; ch++) {
        delete[] m_apOutputBuf[ch];
        m_apOutputBuf[ch] = new sample_t[estOutputSamples];
        memset(m_apOutputBuf[ch], 0, sizeof(sample_t) * estOutputSamples);
    }

    m_samplesWritten  = 0;
    m_outputSizeBytes = 0;
    m_bWriteOpen      = true;

    return true;
}

void WavFileMgr::vWriteFrame(sample_t **ppChannels, u16 frameSize)
{
    if (!m_bWriteOpen || !ppChannels) return;
    if (frameSize != m_frameSize) return;

    /*
     * Overlap-Add (OLA) per channel with 50% overlap.
     * Each channel accumulates independently; interleave on close.
     */

    u32 offset = m_samplesWritten;

    for (u16 ch = 0; ch < m_channels; ch++) {
        if (!ppChannels[ch]) continue;
        for (u16 i = 0; i < frameSize; i++) {
            m_apOutputBuf[ch][offset + i] += ppChannels[ch][i];
        }
    }

    m_samplesWritten += m_hopSize;
}

void WavFileMgr::vCloseWrite()
{
    if (!m_bWriteOpen || !m_pWriteFile) {
        m_bWriteOpen = false;
        return;
    }

    /* Write WAV header */
    u32 totalOut = m_samplesWritten + m_frameSize;
    u16 bytesPerSample = m_bitsPerSample / 8;
    u32 dataSize = totalOut * m_channels * bytesPerSample;
    WavHeader hdr;
    memcpy(hdr.riffId, "RIFF", 4);
    hdr.fileSize      = sizeof(WavHeader) - 8 + dataSize;
    memcpy(hdr.waveId, "WAVE", 4);
    memcpy(hdr.fmtId,  "fmt ", 4);
    hdr.fmtSize       = 16;
    hdr.audioFormat   = WAV_FORMAT_PCM;
    hdr.numChannels   = m_channels;
    hdr.sampleRate    = m_sampleRate;
    hdr.byteRate      = m_sampleRate * m_channels * bytesPerSample;
    hdr.blockAlign    = m_channels * bytesPerSample;
    hdr.bitsPerSample = m_bitsPerSample;
    memcpy(hdr.dataId, "data", 4);
    hdr.dataSize      = dataSize;

    fwrite(&hdr, sizeof(WavHeader), 1, m_pWriteFile);

    /* Write samples in original bit depth, interleaved from per-channel buffers */
    u16 frameBytes = m_channels * bytesPerSample;
    u8 *raw = new u8[totalOut * frameBytes];
    for (u32 i = 0; i < totalOut; i++) {
        for (u16 ch = 0; ch < m_channels; ch++) {
            vConvertFromSample(m_apOutputBuf[ch][i],
                               raw + i * frameBytes + ch * bytesPerSample);
        }
    }
    fwrite(raw, frameBytes, totalOut, m_pWriteFile);
    delete[] raw;

    printf("WAV output: %lu samples (%d-bit) written to file\n",
           (unsigned long)totalOut, m_bitsPerSample);

    fclose(m_pWriteFile);
    m_pWriteFile = nullptr;
    m_bWriteOpen = false;
}

sample_t WavFileMgr::sConvertToSample(const u8 *data) const
{
    switch (m_bitsPerSample) {
    case 8: {
        /* 8-bit unsigned PCM: center at 128 */
        s16 val = (s16)data[0] - 128;
        return (sample_t)(val << 8);
    }
    case 16: {
        /* 16-bit signed PCM (little-endian) */
        s16 val = (s16)(data[0] | (data[1] << 8));
        return (sample_t)val;
    }
    case 24: {
        /* 24-bit signed PCM (little-endian), sign-extend, convert to 16-bit */
        s32 val = (s32)(data[0] | (data[1] << 8) | (data[2] << 16));
        if (val & 0x800000) val |= 0xFF000000;  /* sign extend */
        return (sample_t)(val >> 8);
    }
    case 32: {
        /* 32-bit signed PCM (little-endian), convert to 16-bit */
        s32 val = (s32)(data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24));
        return (sample_t)(val >> 16);
    }
    default:
        return 0;
    }
}

void WavFileMgr::vConvertFromSample(sample_t s, u8 *data) const
{
    /* Clip to s16 range */
    s32 val = (s32)s;
    if (val > 32767) val = 32767;
    if (val < -32768) val = -32768;

    switch (m_bitsPerSample) {
    case 8: {
        /* 8-bit unsigned PCM */
        u8 out = (u8)((s16)val + 128);
        data[0] = out;
        break;
    }
    case 16: {
        /* 16-bit signed PCM (little-endian) */
        s16 out = (s16)val;
        data[0] = (u8)(out & 0xFF);
        data[1] = (u8)((out >> 8) & 0xFF);
        break;
    }
    case 24: {
        /* 24-bit signed PCM (little-endian) */
        s32 out = (s32)val << 8;
        data[0] = (u8)(out & 0xFF);
        data[1] = (u8)((out >> 8) & 0xFF);
        data[2] = (u8)((out >> 16) & 0xFF);
        break;
    }
    case 32: {
        /* 32-bit signed PCM (little-endian) */
        s32 out = (s32)val << 16;
        data[0] = (u8)(out & 0xFF);
        data[1] = (u8)((out >> 8) & 0xFF);
        data[2] = (u8)((out >> 16) & 0xFF);
        data[3] = (u8)((out >> 24) & 0xFF);
        break;
    }
    default:
        break;
    }
}
