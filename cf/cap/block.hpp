/*
 * block.hpp
 */
#ifndef BLOCK_HPP
#define	BLOCK_HPP

#include "dialog.hpp"
#include "../../base/appconn/primitiveie.hpp"
#include "../../base/cqueue.hpp"
#include <string>
#include <sstream>
#include <gmap.h>

#include "../../base/utils.hpp"

#define CL_SVC_CLASS_0 0

using namespace std;

/**
 * <p>The Address class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class Address
{
public:

    /* constants */
    static char TON;
    /* ton 3 bit 4, 5 and 6 */
    static char TON_UNKNOWN;
    static char TON_INTERNATIONAL;
    static char TON_NATIONAL;
    static char TON_NETWORK_SPECIFIC;
    static char TON_DEDICATED_ACCESS;

    static char NPI;
    /* npi 4 bit 0 to 3 */
    static char NPI_UNKNOWN;
    static char NPI_ISDN;
    static char NPI_DNP;
    static char NPI_TNP;
    static char NPI_NATIONAL;
    static char NPI_PRIVATE;

private:

    /* private variables */
    char ton;
    char npi;
    std::string addr;

public:

    /**
     * Creates a new instance of Address.
     * @param &ton const char
     * @param &npi const char
     * @param &addr const std::string
     */
    Address(const char &ton, const char &npi, const std::string &addr)
    {
        if (addr.length() > 0)
        {
            this->ton = ton;
            this->npi = npi;
            this->addr = addr;
        }
        else
        {
            this->ton = TON_UNKNOWN;
            this->npi = NPI_UNKNOWN;
            this->addr = "";
        }
    }

    /**
     * Creates a new instance of Address.
     * @param *addr char
     * @param &len const int
     */
    Address(char *addr, const int &len)
    {
        if (len > 0)
        {
            ton = addr[0] & TON;
            npi = addr[0] & NPI;
            this->addr = toSemiOctetString(addr + 1, len - 1);
        }
        else
        {
            ton = TON_UNKNOWN;
            npi = NPI_UNKNOWN;
            this->addr = "";
        }
    }

    virtual ~Address()
    {
    }

    /**
     * Returns the address.
     * @param *ch char
     * @param &len int
     */
    void getAddress(char *ch, int &len)
    {
        ch[0] = 0x80;
        ch[0] |= ton & TON;
        ch[0] |= npi & NPI;
        toSemiOctet(addr, ch + 1, len);
        len += 1;
    }
    // TODO
//        static void getAddress(const std::string &str, const unsigned char &toa, unsigned char *addr, int &len) {
//        addr[0]=toa;
//        getSemiOctet(str, &addr[1], len);
//        ++len;
//    }

    /**
     * Returns the ton.
     * @return char
     */
    char getTON()
    {
        return ton;
    }

    /**
     * Returns the npi.
     * @return char
     */
    char getNPI()
    {
        return npi;
    }

    /**
     * Returns the addr.
     * @return std::string
     */
    std::string getAddr()
    {
        return addr;
    }

    /**
     * Returns the semi octet.
     * @param &str const std::string
     * @param *so char
     * @param &len int
     */
    static void toSemiOctet(const std::string &s, char *so, int &len)
    {
        len = s.length() >> 1;
        if ((s.length() & 1) == 1) len++;
        for (int i = 0; i < len; i++)
        {
            char o = hexToByte(s.at(i << 1));
            if (o != -1) so[i] = o;
            if (((i << 1) + 1) < (int) s.length())
            {
                o = hexToByte(s.at((i << 1) + 1));
                if (o != -1) so[i] |= o << 4;
            }
            else so[i] |= 0xf0;
        }
    }

    /**
     * Return the string for a semi octet.
     * @param *so char
     * @param &len const int
     * @return std::string
     */
    static std::string toSemiOctetString(char *so, const int &len)
    {
        std::string str = "";
        for (int i = 0; i < len; i++)
        {
            int l = so[i] & 0xf;
            int h = (so[i] >> 4) & 0xf;
            if (l < 0xf) str += hexCh[l];
            if (h < 0xf) str += hexCh[h];
        }
        return str;
    }
};

char Address::TON = 0x70;

char Address::TON_UNKNOWN = 0x0;
char Address::TON_INTERNATIONAL = 0x10;
char Address::TON_NATIONAL = 0x20;
char Address::TON_NETWORK_SPECIFIC = 0x30;
char Address::TON_DEDICATED_ACCESS = 0x40;

char Address::NPI = 0xf;

