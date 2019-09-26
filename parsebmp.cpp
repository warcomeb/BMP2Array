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

#include "parsebmp.h"

#if SHOW_DEBUG == 1
#include <QDebug>
#endif

#include <QTextStream>
#include <iostream>

ParseBMP::ParseBMP(QFile *bmp, BMPType type)
{
    m_stream = new QDataStream(bmp);
    m_stream->setByteOrder(QDataStream::LittleEndian);
    m_bmp.isComplete = false;

    readFileTitle();
    readImageTitle();

    if (m_bmp.headerDimension != m_bmp.offset)
    {
        m_bmp.isPalette = true;
        readPalette();
    }
    else
    {
        m_bmp.isPalette = false;
#if SHOW_DEBUG == 1
    qDebug() << "PALETTE: NO!";
#endif
    }

    BMPErrors bpmErr = readImage();
    if (bpmErr != BMPERRORS_OK)
    {
#if SHOW_DEBUG == 1
        qDebug() << "READING FAIL!";
#endif
    }
    else
    {
#if SHOW_DEBUG == 1
        qDebug() << "READING COMPLETE!";
#endif
    }

    // Close the file!
    bmp->close();
    m_bmp.isComplete = true;
}

void ParseBMP::readFileTitle()
{
    char buffer[4];

    m_stream->readRawData(buffer,2);
    m_bmp.signature[0] = buffer[0];
    m_bmp.signature[1] = buffer[1];
#if SHOW_DEBUG == 1
    qDebug() << "FILE TYPE:" << m_bmp.signature;
#endif

    // Read file dimension
    *m_stream >> m_bmp.dimension;
#if SHOW_DEBUG == 1
    qDebug() << "FILE DIM:" << m_bmp.dimension;
#endif

    // Reserved!
    m_stream->readRawData(buffer,4);
    // Read file offset for data
    *m_stream >> m_bmp.offset;
#if SHOW_DEBUG == 1
    qDebug() << "FILE OFFSET:" << m_bmp.offset;
#endif
}

void ParseBMP::readImageTitle()
{
    char buffer[4];
    quint16 temp16;
    quint32 temp32;

    // Image header dimension
    *m_stream >> m_bmp.headerDimension;
    // Add the first 14 bytes
    m_bmp.headerDimension += 14;
#if SHOW_DEBUG == 1
    qDebug() << "HEADER DIM:" << m_bmp.headerDimension;
#endif

    // Read image width in pixel
    *m_stream >> m_bmp.width;
#if SHOW_DEBUG == 1
    qDebug() << "WIDTH:" << m_bmp.width;
#endif

    // Read image height in pixel
    *m_stream >> m_bmp.height;
#if SHOW_DEBUG == 1
    qDebug() << "HEIGHT:" << m_bmp.height;
#endif

    // Reserved!
    m_stream->readRawData(buffer,2);
    // Read color depth: 1, 4, 8, 16 or 24 bit
    *m_stream >> temp16;
    m_bmp.colorDepth = (BMPType) temp16;
#if SHOW_DEBUG == 1
    qDebug() << "DEPTH:" << temp16;
#endif

    // Read compression type
    *m_stream >> temp32;
    m_bmp.compression = (BMPCompressionType) temp32;
#if SHOW_DEBUG == 1
    qDebug() << "COMPRESSION:" << temp32;
#endif

    // Read image dimension without header
    // Read image height in pixel
    *m_stream >> m_bmp.imageDimension;
#if SHOW_DEBUG == 1
    qDebug() << "IMAGE DIM:" << m_bmp.imageDimension;
#endif

    // FIXME: Unused informations
    m_stream->readRawData(buffer,4);
    m_stream->readRawData(buffer,4);
    *m_stream >> m_bmp.colorUsed;
#if SHOW_DEBUG == 1
    qDebug() << "COLOR USED:" << m_bmp.colorUsed;
#endif
    *m_stream >> m_bmp.colorImportant;
#if SHOW_DEBUG == 1
    qDebug() << "COLOR IMPORTANT:" << m_bmp.colorImportant;
#endif
}

void ParseBMP::readPalette()
{
    char buffer[4];
    uint number = (m_bmp.offset - m_bmp.headerDimension) / 4;

#if SHOW_DEBUG == 1
    qDebug() << "PALETTE NUM:" << number;
#endif

    for (uint i = 0; i < number; ++i)
    {
        m_stream->readRawData(buffer,4);
#if SHOW_DEBUG == 1
        qDebug() << "PALETTE VALUE:" << QChar(buffer[0]).unicode() << ";" <<
                                        QChar(buffer[1]).unicode() << ";" <<
                                        QChar(buffer[2]).unicode() << ";" <<
                                        QChar(buffer[3]).unicode(); //<< buffer[1] << buffer[2] << buffer[3];
#endif
    }
}

