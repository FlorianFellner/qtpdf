#include "qpdfwriter.h"

int WriteBlock(FPDF_FILEWRITE* pFileWrite, const void* data, unsigned long size)
{
    std::string dataStr(static_cast<const char*>(data), size);
    m_ofstream << dataStr;

    return 1;
}

/*!
 * Creates a new Writer object with the path of the file it should write to.
 * This creates a FPDF_FILEWRITE with the version 1 and a WriteBlock using std::ofstream.
 */
Writer::Writer(QString path)
{
    memset(&m_fileWrite, '\0', sizeof(m_fileWrite));

    m_fileWrite.version = 1;
    m_fileWrite.WriteBlock = WriteBlock;

    m_ofstream = std::ofstream(path.toStdString(), std::ios::out | std::ios::binary);
}

Writer::~Writer()
{
    m_ofstream.close();
}

/*!
 * Saves a copy of \a document to the path of the output stream using FPDF_SaveAsCopy(FPDF_Dcoument, FPDF_FILEWRITE, FPDF_DWORD).
 * The flag will always be set to FPDF_INCREMENTAL.
 */
void Writer::saveAs(FPDF_DOCUMENT document)
{
    FPDF_SaveAsCopy(document, &m_fileWrite, FPDF_INCREMENTAL);
}
