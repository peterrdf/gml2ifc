#pragma once

#include <vector>
#include <iostream>
#include <sstream>
#include <random>
#include <climits>
#include <algorithm>
#include <functional>
#include <string>

#ifdef _WINDOWS
#include <rpc.h>
#pragma comment(lib, "rpcrt4.lib")
#endif

#ifdef __EMSCRIPTEN__
static unsigned char random_char()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);

    return static_cast<unsigned char>(dis(gen));
}

static std::string generate_hex(const unsigned int len)
{
    std::stringstream ss;
    for (unsigned int i = 0; i < len; i++)
    {
        auto rc = random_char();
        std::stringstream hexstream;
        hexstream << std::hex << int(rc);
        auto hex = hexstream.str();
        ss << (hex.length() < 2 ? '0' + hex : hex);
    }

    return ss.str();
}

typedef struct _GUID {
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[8];
} GUID;

typedef GUID UUID;
#endif // __EMSCRIPTEN__

// ************************************************************************************************
static const std::string base64_chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_$";

// ************************************************************************************************
class _guid
{
    typedef unsigned char BYTE;

public: // Methods

#ifdef _WINDOWS
    static std::string create()
    {
        UUID uuid;

        BYTE* szUUID = nullptr;
        if ((UuidCreate(&uuid) == RPC_S_OK) && 
            (UuidToStringA(&uuid, &szUUID) == RPC_S_OK))
        {
            std::string strUUID((char*)szUUID);

            RpcStringFreeA(&szUUID);

            return strUUID;
        }        

        return "";
    }
#endif

    // GlobalId
    static std::string createGlobalId()
    {
        UUID uuid;
#ifdef _WINDOWS
        BYTE* szUUID = nullptr;
        if ((UuidCreate(&uuid) == RPC_S_OK) &&
            (UuidToStringA(&uuid, &szUUID) == RPC_S_OK))
        {
            RpcStringFreeA(&szUUID);

            char szGlobalId[23];
            if (createGlobalIdFromGuid(&uuid, szGlobalId))
            {
                return szGlobalId;
            }
        }
#else
#ifdef __EMSCRIPTEN__
        char* szEnd = nullptr;

        std::string strData1 = "0x" + generate_hex(4);
        uuid.Data1 = std::strtoul(strData1.c_str(), &szEnd, 0);
        if (szEnd == nullptr)
        {
            return "";
        }

        std::string strData2 = "0x" + generate_hex(2);
        uuid.Data2 = (unsigned short)std::strtoul(strData2.c_str(), &szEnd, 0);
        if (szEnd == nullptr)
        {
            return "";
        }

        std::string strData3 = "0x" + generate_hex(2);
        uuid.Data3 = (unsigned short)std::strtoul(strData3.c_str(), &szEnd, 0);
        if (szEnd == nullptr)
        {
            return "";
        }

        std::string strData4 = generate_hex(8);
        for (size_t iChar = 0; iChar < strData4.size(); iChar++)
        {
            uuid.Data4[iChar] = (unsigned char)strData4.at(iChar);
        }

        char szGlobalId[23];
        if (createGlobalIdFromGuid(&uuid, szGlobalId))
        {
            return szGlobalId;
        }
#endif
#endif
        return "";
    }

private: // Methods

    // IFC base64 encoding
    static const bool createGlobalIdFromGuid(const GUID* pGuid, char* szGlobalId)
    {
        szGlobalId[0] = '\0';

        //
        // Creation of six 32 Bit integers from the components of the GUID structure
        //
        unsigned long arComponents[6];
        arComponents[0] = (unsigned long)(pGuid->Data1 / 16777216);                                                 // 16. byte  (pGuid->Data1 / 16777216) is the same as (pGuid->Data1 >> 24)
        arComponents[1] = (unsigned long)(pGuid->Data1 % 16777216);                                                 // 15-13. bytes (pGuid->Data1 % 16777216) is the same as (pGuid->Data1 & 0xFFFFFF)
        arComponents[2] = (unsigned long)(pGuid->Data2 * 256 + pGuid->Data3 / 256);                                 // 12-10. bytes
        arComponents[3] = (unsigned long)((pGuid->Data3 % 256) * 65536 + pGuid->Data4[0] * 256 + pGuid->Data4[1]);  // 09-07. bytes
        arComponents[4] = (unsigned long)(pGuid->Data4[2] * 65536 + pGuid->Data4[3] * 256 + pGuid->Data4[4]);       // 06-04. bytes
        arComponents[5] = (unsigned long)(pGuid->Data4[5] * 65536 + pGuid->Data4[6] * 256 + pGuid->Data4[7]);       // 03-01. bytes

        //
        // Conversion of the numbers into a system using a base of 64
        //        
        int_t iStep = 3;
        char szCodes[6][5];
        for (int_t i = 0; i < 6; i++) 
        {
            if (!number2IFCBase64(arComponents[i], szCodes[i], iStep))
            {
                return false;
            }

            strcat(szGlobalId, szCodes[i]);

            iStep = 5;
        }

        return true;
    }

    // IFC base64 encoding
    static bool number2IFCBase64(unsigned long lNumber, char* szCode, int_t iLength)
    {
        assert(szCode);

        if (iLength > 5)
        {
            return false;
        }

        char szResult[5];
        int_t iDigitsCount = iLength - 1;
        for (int_t iDigit = 0; iDigit < iDigitsCount; iDigit++)
        {
            szResult[iDigitsCount - iDigit - 1] = base64_chars[lNumber % 64];

            lNumber /= 64;
        }

        szResult[iLength - 1] = '\0';

        if (lNumber != 0)
        {
            return false;
        }           

        strcpy(szCode, szResult);

        return true;
    }
};

