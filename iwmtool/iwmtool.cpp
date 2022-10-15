#include <Windows.h>
#include <iostream>
#include <cxxopts/include/cxxopts.hpp>

#include "iwm.h"


bool ReadCDKeyFromRegistry(char *cdKey, unsigned int len)
{
    HKEY hKey;
    LSTATUS err;
    DWORD bufLen = len-1;

    err = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Activision\\Call of Duty 4", 0, KEY_READ | KEY_WOW64_32KEY, &hKey);
    if (err != ERROR_SUCCESS) {
        return 0;
    }

    err = RegQueryValueExA(hKey, "codkey", NULL, NULL, (BYTE *)cdKey, &bufLen);
    RegCloseKey(hKey);
    if (err != ERROR_SUCCESS) {
        return 0;
    }

    cdKey[bufLen] = 0;
    return 1;
}

int main(int argc, const char **argv)
{
    cxxopts::Options options("iwmtool", "Tool for manipulating mpdata files for CoD4");
    options.add_options("General")
        ("m,mode",   "Valid args: encrypt, decrypt, stats", cxxopts::value<std::string>())
        ("i,input",  "Input mpdata file", cxxopts::value<std::string>()->default_value("mpdata"), "file")
        ("o,output", "Output mpdata file. If omitted, any changes will be made to input file", cxxopts::value<std::string>()->default_value(""), "file")
        ("k,key",    "Required for (enc/dec)ryption. If omitted, iwmtool will try to fetch the CD-key from registry", cxxopts::value<std::string>()->default_value(""), "cd-key");
    options.add_options("Stats")
        ("n,index",  "Index of the stat to be read/written (0-3497)", cxxopts::value<int>(), "n")
        ("s,set",    "Value to be written at index n", cxxopts::value<int>(), "value");

    if (argc < 2) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    try {
        IWM iwm;
        IWM::EncState encState;
        bool writeNeeded = true;
        auto args = options.parse(argc, argv);

        std::string mode = args["mode"].as<std::string>();
        std::string inFile = args["input"].as<std::string>();
        std::string outFile = args["output"].as<std::string>();
        std::string cdKey = args["key"].as<std::string>();


        if (outFile.empty()) {
            outFile = inFile;
            std::cout << "Warn:  No output file specified, defaulting to: \"" << inFile << "\"" << std::endl;
        }

        if (cdKey.empty()) {
            char buffer[32];

            std::cout << "Warn:  No CD-key specified, defaulting to: \"";
            if (ReadCDKeyFromRegistry(buffer, sizeof(buffer))) {
                cdKey = buffer;
                std::cout << cdKey << "\" (registry)" << std::endl;
            }
            else {
                std::cout << "\" (empty key)" << std::endl;
            }
        }

        if (!iwm.SetCDKey(cdKey)) {
            std::cout << "Error: CD-key \"" << cdKey << "\" appears to be invalid, err: " << iwm.GetLastErrorString() << std::endl;
            return 1;
        }

        if (!iwm.ReadFile(inFile)) {
            std::cout << "Error: Input stats file \"" << inFile << "\" appears to be invalid, err: " << iwm.GetLastErrorString() << std::endl;
            return 1;
        }

        if (mode == "encrypt") {
            encState = IWM::EncState::Enc;
        }
        else if (mode == "decrypt") {
            encState = IWM::EncState::Dec;
        }
        else if (mode == "stats") {
            int index = args["index"].as<int>();
            int value;

            encState = iwm.GetEncryptionState();

            if (args.count("set")) {
                value = args["set"].as<int>();

                if (!iwm.SetStat(index, value)) {
                    std::cout << "Error: Can't set stat index=" << index << " to value=" << value << ", err: " << iwm.GetLastErrorString() << std::endl;
                    return 1;
                }
                std::cout << "Info:  Written index=" << index << ", value=" << value << std::endl;
            }
            else {
                writeNeeded = false;

                if (!iwm.GetStat(index, value)) {
                    std::cout << "Error: Can't read stat index=" << index << ", err: " << iwm.GetLastErrorString() << std::endl;
                    return 1;
                }
                std::cout << "Info:  Read index=" << index << ", value=" << value << std::endl;
            }
        }
        else {
            std::cout << "Error: Unknown mode specified: " << mode << std::endl;
            return 1;
        }

        if (writeNeeded) {
            if (!iwm.WriteFile(outFile, encState)) {
                std::cout << "Error: Output stats file \"" << outFile << "\" can't be written, err: " << iwm.GetLastErrorString() << std::endl;
                return 1;
            }

            std::cout << "Info:  Output file written: \"" << outFile << "\"";
            if (encState == IWM::EncState::Enc) {
                std::cout << " (encrypted)" << std::endl;
            }
            else {
                std::cout << " (decrypted)" << std::endl;
            }
        }
    }
    catch (const cxxopts::exceptions::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    return 0;
}