char Address::NPI_UNKNOWN = 0x0;
char Address::NPI_ISDN = 0x1;
char Address::NPI_DNP = 0x3;
char Address::NPI_TNP = 0x4;
char Address::NPI_NATIONAL = 0x8;
char Address::NPI_PRIVATE = 0x9;

/**
 * <p>The Number class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class Number
{
public:

    /* constants */
    static char OEI;
    /* oei 1 bit 7 */
    static char OEI_EVEN;
    static char OEI_ODD;

    static char NAI;
    /* nai 7 bit 0 to 6 */
    static char NAI_SUBSCRIBER_NUMBER;
    static char NAI_UNKNOWN;
    static char NAI_NATIONAL;
    static char NAI_INTERNATIONAL;

    static char NPI;
    /* npi 3 bit 4 to 6 */
    static char NPI_ISDN;
    static char NPI_DNP;
    static char NPI_TNP;

private:

    /* private variables */
    char oei;
    char nai;
    char npi;
    std::string addr;

public:

    /**
     * Creates a new instance of Number.
     * @param &nai const char
     * @param &npi const char
     * @param &addr const std::string
     */
    Number(const char &nai, const char &npi, const std::string &addr)
    {
        if ((addr.length() & 1) == 0) oei = OEI_EVEN;
        else oei = OEI_ODD;
        this->nai = nai;
        this->npi = npi;
        this->addr = addr;
    }

    /**
     * Creates a new instance of Number.
     * @param *nbr char
     * @param &len const int
     */
    Number(char *nbr, const int &len)
    {
        if (len > 1)
        {
            oei = nbr[0] & OEI;
            nai = nbr[0] & NAI;
            npi = nbr[1] & NPI;
            addr = toSemiOctetString(oei, nbr + 2, len - 2);
        }
        else
        {
            oei = OEI_EVEN;
            nai = 0;
            npi = 0;
            addr = "";
        }
    }

    virtual ~Number()
    {
    }

protected:

    /**
     * Returns the number.
     * @param *ch char
     * @param &len int
     */
    void getNumber(char *ch, int &len)
    {
        ch[0] = 0;
        ch[0] |= oei & OEI;
        ch[0] |= nai & NAI;
        ch[1] = npi & NPI;
        toSemiOctet(addr, ch + 2, len);
        len += 2;
    }

public:

    /**
     * Returns the nai.
     * @return char
     */
    char getNAI()
    {
        return nai;
    }

    /**
     * Returns the npi.
     * @return char
     */
    char getNPI()
    {
        return npi;
    }

    /**
     * Returns the addr.
     * @return std::string
     */
    std::string getAddr()
    {
        return addr;
    }

    /**
     * Returns the semi octet.
     * @param &str const std::string
     * @param *so char
     * @param &len int
     */
    static void toSemiOctet(const std::string &s, char *so, int &len)
    {
        int slen = s.size();
        len = slen >> 1;
        if ((slen & 1) == 1) len++;
        int soi = 0;
        for (int si = 0; si < slen; si++)
        {
            char o = hexToByte(s.at(si));
            if (o != -1)
            {
                if ((si & 1) == 0) so[soi] = o;
                else
                {
                    so[soi] |= o << 4;
                    soi++;
                }
            }
        }
    }

    /**
     * Return the string for the semi octet.
     * @param &oei const char
     * @param *so char
     * @param &len const int
     * @return std::string
     */
    static std::string toSemiOctetString(const char &oei, char *so, const int &len)
    {
        std::string s = "";
        int tmpLen = len << 1;
        if ((oei & OEI) == OEI_ODD) tmpLen -= 1;
        int soi = 0;
        for (int si = 0; si < tmpLen; si++)
        {
            if ((si & 1) == 0)
                s = s + hexCh[so[soi] & 0xf];
            else
            {
                s = s + hexCh[(so[soi] >> 4) & 0xf];
                soi++;
            }
        }
        return s;
    }
};

char Number::OEI = 0x80;

char Number::OEI_EVEN = 0x0;
char Number::OEI_ODD = 0x80;

char Number::NAI = 0x7f;

char Number::NAI_SUBSCRIBER_NUMBER = 0x1;
char Number::NAI_UNKNOWN = 0x2;
char Number::NAI_NATIONAL = 0x3;
char Number::NAI_INTERNATIONAL = 0x4;

char Number::NPI = 0x70;

char Number::NPI_ISDN = 0x10;
char Number::NPI_DNP = 0x30;
char Number::NPI_TNP = 0x40;

