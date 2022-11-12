#include <fstream>
#include <cctype>
#include <algorithm>
#include <cstring>

#include "iwm.h"
#include "iw3/md4.h"

IWM::IWM() : _iwm(), _fileRead(false), _cdKey(), _encState(EncState::Enc), _error(Error::NoError)
{
}

bool IWM::SetCDKey(const std::string& cdKey)
{
    _cdKey = cdKey;
    _cdKey.erase(std::remove_if(_cdKey.begin(), _cdKey.end(), [](char c) { return !isalnum(c); }), _cdKey.end());
    std::transform(_cdKey.begin(), _cdKey.end(), _cdKey.begin(), toupper);

    if (!_cdKey.empty() && _cdKey.length() < 16) {
        _cdKey.clear();
        _error = Error::InvalidCDKeyProvided;
        return false;
    }

    return true;
}

IWM::EncState IWM::GetEncryptionState()
{
    return _encState;
}

bool IWM::SetStat(int index, int value)
{
    if (!_fileRead) {
        _error = Error::NoIWMFileRead;
        return false;
    }

    if (index < 2000) {
        if (value < 0 || value > 255) {
            _error = Error::InvalidStatValue;
            return false;
        }

        _iwm.stats.data.byte[index] = value;
        return true;
    }

    index -= 2000;
    if (index < 1498) {
        _iwm.stats.data.dword[index] = value;
        return true;
    }

    _error = Error::InvalidStatIndex;
    return false;
}

bool IWM::GetStat(int index, int& value)
{
    if (!_fileRead) {
        _error = Error::NoIWMFileRead;
        return false;
    }

    if (index < 2000) {
        value = _iwm.stats.data.byte[index];
        return 1;
    }

    index -= 2000;
    if (index < 1498) {
        value = _iwm.stats.data.dword[index];
        return 1;
    }

    _error = Error::InvalidStatIndex;
    return 0;
}

bool IWM::ReadFile(const std::string& fileName)
{
    std::ifstream fileStream(fileName, std::ios::binary);

    if (!fileStream.is_open()) {
        _error = Error::OpenFileForRead;
        return false;
    }

    fileStream.read((char *)&_iwm, sizeof(iwm_t));
    if (fileStream.gcount() != sizeof(iwm_t)) {
        _error = Error::InvalidFileSize;
        return false;
    }

    return Parse();
}

bool IWM::WriteFile(const std::string& fileName, EncState encState)
{
    std::ofstream fileStream(fileName, std::ios::binary);
    iwm_t writtenIWM;

    if (!_fileRead) {
        _error = Error::NoIWMFileRead;
        return false;
    }

    if (!PrepareForWrite(writtenIWM, encState)) {
        return false;
    }

    if (!fileStream.is_open()) {
        _error = Error::OpenFileForWrite;
        return false;
    }

    fileStream.write((char *)&writtenIWM, sizeof(iwm_t));
    return true;
}

bool IWM::Parse()
{
    unsigned int mpDataChecksum;

    if (memcmp(_iwm.signature, "iwm0", 4) == 0) {
        _encState = EncState::Enc;

        if (_cdKey.empty()) {
            _error = Error::NoCDKeyProvided;
            return false;
        }

        if (!LiveStorage_DecryptIWMFile(&_iwm, _cdKey.c_str())) {
            _error = Error::InvalidIWMHash;
            return false;
        }
    }
    else if (memcmp(_iwm.signature, "ice0", 4) == 0) {
        _encState = EncState::Dec;
    }
    else {
        _error = Error::InvalidFileHeader;
        return false;
    }

    mpDataChecksum = Com_BlockChecksumKey32(&_iwm.stats.data, sizeof(playerData_t), 0);
    if (mpDataChecksum != _iwm.stats.checksum) {
        _error = Error::InvalidStatsChecksum;
        return false;
    }

    _fileRead = true;
    return true;
}

bool IWM::PrepareForWrite(iwm_t& writtenIWM, EncState encState)
{
    memcpy(&writtenIWM, &_iwm, sizeof(iwm_t));
    writtenIWM.stats.checksum = Com_BlockChecksumKey32(&writtenIWM.stats.data, sizeof(playerData_t), 0);

    if (encState == EncState::Enc) {
        if (_cdKey.empty()) {
            _error = Error::NoCDKeyProvided;
            return false;
        }
        LiveStorage_EncryptIWMFile(&writtenIWM, _cdKey.c_str());
    }
    else {
        memcpy(writtenIWM.signature, "ice0", 4);
    }

    return true;
}

IWM::Error IWM::GetLastError() const
{
    return _error;
}

const char* IWM::GetLastErrorString() const
{
    return GetErrorString(_error);
}

#define ERROR_CASE(err) case Error::err: return #err
const char* IWM::GetErrorString(Error error)
{
    switch (error) {
        ERROR_CASE(NoError);
        ERROR_CASE(OpenFileForRead);
        ERROR_CASE(OpenFileForWrite);
        ERROR_CASE(NoIWMFileRead);
        ERROR_CASE(NoCDKeyProvided);
        ERROR_CASE(InvalidCDKeyProvided);
        ERROR_CASE(InvalidFileSize);
        ERROR_CASE(InvalidFileHeader);
        ERROR_CASE(InvalidIWMHash);
        ERROR_CASE(InvalidStatsChecksum);
        ERROR_CASE(InvalidStatIndex);
        ERROR_CASE(InvalidStatValue);
    default:
        return "Unknown error";
    }
}
