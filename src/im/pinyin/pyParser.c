/***************************************************************************
 *   Copyright (C) 2002~2005 by Yuking                                     *
 *   yuking_net@sohu.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <stdio.h>
#include <string.h>

#include "fcitx/fcitx.h"

#include "pyMapTable.h"
#include "PYFA.h"
#include "sp.h"
#include "pyParser.h"
#include "pyconfig.h"
#include "fcitx/ime.h"
#include "py.h"

extern const ConsonantMap consonantMapTable[];
extern const SyllabaryMap syllabaryMapTable[];

int IsSyllabary (const char *strPY, boolean bMode)
{
    register int    i;

    for (i = 0; syllabaryMapTable[i].cMap; i++) {
        if (bMode) {
            if (!strncmp (strPY, syllabaryMapTable[i].strPY, strlen (syllabaryMapTable[i].strPY)))
                return i;
        }
        else {
            if (!strcmp (strPY, syllabaryMapTable[i].strPY))
                return i;
        }
    }

    return -1;
}

int IsConsonant (const char *strPY, boolean bMode)
{
    register int    i;

    for (i = 0; consonantMapTable[i].cMap; i++) {
        if (bMode) {
            if (!strncmp (strPY, consonantMapTable[i].strPY, strlen (consonantMapTable[i].strPY)))
                return i;
        }
        else {
            if (!strcmp (strPY, consonantMapTable[i].strPY))
                return i;
        }
    }

    return -1;
}

int FindPYFAIndex (FcitxPinyinConfig *pyconfig, const char *strPY, boolean bMode)
{
    int             i;
    
    for (i = 0; pyconfig->PYTable[i].strPY[0] != '\0'; i++) {
        int cmp;
        if (bMode)
            cmp = strncmp (strPY, pyconfig->PYTable[i].strPY, strlen ( pyconfig->PYTable[i].strPY));
        else
            cmp = strcmp (strPY, pyconfig->PYTable[i].strPY);
        if (!cmp) {
            if (!pyconfig->PYTable[i].pMH)
                return i;
            else if (*(pyconfig->PYTable[i].pMH))
            {
                /* trick: not the kind of misstype */
                if (pyconfig->PYTable[i].pMH != &pyconfig->bMisstype )
                    return i;
                else
                    /* fixed pinyin is valid? */
                    if (!pyconfig->PYTable[i + 1].pMH || *(pyconfig->PYTable[i + 1].pMH))
                        return i;
            }
        }
    }

    return -1;
}