///**
// * <p>The CallingPartyNumber class. See Q.763</p>
// * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
// *
// * @author Eduardo Marin
// * @version 2.0
// */
//class CallingPartyNumber : public Number
//{
//public:
//
//    /* constants */
//    static char NI;
//    /* ni 1 bit 7 */
//    static char NI_COMPLETE;
//    static char NI_INCOMPLETE;
//
//    static char APRI;
//    /* apri 2 bit 2 and 3 */
//    static char APRI_PRESENTATION_ALLOWED;
//    static char APRI_PRESENTATION_RESTRICTED;
//    static char APRI_ADDRESS_NA;
//
//    static char SI;
//    /* apri 2 bit 0 and 1 */
//    static char SI_USER_PROVIDED_NV;
//    static char SI_USER_PROVIDED_VP;
//    static char SI_USER_PROVIDED_VF;
//    static char SI_NETWORK_PROVIDED;
//
//private:
//
//    /* private variables */
//    char ni;
//    char apri;
//    char si;
//
//public:
//
//    /**
//     * Creates a new instance of CallingPartyNumber.
//     * @param &nai const char
//     * @param &npi const char
//     * @param &addr const std::string
//     * @param &ni const char
//     * @param &apri const char
//     * @param &si const char
//     */
//    CallingPartyNumber(const char &nai, const char &npi, const std::string &addr, const char &ni, const char &apri, const char &si) : Number(nai, npi, addr)
//    {
//        this->ni = ni;
//        this->apri = apri;
//        this->si = si;
//    }
//
//    /**
//     * Creates a new instance of CallingPartyNumber.
//     * @param *nbr char
//     * @param &len const int
//     */
//    CallingPartyNumber(char *nbr, const int &len) : Number(nbr, len)
//    {
//        if (len > 1)
//        {
//            ni = nbr[1] & NI;
//            apri = nbr[1] & APRI;
//            si = nbr[1] & SI;
//        }
//        else
//        {
//            ni = 0;
//            apri = 0;
//            si = 0;
//        }
//    }
//
//    virtual ~CallingPartyNumber()
//    {
//    }
//
//    /**
//     * Returns the callingPartyNumber.
//     * @param *ch char
//     * @param &len int
//     */
//    void getCallingPartyNumber(char *ch, int &len)
//    {
//        Number::getNumber(ch, len);
//        ch[1] |= ni & NI;
//        ch[1] |= apri & APRI;
//        ch[1] |= si & SI;
//    }
//
//    /**
//     * Returns the ni.
//     * @return char
//     */
//    char getNI()
//    {
//        return ni;
//    }
//
//    /**
//     * Returns the apri.
//     * @return char
//     */
//    char getAPRI()
//    {
//        return apri;
//    }
//
//    /**
//     * Returns the si.
//     * @return char
//     */
//    char getSI()
//    {
//        return si;
//    }
//};
//
//char CallingPartyNumber::NI = 0x80;
//
//char CallingPartyNumber::NI_COMPLETE = 0x0;
//char CallingPartyNumber::NI_INCOMPLETE = 0x80;
//
//char CallingPartyNumber::APRI = 0xc;
//
//char CallingPartyNumber::APRI_PRESENTATION_ALLOWED = 0x0;
//char CallingPartyNumber::APRI_PRESENTATION_RESTRICTED = 0x4;
//char CallingPartyNumber::APRI_ADDRESS_NA = 0x8;
//
//char CallingPartyNumber::SI = 0x3;
//
//char CallingPartyNumber::SI_USER_PROVIDED_NV = 0x0;
//char CallingPartyNumber::SI_USER_PROVIDED_VP = 0x1;
//char CallingPartyNumber::SI_USER_PROVIDED_VF = 0x2;
//char CallingPartyNumber::SI_NETWORK_PROVIDED = 0x3;
//
///**
// * <p>The CalledPartyNumber class. See Q.763</p>
// * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
// *
// * @author Eduardo Marin
// * @version 2.0
// */
//class CalledPartyNumber : public Number
//{
//public:
//
//    /* constants */
//    static char INN;
//    /* inn 1 bit 7 */
//    static char INN_RINNA;
//    static char INN_RINNNA;
//
//private:
//
//    /* private variables */
//    char inn;
//
//public:
//
//    /**
//     * Creates a new instance of CalledPartyNumber.
//     * @param &nai const char
//     * @param &npi const char
//     * @param &addr const std::string
//     * @param &inn const char
//     */
//    CalledPartyNumber(const char &nai, const char &npi, const std::string &addr, const char &inn) : Number(nai, npi, addr)
//    {
//        this->inn = inn;
//    }
//
//    /**
//     * Creates a new instance of CalledPartyNumber.
//     * @param *nbr char
//     * @param &len const int
//     */
//    CalledPartyNumber(char *nbr, const int &len) : Number(nbr, len)
//    {
//        if (len > 1) inn = nbr[1] & INN;
//        else inn = 0;
//    }
//
//    /**
//     * Returns the calledPartyNumber.
//     * @param *ch char
//     * @param &len int
//     */
//    void getCalledPartyNumber(char *ch, int &len)
//    {
//        Number::getNumber(ch, len);
//        ch[1] |= inn & INN;
//    }
//
//    /**
//     * Returns the inn.
//     * @return char
//     */
//    char getINN()
//    {
//        return inn;
//    }
//};

