#pragma once

#include "common.hpp"

namespace iic_commu {

extern master_to_slave_iic_data_t master_data;
extern slave_to_master_iic_data_t slave_data;

auto rec_event_callback(int len) -> void;
auto req_event_callback() -> void;

auto begin() -> void;

} // namespace iic_commu