void ParsePY (FcitxPinyinConfig *pyconfig, const char *strPY, ParsePYStruct * parsePY, PYPARSEINPUTMODE mode, boolean bSP)
{
    const char           *strP;
    int             iIndex;
    int             iTemp;
    char            str_Map[3];
    char            strTemp[7];

    parsePY->iMode = PARSE_SINGLEHZ;
    strP = strPY;
    parsePY->iHZCount = 0;

    if (bSP) {
        char            strQP[7];
        char            strJP[3];

        strJP[2] = '\0';

        while (*strP) {
            strJP[0] = *strP++;
            strJP[1] = *strP;
            SP2QP (pyconfig, strJP, strQP);
            MapPY (pyconfig, strQP, str_Map, mode);

            if (!*strP) {
                strcpy (parsePY->strMap[parsePY->iHZCount], str_Map);
                strcpy (parsePY->strPYParsed[parsePY->iHZCount++], strJP);
                break;
            }

            iIndex = FindPYFAIndex (pyconfig, strQP, 0);
            if (iIndex != -1) {
                strcpy (parsePY->strMap[parsePY->iHZCount], str_Map);
                strcpy (parsePY->strPYParsed[parsePY->iHZCount++], strJP);
                strP++;
            }
            else {
                strJP[1] = '\0';
                SP2QP (pyconfig, strJP, strQP);
                if (!MapPY (pyconfig, strQP, str_Map, mode))
                    strcpy (parsePY->strMap[parsePY->iHZCount], strJP);
                else
                    strcpy (parsePY->strMap[parsePY->iHZCount], str_Map);
                strcpy (parsePY->strPYParsed[parsePY->iHZCount++], strJP);
            }

            if (*strP == PY_SEPARATOR) {
                strcat (parsePY->strPYParsed[parsePY->iHZCount - 1], PY_SEPARATOR_S);
                while (*strP == PY_SEPARATOR )
                    strP++;
            }
        }
    }
    else {
        boolean            bSeperator = false;

        do {
            iIndex = FindPYFAIndex (pyconfig, strP, 1);

            if (iIndex != -1) {
                size_t lIndex = strlen (pyconfig->PYTable[iIndex].strPY);
                strTemp[0] = pyconfig->PYTable[iIndex].strPY[lIndex - 1];
                iTemp = -1;
                if (strTemp[0] == 'g' || strTemp[0] == 'n') {
                    strncpy (strTemp, strP, lIndex - 1);
                    strTemp[lIndex - 1] = '\0';

                    iTemp = FindPYFAIndex (pyconfig, strTemp, 0);
                    if (iTemp != -1) {
                        iTemp = FindPYFAIndex (pyconfig, strP + strlen (pyconfig->PYTable[iTemp].strPY), 1);
                        if (iTemp != -1) {
                            if (strlen (pyconfig->PYTable[iTemp].strPY) == 1 || !strcmp ("ng", pyconfig->PYTable[iTemp].strPY))
                                iTemp = -1;
                        }
                        if (iTemp != -1) {
                            strncpy (strTemp, strP, lIndex - 1);
                            strTemp[lIndex - 1] = '\0';
                        }
                    }
                }
                if (iTemp == -1)
                    strcpy (strTemp, pyconfig->PYTable[iIndex].strPY);
                MapPY (pyconfig, strTemp, str_Map, mode);
                strcpy (parsePY->strMap[parsePY->iHZCount], str_Map);
                strP += strlen (strTemp);

                if (bSeperator) {
                    bSeperator = false;
                    parsePY->strPYParsed[parsePY->iHZCount][0] = PY_SEPARATOR;
                    parsePY->strPYParsed[parsePY->iHZCount][1] = '\0';
                }
                else
                    parsePY->strPYParsed[parsePY->iHZCount][0] = '\0';
                strcat (parsePY->strPYParsed[parsePY->iHZCount++], strTemp);
            }
            else {
                if (pyconfig->bFullPY && *strP != PY_SEPARATOR)
                    parsePY->iMode = PARSE_ERROR;

                iIndex = IsConsonant (strP, 1);
                if (-1 != iIndex) {
                    parsePY->iMode = PARSE_ERROR;

                    if (bSeperator) {
                        bSeperator = false;
                        parsePY->strPYParsed[parsePY->iHZCount][0] = PY_SEPARATOR;
                        parsePY->strPYParsed[parsePY->iHZCount][1] = '\0';
                    }
                    else
                        parsePY->strPYParsed[parsePY->iHZCount][0] = '\0';
                    strcat (parsePY->strPYParsed[parsePY->iHZCount], consonantMapTable[iIndex].strPY);
                    MapPY (pyconfig, consonantMapTable[iIndex].strPY, str_Map, mode);
                    strcpy (parsePY->strMap[parsePY->iHZCount++], str_Map);
                    strP += strlen (consonantMapTable[iIndex].strPY);
                }
                else {
                    iIndex = IsSyllabary (strP, 1);
                    if (-1 != iIndex) {
                        if (bSeperator) {
                            bSeperator = false;
                            parsePY->strPYParsed[parsePY->iHZCount][0] = PY_SEPARATOR;
                            parsePY->strPYParsed[parsePY->iHZCount][1] = '\0';
                        }
                        else
                            parsePY->strPYParsed[parsePY->iHZCount][0] = '\0';
                        strcat (parsePY->strPYParsed[parsePY->iHZCount], syllabaryMapTable[iIndex].strPY);
                        MapPY (pyconfig, syllabaryMapTable[iIndex].strPY, str_Map, mode);
                        strcpy (parsePY->strMap[parsePY->iHZCount++], str_Map);

                        strP += strlen (syllabaryMapTable[iIndex].strPY);
                        if (parsePY->iMode != PARSE_ERROR)
                            parsePY->iMode = PARSE_ABBR;
                    }
                    else {  //必定是分隔符
                        strP++;
                        bSeperator = true;
                        parsePY->strPYParsed[parsePY->iHZCount][0] = PY_SEPARATOR;
                        parsePY->strPYParsed[parsePY->iHZCount][1] = '\0';
                        parsePY->strMap[parsePY->iHZCount][0] = '0';
                        parsePY->strMap[parsePY->iHZCount][1] = '0';
                        parsePY->strMap[parsePY->iHZCount][2] = '\0';
                    }
                }
            }
        } while (*strP);
    }

    if (strPY[strlen (strPY) - 1] == PY_SEPARATOR && !bSP)
        parsePY->iHZCount++;

    if (parsePY->iMode != PARSE_ERROR) {
        parsePY->iMode = parsePY->iMode & PARSE_ABBR;
        if (parsePY->iHZCount > 1)
            parsePY->iMode = parsePY->iMode | PARSE_PHRASE;
        else
            parsePY->iMode = parsePY->iMode | PARSE_SINGLEHZ;
    }
}

/*
 * 将一个拼音(包括仅为声母或韵母)转换为拼音映射
 * 返回true为转换成功，否则为false(一般是因为strPY不是一个标准的拼音)
 */