//char CalledPartyNumber::INN = 0x80;
//
//char CalledPartyNumber::INN_RINNA = 0x0;
//char CalledPartyNumber::INN_RINNNA = 0x80;

/**
 * <p>The GenericNumber class. See Q.763</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
//class GenericNumber : public CallingPartyNumber
//{
//public:
//
//    /* constants */
//    static char NQI_RESERVED_DIALED_DIGITS;
//    static char NQI_ADDITIONAL_CALLED_NUMBER;
//    static char NQI_ADDITIONAL_CONNECTED_NUMBER;
//    static char NQI_ADDITIONAL_CALLING_PARTY_NUMBER;
//
//private:
//
//    /* private variables */
//    char nqi;
//
//public:
//
//    /**
//     * Creates a new instance of GenericNumber.
//     * @param &nai const char
//     * @param &npi const char
//     * @param &addr const std::string
//     * @param &ni const char
//     * @param &apri const char
//     * @param &si const char
//     * @param &nqi const char
//     */
//    GenericNumber(const char &nai, const char &npi, const std::string &addr, const char &ni, const char &apri, const char &si, const char &nqi) : CallingPartyNumber(nai, npi, addr, ni, apri, si)
//    {
//        this->nqi = nqi;
//    }
//
//    /**
//     * Creates a new instance of GenericNumber.
//     * @param *nbr char
//     * @param &len const int
//     */
//    GenericNumber(char *nbr, const int &len) : CallingPartyNumber(nbr + 1, len - 1)
//    {
//        if (len > 0) nqi = nbr[0];
//        else nqi = 0;
//    }
//
//    /**
//     * Returns the genericNumber.
//     * @param *ch char
//     * @param &len int
//     */
//    void getGenericNumber(char *ch, int &len)
//    {
//        ch[0] = nqi;
//        CallingPartyNumber::getCallingPartyNumber(ch + 1, len);
//        len++;
//    }
//
//    /**
//     * Returns the nqi.
//     * @return char
//     */
//    char getNQI()
//    {
//        return nqi;
//    }
//};
//
//char GenericNumber::NQI_RESERVED_DIALED_DIGITS = 0x0;
//char GenericNumber::NQI_ADDITIONAL_CALLED_NUMBER = 0x1;
//char GenericNumber::NQI_ADDITIONAL_CONNECTED_NUMBER = 0x5;
//char GenericNumber::NQI_ADDITIONAL_CALLING_PARTY_NUMBER = 0x6;

/**
 * <p>The GenericNumber class. See Q.850</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class Cause
{
public:

    /* constants */
    static char CODING_STANDARD;
    /* codingStandard 2 bit 5 and 6 */
    static char CS_ITU_T;
    static char CS_ISO_IEC;

    static char LOCATION;
    /* location 4 bit 0 to 3 */
    static char L_USER;

    static char CAUSE;
    /* cause 7 bit 0 to 6 */
    static char CAUSE_NORMAL_CALL_CLEARING;

private:

    /* private variables */
    char codingStandard;
    char location;
    char cause;

public:

    /**
     * Creates a new instance of Cause.
     * @param &codingStandard const char
     * @param &location const char
     * @param &cause const char
     */
    Cause(const char &codingStandard, const char &location, const char &cause)
    {
        this->codingStandard = codingStandard;
        this->location = location;
        this->cause = cause;
    }

    /**
     * Creates a new instance of Cause.
     * @param *ch char
     * @param &len const int
     */
    Cause(char *ch, const int &len)
    {
        if (len == 2)
        {
            codingStandard = ch[0] & CODING_STANDARD;
            location = ch[0] & LOCATION;
            cause = ch[1] & CAUSE;
        }
        else
        {
            codingStandard = 0;
            location = 0;
            cause = 0;
        }
    }

    virtual ~Cause()
    {
    }

    /**
     * Returns the cause.
     * @param *ch char
     * @param &len int
     */
    void getCause(char *ch, int &len)
    {
        ch[0] = 0x80 | codingStandard | location;
        ch[1] = 0x80 | cause;
        len = 2;
    }

    /**
     * Returns the codingStandard.
     * @return char
     */
    char getCodingStandard()
    {
        return codingStandard;
    }

    /**
     * Returns the location.
     * @return char
     */
    char getLocation()
    {
        return location;
    }

    /**
     * Returns the cause.
     * @return char
     */
    char getCause()
    {
        return cause;
    }

};