BMPErrors ParseBMP::readImage()
{
    int result = 0;

    if (m_bmp.colorDepth == BMPTYPE_1BIT)
    {
        // Number of byte per row: it must be multiple of 4!
        m_bmp.numByteRow = ((m_bmp.width * m_bmp.colorDepth) +
                            ((((m_bmp.width * m_bmp.colorDepth) % 32) == 0) ? 0 : (32 - (m_bmp.width * m_bmp.colorDepth) % 32))) / 8;

#if SHOW_DEBUG == 1
        qDebug() << "BYTE ROW:" << m_bmp.numByteRow;
#endif

        for (uint i = 0; i < m_bmp.height; ++i)
        {
            result = m_stream->readRawData((char *)m_bmp.image[i],m_bmp.numByteRow);

            if (result == -1)
            {
                m_bmp.isImageReaded = false;
                return BMPERRORS_CONVERSION_FAIL;
            }
        }
        m_bmp.isImageReaded = true;
        return BMPERRORS_OK;
    }
    else if (m_bmp.colorDepth == BMPTYPE_4BIT)
    {
        // Number of byte per row: it must be multiple of 4!
        m_bmp.numByteRow = (m_bmp.width * m_bmp.colorDepth)/8;
        quint32 padding = (32 - ((m_bmp.width * m_bmp.colorDepth) % 32))/8;
        m_bmp.numByteRow += (padding == 4) ? 0 : padding;

#if SHOW_DEBUG == 1
        qDebug() << "BYTE ROW:" << m_bmp.numByteRow;
#endif

        for (uint i = 0; i < m_bmp.height; ++i)
        {
            result = m_stream->readRawData((char *)m_bmp.image[i],m_bmp.numByteRow);

            if (result == -1)
            {
                m_bmp.isImageReaded = false;
                return BMPERRORS_CONVERSION_FAIL;
            }
        }
        m_bmp.isImageReaded = true;
        return BMPERRORS_OK;
    }
    else
    {
        m_bmp.isImageReaded = false;
        return BMPERRORS_WRONG_PARAM;
    }
}

void ParseBMP::dumpImage()
{
#if SHOW_DEBUG == 1
    qDebug() << "DUMP IMAGE...";
#endif
    if (!m_bmp.isImageReaded)
    {
#if SHOW_DEBUG == 1
        qDebug() << "IMAGE NOT READED!";
        return;
#endif
    }

    if (m_bmp.isComplete)
    {
        if (m_bmp.colorDepth == BMPTYPE_1BIT)
        {
            for (uint i = 0; i < m_bmp.height; ++i)
            {
                for (uint j = 0; j < m_bmp.numByteRow; ++j)
                {
                    quint8 line = m_bmp.image[i][j];
                    for (uint k = 0; k < 8; ++k)
                    {
                       if (line & 0x80)
                           std::cout << "1";
                       else
                           std::cout << "0";

                       line <<= 1;
                    }
                }
                std::cout << std::endl;
            }
        }
        else if (m_bmp.colorDepth == BMPTYPE_4BIT)
        {
            quint32 numByteRow = (m_bmp.width * m_bmp.colorDepth)/8;
            quint32 padding = ((m_bmp.width * m_bmp.colorDepth) % 8);
            numByteRow += (padding == 4) ? 1 : 0;

            for (uint i = 0; i < m_bmp.height; ++i)
            {
                for (uint j = 0; j < numByteRow; ++j)
                {
                    quint8 line = m_bmp.image[i][j];
                    std::cout << QChar((line & 0xF0) >>4).unicode() << " " << QChar((line & 0x0F)).unicode() << " ";
                }
            }
            std::cout << std::endl;
        }
    }
}

void ParseBMP::saveArray(const char* output, bool reverse)
{
    if (!m_bmp.isImageReaded)
    {
#if SHOW_DEBUG == 1
        qDebug() << "IMAGE NOT READED!";
        return;
#endif
    }

    QFile file(output);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        std::cerr << "ERROR: Can't open output file!";
        return;
    }

    QTextStream outStream(&file);

    if (m_bmp.isComplete)
    {
        if (m_bmp.colorDepth == BMPTYPE_1BIT)
        {
            outStream << "static const uint8_t imgArray[] = \n{\n";
            if  (reverse)
            {
                for (int i = m_bmp.height - 1; i >= 0; --i)
                {
                    for (int j = 0; j < m_bmp.numByteRow; ++j)
                    {
                        outStream << "0b";
                        quint8 line = m_bmp.image[i][j];
                        for (uint k = 0; k < 8; ++k)
                        {
                           if (line & 0x80)
                               outStream << "0";
                           else
                               outStream << "1";

                           line <<= 1;
                        }
                        outStream << ", ";
                    }
                    outStream << "\n";
                }
            }
            else
            {
                for (int i = 0; i < m_bmp.height; ++i)
                {
                    for (int j = 0; j < m_bmp.numByteRow; ++j)
                    {
                        outStream << "0b";
                        quint8 line = m_bmp.image[i][j];
                        for (uint k = 0; k < 8; ++k)
                        {
                           if (line & 0x80)
                               outStream << "0";
                           else
                               outStream << "1";

                           line <<= 1;
                        }
                        outStream << ", ";
                    }
                    outStream << "\n";
                }
            }
            outStream << "};";
        }
        else if (m_bmp.colorDepth == BMPTYPE_4BIT)
        {
            quint32 numByteRow = (m_bmp.width * m_bmp.colorDepth)/8;
            quint32 padding = ((m_bmp.width * m_bmp.colorDepth) % 8);
            numByteRow += (padding == 4) ? 1 : 0;

            outStream << "static const uint8_t imgArray[] = \n{\n";
            for (int i = m_bmp.height - 1; i >= 0; --i)
            {
                for (int j = 0; j < numByteRow; ++j)
                {
                    quint8 line = m_bmp.image[i][j];
                    quint8 newline = ((15 - ((line & 0xF0) >> 4)) << 4) + (15 - (line & 0x0F));

                    outStream << newline;
                    outStream << ",\t";
                }
                outStream << "\n";
            }
            outStream << "};";
        }
    }

    file.close();
}