boolean MapPY (FcitxPinyinConfig* pyconfig, const char* strPYorigin, char strMap[3], PYPARSEINPUTMODE mode)
{
    char            str[5];
    char            strPY[7];
    int             iIndex;
    
    strcpy(strPY, strPYorigin);
    
    size_t          len = strlen(strPY);
    if (pyconfig->bMisstype && strPY[len - 1] == 'n' && strPY[len - 2] == 'g')
    {
        strPY[len - 2] = 'n';
        strPY[len - 1] = 'g';
    }

    //特殊处理eng
    if (!strcmp (strPY, "eng") && pyconfig->MHPY_C[1].bMode) {
        strcpy (strMap, "X0");
        return true;
    }

    strMap[2] = '\0';
    iIndex = IsSyllabary (strPY, 0);
    if (-1 != iIndex) {
        strMap[0] = syllabaryMapTable[iIndex].cMap;
        strMap[1] = mode;
        return true;
    }
    iIndex = IsConsonant (strPY, 0);

    if (-1 != iIndex) {
        strMap[0] = mode;
        strMap[1] = consonantMapTable[iIndex].cMap;
        return true;
    }

    str[0] = strPY[0];
    str[1] = '\0';

    if (strPY[1] == 'h' || strPY[1] == 'g') {
        str[0] = strPY[0];
        str[1] = strPY[1];
        str[2] = '\0';
        iIndex = IsSyllabary (str, 0);
        strMap[0] = consonantMapTable[iIndex].cMap;
        iIndex = IsConsonant (strPY + 2, 0);
        strMap[1] = consonantMapTable[iIndex].cMap;
    }
    else {
        str[0] = strPY[0];
        str[1] = '\0';
        iIndex = IsSyllabary (str, 0);
        if (iIndex == -1)
            return false;
        strMap[0] = consonantMapTable[iIndex].cMap;
        iIndex = IsConsonant (strPY + 1, 0);
        if (iIndex == -1)
            return false;
        strMap[1] = consonantMapTable[iIndex].cMap;
    }

    return true;
}

/*
 * 将一个完整的拼音映射转换为拼音，返回false表示失败，
 * 一般原因是拼音映射不正确
 */
boolean MapToPY (char strMap[3], char *strPY)
{
    int             i;

    strPY[0] = '\0';
    if (strMap[0] != ' ') {
        i = 0;
        while (syllabaryMapTable[i].cMap) {
            if (syllabaryMapTable[i].cMap == strMap[0]) {
                strcpy (strPY, syllabaryMapTable[i].strPY);
                break;
            }
            i++;
        }
        if (!strlen (strPY))
            return false;
    }

    if (strMap[1] != ' ') {
        i = 0;
        while (consonantMapTable[i].cMap) {
            if (consonantMapTable[i].cMap == strMap[1]) {
                strcat (strPY, consonantMapTable[i].strPY);
                return true;
            }
            i++;
        }
    }
    else
        return true;

    return false;
}

/*
 * 比较一位拼音映射
 * 0表示相等
 * b指示是声母还是韵母，true表示声母
 */
int Cmp1Map (FcitxPinyinConfig* pyconfig, char map1, char map2, boolean b, boolean bUseMH, boolean bSP)
{
    int             iVal1, iVal2;

    if (map2 == '0' || map1 == '0') {
        if (map1 == ' ' || map2 == ' ' || !pyconfig->bFullPY || bSP)
            return 0;
    }
    else {
        if (b) {
            iVal1 = GetMHIndex_S (pyconfig->MHPY_S, map1, bUseMH);
            iVal2 = GetMHIndex_S (pyconfig->MHPY_S, map2, bUseMH);
        }
        else {
            iVal1 = GetMHIndex_C (pyconfig->MHPY_C, map1);
            iVal2 = GetMHIndex_C (pyconfig->MHPY_C, map2);
        }
        if (iVal1 == iVal2)
            if (iVal1 >= 0)
                return 0;
    }

    return (map1 - map2);
}

/*
 * 比较第二位拼音映射是否和第一个相等
 * 0表示相等
 * >0表示前者大
 * <0表示后者大
 */
int Cmp2Map (FcitxPinyinConfig* pyconfig, char map1[3], char map2[3], boolean bSP)
{
    int             i;

    if (IsZ_C_S(map2[0]) && map2[1]=='0')
        i = Cmp1Map (pyconfig, map1[0], map2[0], true, true, bSP);
    else
        i = Cmp1Map (pyconfig, map1[0], map2[0], true, false, bSP);

    if (i)
        return i;

    return Cmp1Map (pyconfig, map1[1], map2[1], false, false, bSP);
}

/*
 * 判断strMap2是否与strMap1相匹配
 * 是 返回值为0
 * 否 返回值不为0
 * *iMatchedLength 记录了二者能够匹配的长度
 */
int CmpMap (FcitxPinyinConfig* pyconfig, char *strMap1, char *strMap2, int *iMatchedLength, boolean bSP)
{
    int             val;

    *iMatchedLength = 0;
    for (;;) {
        if (!strMap2[*iMatchedLength])
            return (strMap1[*iMatchedLength] - strMap2[*iMatchedLength]);
        if ( ((*iMatchedLength + 1) % 2) && (IsZ_C_S(strMap2[*iMatchedLength]) && (strMap2[*iMatchedLength + 1]=='0' || !strMap2[*iMatchedLength + 1]) ) )
            val = Cmp1Map (pyconfig, strMap1[*iMatchedLength], strMap2[*iMatchedLength], (*iMatchedLength + 1) % 2, true, bSP);
        else
            val = Cmp1Map (pyconfig, strMap1[*iMatchedLength], strMap2[*iMatchedLength], (*iMatchedLength + 1) % 2, false, bSP);

        if (val)
            return val;

        (*iMatchedLength)++;
    }

    return 0;
}