char Cause::CODING_STANDARD = 0x60;

char Cause::CS_ITU_T = 0x00;
char Cause::CS_ISO_IEC = 0x20;

char Cause::LOCATION = 0xf;

char Cause::L_USER = 0x0;

char Cause::CAUSE = 0x7f;

char Cause::CAUSE_NORMAL_CALL_CLEARING = 0x10;

/**
 * <p>The CellGlobalId class. See 3GPP TS 23.003 V4.8.0, 3GPP TS 24.008 V4.8.0</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class CellGlobalId
{
    /* private variables */
    std::string mcc;
    std::string mnc;
    int lac;
    int ci;

public:

    /**
     * Creates a new instance of CellGlobalId.
     * @param *ch char
     * @param &len const int
     */
    CellGlobalId(char *ch, const int &len)
    {
        if (len == 7)
        {
            mcc = Number::toSemiOctetString(Number::OEI_ODD, ch, 2);
            mnc = Number::toSemiOctetString(Number::OEI_EVEN, (ch + 2), 1);
            lac = ((ch[3] & 0xff) << 8) | (ch[4] & 0xff);
            ci = ((ch[5] & 0xff) << 8) | (ch[6] & 0xff);
        }
        else
        {
            mcc = "";
            mnc = "";
            lac = 0;
            ci = 0;
        }
    }

    virtual ~CellGlobalId()
    {
    }

    /**
     * Returns the cellGlobalId.
     * @return std::string
     */
    std::string getCellGlobalId()
    {
        std::stringstream ss;
        ss << mcc << mnc;
        std::stringstream tmpLAC;
        tmpLAC << lac;
        for (int i = tmpLAC.str().length(); i < 5; i++) ss << '0';
        ss << tmpLAC.str();
        std::stringstream tmpCI;
        tmpCI << ci;
        for (int i = tmpCI.str().length(); i < 5; i++) ss << '0';
        ss << tmpCI.str();
        return ss.str();
    }

    /**
     * Returns the mcc.
     * @return std::string
     */
    std::string getMCC()
    {
        return mcc;
    }

    /**
     * Returns the mnc.
     * @return std::string
     */
    std::string getMNC()
    {
        return mnc;
    }

    /**
     * Returns the lac.
     * @return int
     */
    int getLAC()
    {
        return lac;
    }

    /**
     * Returns the ci.
     * @return int
     */
    int getCI()
    {
        return ci;
    }
};

/**
 * <p>The AccessPointName class. See 3GPP TS 23.003 V4.8.0, 3GPP TS 24.008 V4.8.0</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class AccessPointName
{
    /* private variables */
    std::string accessPointName;

public:

    /**
     * Creates a new instance of AccessPointName.
     * @param *ch char
     * @param &len const int
     */
    AccessPointName(char *ch, const int &len)
    {
        if (len > 0)
        {
            int chi = 0;
            while (chi < len)
            {
                int ll = ch[chi];
                for (int i = 0; i < ll; i++)
                {
                    accessPointName += ch[chi + i + 1];
                }

                chi += ll + 1;
                if (chi < len)
                {
                    accessPointName += '.';
                }
            }
        }
        else
        {
            accessPointName = "";
        }
    }

    virtual ~AccessPointName()
    {
    }

    /**
     * Returns the accessPointName.
     * @return std::string
     */
    std::string getAccessPointName()
    {
        return accessPointName;
    }
};

/**
 * Returns the natureOfAddres for a given type.
 * @param &ch const char
 * @return NatureOfAddress
 */
static U8 getNatureOfAddress(const char &ch)
{
    char ton = ch & Address::TON;
    if (ton == Address::TON_INTERNATIONAL)
    {
        return NA_INTERNATIONAL_NUM;
    }
    else if (ton == Address::TON_NATIONAL)
    {
        return NA_NATIONAL_SIGNIFANT_NUM;
    }
    else if (ton == Address::TON_DEDICATED_ACCESS)
    {
        return NA_SUSCRIBER_NUM;
    }
    else
    {
        return NA_UNKNOWN;
    }
}

/**
 * Returns the numberingPlan for a given type.
 * @param &ch const char
 * @return NumberingPlan
 */
static SccpNumberingPlan getNumberingPlan(const char &ch)
{
    char npi = ch & Address::NPI;
    if (npi == Address::NPI_ISDN)
    {
        return NP_ISDN;
    }
    return NP_UNKNOWN;
}

