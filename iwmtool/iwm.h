#pragma once

#include <string>
#include "iw3/iwm_encdec.h"

class IWM
{
public:
    enum class EncState {
        Enc, // iwm0 - CoD4 vanilla
        Dec  // ice0 - CoD4X
    };
    enum class Error {
        NoError,
        OpenFileForRead,
        OpenFileForWrite,
        NoIWMFileRead,
        NoCDKeyProvided,
        InvalidCDKeyProvided,
        InvalidFileSize,
        InvalidFileHeader,
        InvalidIWMHash,
        InvalidStatsChecksum,
        InvalidStatIndex,
        InvalidStatValue
    };

    IWM();
    IWM(const IWM&) = delete;
    IWM& operator=(const IWM&) = delete;

    bool SetCDKey(const std::string& cdKey);
    EncState GetEncryptionState();

    bool SetStat(int index, int value);
    bool GetStat(int index, int& value);

    bool ReadFile(const std::string& fileName);
    bool WriteFile(const std::string& fileName, EncState encState);

    Error GetLastError() const;
    const char* GetLastErrorString() const;
    static const char* GetErrorString(Error error);

private:
    bool Parse();
    bool PrepareForWrite(iwm_t& iwm, EncState encState);

    iwm_t _iwm;
    bool _fileRead;
    std::string _cdKey;
    EncState _encState;
    Error _error;
};
