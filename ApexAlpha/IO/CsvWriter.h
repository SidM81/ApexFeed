#ifndef APEXALPHA_IO_CSV_WRITER_H
#define APEXALPHA_IO_CSV_WRITER_H

#include <fstream>
#include <string>

#include "ApexAlpha/Models/EventRecord.h"

namespace apex::alpha::io {

class CsvWriter {
public:
    explicit CsvWriter(std::string path);

    void write(const EventRecord& record);
    void flush();

private:
    void writeHeader();

private:
    std::ofstream output_{};
    bool enabled_{false};
};

}  // namespace apex::alpha::io

#endif