/**
 * <p>The GBlock class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class GBlock
{
    /* private variables */
    Dialog *dialog;

protected:

    /**
     * Gblock Map
     */
    gblock_t gb;

    /**
     * Creates a new instance of GBlock.
     * @param *dialog Dialog
     * @param &bitMask const unsigned char
     * @param &serviceType const CapServiceType
     * @param &serviceMsg const int
     * @param &applicationId const unsigned short
     * @param &invokeId const short
     * @param &linkedId const short
     */
    GBlock(Dialog *dialog,
          const unsigned char &bitMask,
          const ServiceType &serviceType,
          const int &serviceMsg,
          const unsigned short &applicationId,
          const short &invokeId,
          const short &linkedId)
    {
        this->dialog = dialog;
        gb.bit_mask = bitMask;
        gb.serviceType = serviceType;
        gb.serviceMsg = serviceMsg;
        gb.applicationId = applicationId;
        gb.invokeId = invokeId;
        gb.linkedId = linkedId;
    }

public:

    virtual ~GBlock()
    {
        // No op
    }

    /**
     * Returns the capBlock.
     * @return *CapBlock
     */
    gblock_t* getGBlock()
    {
        gb.dialogId = dialog->getDialogId();
        return &gb;
    }

    /**
     * Returns the dialog.
     * @return *Dialog
     */
    Dialog* getDialog()
    {
        return dialog;
    }
};

/**
 * The PutBlockQueue abstract class.
 */
class PutGBlockQueue : public Thread
{
protected:

    /** The blockQueue. */
    Queue<GBlock> blockQueue;

public:

    /**
     * Creates a new instance of PutBlockQueue.
     */
    PutGBlockQueue()
    {
        // No op
    }

    virtual ~PutGBlockQueue()
    {
        // No op
    }

    /**
     * Pushes a block.
     * @param *block GBlock
     */
    void push(GBlock *block)
    {
        blockQueue.push(block);
    }
};
/****************************************/
/* Definition of GBlock implementations */
/****************************************/
/**
 * <p>The OpenReqBlock class.</p>
 */
class OpenReqBlock : public GBlock
{
    /* private variables */
    ObjectID *objectId;
    unsigned char localSSN;
    unsigned int localPC;
    std::string localMSISDN;
    unsigned char localMSISDNType;
    unsigned char remoteSSN;
    unsigned int remotePC;
    std::string remoteMSISDN;
    unsigned char remoteMSISDNType;

public:

    /**
     * Creates a new instance of OpenReqBlock.
     * @param *dialog Dialog
     * @param *objectId ObjectID
     * @param &localSSN const unsigned char
     * @param &localPC const unsigned int
     * @param &localMSISDN const std::string
     * @param &localMSISDNType const unsigned char
     * @param &remoteSSN const unsigned char
     * @param &remotePC const unsigned int
     * @param &remoteMSISDN const std::string
     * @param &remoteMSISDNType const unsigned char
     */
    OpenReqBlock(Dialog *dialog,
                 ObjectID *objectId,
                 const unsigned char &localSSN,
                 const unsigned int &localPC,
                 const std::string &localMSISDN,
                 const unsigned char &localMSISDNType,
                 const unsigned char &remoteSSN,
                 const unsigned int &remotePC,
                 const std::string &remoteMSISDN,
                 const unsigned char &remoteMSISDNType) : GBlock(dialog, gblock_t_parameter_present, GMAP_REQ, GMAP_OPEN, 0, -1)
    {
        this->objectId = objectId;
        this->localSSN = localSSN;
        this->localPC = localPC;
        this->localMSISDN = localMSISDN;
        this->localMSISDNType = localMSISDNType;
        this->remoteSSN = remoteSSN;
        this->remotePC = remotePC;
        this->remoteMSISDN = remoteMSISDN;
        this->remoteMSISDNType = remoteMSISDNType;
    }

