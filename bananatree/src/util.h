#pragma once
void* load_file_from_resource(int resource, const std::string& type, size_t& size);
std::string open_dialog(const char* filter);
std::string save_dialog(const char* filter);