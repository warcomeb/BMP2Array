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

#include <QCoreApplication>
#include <QString>

#include <iostream>

#include "metadata.h"
#include "parsebmp.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // Print program informations
    std::cout << "********************************************************************************" << std::endl;
    std::cout << PROJECT_NAME << std::endl;
    std::cout << "(C) " <<  COPYRIGHT_YEAR << " of " << ORGANIZATION_NAME << std::endl;
    std::cout << "This software is released under GPLV3!" << std::endl;
    std::cout << "********************************************************************************" << std::endl;
    std::cout << "Version: " <<  PROJECT_VERSION << " of " << RELEASE_DATE << std::endl;
    std::cout << "********************************************************************************" << std::endl;

    if ((argc == 4) || (argc == 5))
    {
        QFile file(argv[1]);
        if (!file.open(QIODevice::ReadOnly))
        {
            std::cerr << "No file found!" << std::endl;
        }
        else
        {
            ParseBMP bmp(&file, (BMPType) atoi(argv[2]));
            bmp.dumpImage();

            bool reverse = false;
            if (argc == 5)
            {
                if (QString::compare(argv[4], "R", Qt::CaseInsensitive) == 0)
                {
                    reverse = true;
                }
                else
                {
                    reverse = false;
                }
            }

            bmp.saveArray(argv[3],reverse);
        }
    }
    else
    {
        std::cerr << "Wrong parameters number!" << std::endl;
        std::cerr << "Usare: BMP2Array file.bmp depth destination.c [R|N]" << std::endl;
        // TODO: Print usage
    }

    return a.exec();
}