    gblock_t* getGBlock()
    {
        gblock_t *gb = GBlock::getGBlock();
        gb->parameter.openArg.bit_mask = MAP_OpenArg_originatingAddress_present;
        memcpy(&gb->parameter.openArg.applicationContext, objectId, sizeof (ObjectID));
        // originatingAddress
        gb->parameter.openArg.originatingAddress.bit_mask = MAP_SccpAddr_gt_present;
        gb->parameter.openArg.originatingAddress.routingIndicator = routeOnGt;
        gb->parameter.openArg.originatingAddress.ssn = localSSN;
        gb->parameter.openArg.originatingAddress.pointCode = localPC;
        gb->parameter.openArg.originatingAddress.gt.bit_mask = MAP_SccpAddr_gt_numberingPlan_present;
        gb->parameter.openArg.originatingAddress.gt.natureOfAddress = getNatureOfAddress(localMSISDNType);
        gb->parameter.openArg.originatingAddress.gt.numberingPlan = getNumberingPlan(localMSISDNType);
        int tmpLocalLen = 32;
        unsigned char tmpLocalMSISDN[tmpLocalLen];
        getOctet(localMSISDN, tmpLocalMSISDN, tmpLocalLen);
        gb->parameter.openArg.originatingAddress.gt.msisdnLength = tmpLocalLen;
        memcpy(gb->parameter.openArg.originatingAddress.gt.msisdn, tmpLocalMSISDN, tmpLocalLen);
        gb->parameter.openArg.originatingAddress.gt.gtIndicator = 4;
        gb->parameter.openArg.originatingAddress.gt.translationType = 0;

        // destinationAddress
        gb->parameter.openArg.destinationAddress.bit_mask = MAP_SccpAddr_gt_present;
        gb->parameter.openArg.destinationAddress.routingIndicator = routeOnGt;
        gb->parameter.openArg.destinationAddress.ssn = remoteSSN;
        gb->parameter.openArg.destinationAddress.pointCode = remotePC;
        gb->parameter.openArg.destinationAddress.gt.bit_mask = MAP_SccpAddr_gt_numberingPlan_present;
        gb->parameter.openArg.destinationAddress.gt.natureOfAddress = getNatureOfAddress(remoteMSISDNType);
        gb->parameter.openArg.destinationAddress.gt.numberingPlan = getNumberingPlan(remoteMSISDNType);
        int tmpRemoteLen = 32;
        unsigned char tmpRemoteMSISDN[tmpRemoteLen];
        getOctet(remoteMSISDN, tmpRemoteMSISDN, tmpRemoteLen);
        gb->parameter.openArg.destinationAddress.gt.msisdnLength = tmpRemoteLen;
        memcpy(gb->parameter.openArg.destinationAddress.gt.msisdn, tmpRemoteMSISDN, tmpRemoteLen);
        gb->parameter.openArg.destinationAddress.gt.gtIndicator = 4;
        gb->parameter.openArg.destinationAddress.gt.translationType = 0;
        return gb;
    }
};
/**
 * <p>The OpenResBlock class.</p>
 */
class OpenResBlock : public GBlock
{
    /* private variables */
    DialogResult result;

public:

    /**
     * Creates a new instance of OpenResBlock.
     * @param *dialog Dialog
     * @param &result DialogResult
     */
    OpenResBlock(Dialog *dialog,
                 const DialogResult &result) : GBlock(dialog, gblock_t_parameter_present, GMAP_RSP, GMAP_OPEN, 0, -1)
    {
        this->result = result;
    }

    gblock_t* getGBlock()
    {
        gblock_t *gb = GBlock::getGBlock();
        gb->parameter.openRes.bit_mask = 0;
        gb->parameter.openRes.result = result;
        return gb;
    }
};
/**
 * <p>The DelimiterReqBlock class.</p>
 */
class DelimiterReqBlock : public GBlock
{
public:

    /**
     * Creates a new instance of DelimiterReqBlock.
     * @param *dialog Dialog
     */
    DelimiterReqBlock(Dialog *dialog) : GBlock(dialog, gblock_t_parameter_present, GMAP_REQ, GMAP_DELIMITER, 0, -1)
    {
        // No op
    }

    gblock_t* getGBlock()
    {
        gblock_t *gb = GBlock::getGBlock();
        gb->parameter.delimiter.qualityOfService = CL_SVC_CLASS_0;
        return gb;
    }
};
/**
 * <p>The CloseReqBlock class.</p>
 */
class CloseReqBlock : public GBlock
{
public:

    /**
     * Creates a new instance of CloseReqBlock.
     * @param *dialog Dialog
     */
    CloseReqBlock(Dialog *dialog) : GBlock(dialog, gblock_t_parameter_present, GMAP_REQ, GMAP_CLOSE, 0, -1)
    {
        // No op
    }

    gblock_t* getGBlock()
    {
        gblock_t* gb = GBlock::getGBlock(gb);
        gb->parameter.closeArg.releaseMethod = normalRelease;
        gb->parameter.closeArg.qualityOfService = CL_SVC_CLASS_0;
        return gb;
    }

};
/**
 * <p>The PAbortReqBlock class.</p>
 */
class PAbortReqBlock : public GBlock
{
public:

