#ifndef QPDFWRITER_H
#define QPDFWRITER_H

#include "public/fpdf_save.h"
#include "public/fpdf_doc.h"

#include <QString>
#include <string>
#include <fstream>

class Writer {
public:
    Writer(QString path);
    ~Writer();

    void saveAs(FPDF_DOCUMENT document);

    FPDF_FILEWRITE m_fileWrite;
};

static std::ofstream m_ofstream;

#endif // QPDFWRITER_H
