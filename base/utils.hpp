/* 
 * File:   utils.hpp
 * Author: asalazar
 *
 * Created on July 30, 2015, 11:29 AM
 */

#ifndef UTILS_HPP
#define	UTILS_HPP

static NatureOfAddress getNatureOfAddress(const unsigned char &type)
{
    unsigned char ton = type & 0x70;
    if (ton == 16)
    {
        return msisdnInternationalNumber;
    }
    else if (ton == 32)
    {
        return msisdnNationalSignificantNumber;
    }
    else if (ton == 64)
    {
        return msisdnSubscriberNumber;
    }
    else
    {
        return natureOfAddressNotPresent;
    }
}

static char getHexO(const char ch)
{
    if (isxdigit(ch))
    {
        if (ch > 0x2f && ch < 0x3a)
        {
            return ch - 0x30;
        }
        else
        {
            if ((ch & 0xf) == 1)
            {
                return 0xa;
            }
            else if ((ch & 0xf) == 2)
            {
                return 0xb;
            }
            else if ((ch & 0xf) == 3)
            {
                return 0xc;
            }
            else if ((ch & 0xf) == 4)
            {
                return 0xd;
            }
            else if ((ch & 0xf) == 5)
            {
                return 0xe;
            }
            else if ((ch & 0xf) == 6)
            {
                return 0xf;
            }
        }
    }
    return -1;
}

static void getOctet(const std::string &str, unsigned char *o, int &len)
{
    len = str.length();
    for (unsigned int strIndex = 0; strIndex < str.length(); strIndex++)
    {
        char tmpO = getHexO(str.at(strIndex));
        if (tmpO != -1) o[strIndex] = tmpO;
    }
}

/**
 * Returns the semi octet.
 * @param &str const std::string
 * @param *so unsigned char
 * @param &len int
 */
static void getSemiOctet(const std::string &str, unsigned char *so, int &len)
{
    len = str.length() >> 1;
    if ((str.length() & 1) == 1)
    {
        len++;
    }
    
    for (unsigned char soIndex = 0; soIndex < len; soIndex++)
    {
        char tmpO = getHexO(str.at(soIndex << 1));
        if (tmpO != -1)
        {
            so[soIndex] = tmpO;
        }
        
        if (((soIndex << 1) + 1) < (int) str.length())
        {
            tmpO = getHexO(str.at((soIndex << 1) + 1));
            if (tmpO != -1)
            {
                so[soIndex] |= tmpO << 4;
            }
        }
        else
        {
            so[soIndex] |= 0xf0;
        }
    }
}

typedef enum
{
    /* states for the server */
    waitingForOpenReq,
    openReqReceived,

    /* states for the client */
    waitingForOpenRsp,
    waitingForInvokeRsp,
    waitingForClose
} MyState;

/**
     * Returns the address.
     * @param &str const std::string
     * @param *addr unsigned char
     * @param &len int
     */
    static void getAddress(const std::string &str, const unsigned char &toa, unsigned char *addr, int &len) {
        addr[0]=toa;
        getSemiOctet(str, &addr[1], len);
        ++len;
    }

#endif	/* UTILS_HPP */