    /**
     * Creates a new instance of PAbortReqBlock.
     * @param *dialog Dialog
     */
    PAbortReqBlock(Dialog *dialog) : GBlock(dialog, gblock_t_parameter_present, GMAP_REQ, GMAP_P_ABORT, 0, -1)
    {
        // No op
    }

    gblock_t* getGBlock()
    {
        gblock_t *gb = GBlock::getGBlock(gb);
        gb->parameter.pAbortArg.providerReason = ressourceLimitation;
        gb->parameter.pAbortArg.source = networkServiceProblem;
        return gb;
    }
};
/**
 * <p>The UAbortReqBlock class.</p>
 */
class UAbortReqBlock : public GBlock
{
public:

    /**
     * Creates a new instance of UAbortReqBlock.
     * @param *dialog Dialog
     */
    UAbortReqBlock(Dialog *dialog) : GBlock(dialog, gblock_t_parameter_present, GMAP_REQ, GMAP_U_ABORT, 0, -1)
    {
        // No op
    }

    gblock_t* getGBlock()
    {
        gblock_t* gb = GBlock::getGBlock();
        gb->parameter.uAbortArg.userReason.choice = MAP_UserAbortChoice_applicationProcedureCancellation_chosen;
        gb->parameter.uAbortArg.userReason.u.applicationProcedureCancellation = ProcedureCancellationReason_remoteOperationsFailure;
        gb->parameter.uAbortArg.qualityOfService = CL_SVC_CLASS_0;
        return gb;
    }

};
/**
 * <p>The ErrorBlock class.</p>
 */
class ErrorBlock : public GBlock
{
public:

    /**
     * Creates a new instance of ErrorBlock.
     * @param *dialog Dialog
     */
    ErrorBlock(Dialog *dialog, const int &serviceMsg) : GBlock(dialog, 0, GMAP_ERROR, serviceMsg, 0, -1)
    {
        // No op
    }

    gblock_t* getGBlock()
    {
        return GBlock::getGBlock();
    }
};

class AnytimeInterrogationReqBlock : public GBlock
{
private:
    string name;
    unsigned char type;
    unsigned char length;
    unsigned char smRPPRI;
    string serviceCentreAddress;
    unsigned char serviceCentreAddressType;

public:
    AnytimeInterrogationReqBlock(Dialog* dialog,
                                 const short& invokeId,
                                 const string& name,
                                 const unsigned char& type,
                                 const unsigned char& length, // TODO msisdnlength
                                 const unsigned char& smRPPRI,
                                 const string& serviceCentreAddress,
                                 const unsigned char& serviceCentreAddressType) : GBlock(dialog, gblock_t_parameter_present, GMAP_REQ, ANY_TIME_INTERROGATION, 0, invokeId, -1)
    {
        this->name = name;
        this->type = type;
        this->length = length;
        this->smRPPRI = smRPPRI;
        this->serviceCentreAddress = serviceCentreAddress;
        this->serviceCentreAddressType = serviceCentreAddressType;
    }
    
    gblock_t* getGBlock() {
        gblock_t* gb = GBlock::getGBlock();
        int tmpIMSILen = 32;
        unsigned char tmpIMSI[tmpIMSILen];
        Address msisdnAddr(name, length);
        msisdnAddr.getAddress(tmpIMSI, tmpIMSILen);
        // pgb->parameter.anyTimeInterrogationArg_v3.subscriberIdentity.choice = SubscriberIdentity_imsi_chosen;
        gb->parameter.anyTimeInterrogationArg_v3.subscriberIdentity.choice = SubscriberIdentity_imsi_chosen;
        gb->parameter.anyTimeInterrogationArg_v3.subscriberIdentity.u.imsi.length = tmpIMSILen;
        memcpy(gb->parameter.anyTimeInterrogationArg_v3.subscriberIdentity.u.imsi.value, tmpIMSI, tmpIMSILen);
        
        
//        gb->parameter.routingInfoForSM_Arg_v1.sm_RP_PRI=smRPPRI;
//        int tmpSCALen=32;
//        unsigned char tmpSCA[tmpSCALen];
//        getAddress(serviceCentreAddress, serviceCentreAddressType, tmpSCA, tmpSCALen);
//        gb->parameter.routingInfoForSM_Arg_v1.serviceCentreAddress.length=tmpSCALen;
//        memcpy(gb->parameter.routingInfoForSM_Arg_v1.serviceCentreAddress.value, tmpSCA, tmpSCALen);
        
        return GBlock::getGBlock();
    }
};
//
//class AnytimeInterrogationResBlock : public GBlock {
//    // TODO 
//};

#endif	/* BLOCK_HPP */
