/*
 * This file is part of BMP2Array.
 * (c) 2017-2019 Marco Giammarini
 *
 * BMP2Array is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * BMP2Array is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Nome-Programma.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PARSEBMP_H
#define PARSEBMP_H

#include <QFile>
#include <QDataStream>

#include "metadata.h"

typedef enum
{
    BMPTYPE_1BIT = 1,
    BMPTYPE_4BIT = 4,
    BMPTYPE_8BIT = 8,
    BMPTYPE_16BIT = 16,
    BMPTYPE_24BIT = 24,
} BMPType;

typedef enum
{
    BMPERRORS_OK,
    BMPERRORS_WRONG_PARAM,
    BMPERRORS_CONVERSION_FAIL,
} BMPErrors;

typedef enum
{
    BMPCOMPRESSIONTYPE_NONE = 0,
    BMPCOMPRESSIONTYPE_RLE8 = 1,
    BMPCOMPRESSIONTYPE_RLE4 = 2,
    BMPCOMPRESSIONTYPE_BITFIELDS = 3,
    BMPCOMPRESSIONTYPE_JPEG= 4,
    BMPCOMPRESSIONTYPE_PNG = 5,
} BMPCompressionType;

/**
 * https://en.wikipedia.org/wiki/BMP_file_format
 * http://www.ece.ualberta.ca/~elliott/ee552/studentAppNotes/2003_w/misc/bmp_file_format/bmp_file_format.htm
 */
typedef struct BMPData
{
    // File Title
    char signature[2];
    quint32 dimension;
    quint32 offset;

    // Image Title
    quint32 headerDimension;
    quint32 width;
    quint32 height;
    BMPType colorDepth;
    BMPCompressionType compression;
    quint32 imageDimension;
    quint32 colorUsed;
    quint32 colorImportant; // 0 means all are importants

    bool isPalette;

    bool isImageReaded;

    quint32 numByteRow;
    quint8 image[500][500];

    // TRUE if the BMP was parse
    bool isComplete;
} BMPData;

class ParseBMP
{
public:    
    ParseBMP(QFile *bmp, BMPType type);

    void dumpImage();

    void saveArray(const char* output, bool reverse);

private:
    void readFileTitle();
    void readImageTitle();
    void readPalette();
    BMPErrors readImage();

    BMPData m_bmp;
    QDataStream* m_stream;
};

#endif // PARSEBMP_H
