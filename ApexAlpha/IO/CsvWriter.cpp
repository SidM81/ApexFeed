#include "ApexAlpha/IO/CsvWriter.h"

#include <iomanip>
#include <iostream>
#include <utility>

namespace apex::alpha::io {

CsvWriter::CsvWriter(std::string path) {
    if (path.empty()) {
        return;
    }

    output_.open(path, std::ios::out | std::ios::trunc);
    if (!output_) {
        std::cerr << "[ApexAlpha] failed to open csv output: " << path << '\n';
        return;
    }

    output_ << std::fixed << std::setprecision(6);
    enabled_ = true;
    writeHeader();
}

void CsvWriter::writeHeader() {
    if (!enabled_) {
        return;
    }

    output_ << "timestamp,symbol,msg_type,spread,mid,microprice,queue_imbalance,order_flow_imbalance,has_valid_tob,"
            << "signal,signal_size,trade_side,trade_size,trade_price,position,realized_pnl,unrealized_pnl,total_pnl\n";
}

void CsvWriter::write(const EventRecord& record) {
    if (!enabled_) {
        return;
    }

    output_ << record.features.timestamp << ','
            << record.features.symbol << ','
            << record.message_type << ','
            << record.features.spread << ','
            << record.features.mid << ','
            << record.features.microprice << ','
            << record.features.queue_imbalance << ','
            << record.features.order_flow_imbalance << ','
            << (record.features.has_valid_tob ? 1 : 0) << ','
            << toString(record.signal.side) << ','
            << record.signal.size << ',';

    if (record.trade.has_value()) {
        output_ << toString(record.trade->side) << ','
                << record.trade->size << ','
                << record.trade->price << ',';
    } else {
        output_ << "N/A,0,0,";
    }

    output_ << record.portfolio.position << ','
            << record.portfolio.realized_pnl << ','
            << record.portfolio.unrealized_pnl << ','
            << record.portfolio.total_pnl
            << '\n';
}

void CsvWriter::flush() {
    if (!enabled_) {
        return;
    }

    output_.flush();
}

}  // namespace apex::alpha::io
